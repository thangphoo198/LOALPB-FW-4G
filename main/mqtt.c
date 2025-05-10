/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/

#include "MQTTClient.h"
#include "ql_data_call.h"
#include "ql_application.h"
#include "sockets.h"
#include "ql_ssl_hal.h"
#include "uart_usb.h"
#include "cJSON.h"
#include "mqtt.h"
#include "ql_nw.h"
#include "ql_rtos.h"
#include "play_amount.h"
#include "main.h"
#include "flash.h"
#include "play_amount_LPB.h"
#include "name_mp3.h"
#include "ql_power.h"
#define mqtt_exam_log(fmt, ...) printf("[MQTT_EXAM][%s, %d] " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#if SSL_ENABLE
#define SSL_VERIFY_MODE 1 // SSL_VERIFY_MODE_OPTIONAL
#define SSL_CERT_FROM 0   // SSL_CERT_FROM_BUF
#define SSL_CIPHER_LIST "ALL"
#define SERVER_PORT_VERIFY_BOTH 8883
#define SERVER_PORT 8883
#else
#define SERVER_PORT 1883
#endif
MQTTClient client = {0};
unsigned char sendbuf[256] = {0}, readbuf[256] = {0};
SSLConfig SSL_con = {
    .en = SSL_ENABLE,
#if SSL_ENABLE
    .profileIdx = PROFILE_IDX,
    .serverName = SERVER_DOMAIN,
    .serverPort = SERVER_PORT,
    .protocol = 0,
    .dbgLevel = 0,
    .sessionReuseEn = 0,
    .vsn = SSL_VSN_ALL,
    .verify = SSL_VERIFY_MODE,
    .cert.from = SSL_CERT_FROM_BUF,
#if (SERVER_PORT == 8307)
    .cert.path.rootCA = rootCA_path,
    .cert.path.clientKey = NULL,
    .cert.path.clientCert = NULL,
#elif (SERVER_PORT == 8883)
    .cert.path.rootCA = rootCA_path,
    .cert.path.clientKey = clientKey_path,
    .cert.path.clientCert = clientCert_path,
    .cert.clientKeyPwd.data = NULL,
    .cert.clientKeyPwd.len = 0,
#endif
    .cipherList = SSL_CIPHER_LIST,
    .CTRDRBGSeed.data = NULL,
    .CTRDRBGSeed.len = 0
#endif
};
static void messageArrived(MessageData *data)
{
    usb_log("topic %s: Size:%d %s\r\n", (char *)data->topicName->lenstring.data, data->message->payloadlen, (char *)data->message->payload);
    ql_rtos_queue_release(mqtt_queue, sizeof(u8 *), (u8 *)data->message->payload, QL_NO_WAIT);
    // if (data->message->payload != NULL)
    // {
    //     free(data->message->payload);
    //     data->message->payload = NULL;
    // }
}
int pub_mqtt(char *topic, char *mess)
{
    MQTTMessage message;
    message.qos = 0;
    message.retained = 0;
    message.payload = mess;
    message.payloadlen = strlen(mess);
    int rc = MQTTPublish(&client, topic, &message);
    // MQTTYield(&client, 100);
    return rc;
}
int sub_mqtt(char *topic)
{
    return MQTTSubscribe(&client, topic, 2, messageArrived);
}

static void MQTTEchoTask(void *argv)
{
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly for 3 times */
    Network network = {0};
    unsigned char sendbuf[256] = {0}, readbuf[256] = {0};
    int rc = 0;
    MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    NetworkInit(&network, &SSL_con, PROFILE_IDX);
    MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
    if (sn_mode == 0)
    {
        strcat(topic_remote, mac_id);
        strcat(topic_remote, "/AMOUNT");
        strcat(topic_rec, mac_id);
        strcat(topic_rec, "/REC");
    }
    else if (sn_mode == 1)
    {
        printf("SN %s \r\n", SN);
        strcat(topic_remote, SN);
        strcat(topic_remote, "/AMOUNT");
        strcat(topic_rec, SN);
        strcat(topic_rec, "/REC");
    }
    printf("topic rec:%s topic remote:%s TOPIC_SERIAL%s\r\n", topic_rec, topic_remote, TOPIC_SERIAL);
    if (rc = NetworkConnect(&network, SERVER_DOMAIN, SERVER_PORT) != 0)
    {
        mqtt_exam_log("Return code from network connect is %d", rc);
        MQTTClientDeinit(&client);
        goto exit;
    }

    connectData.MQTTVersion = 3;
    connectData.clientID.cstring = mac_id; // ID
    connectData.username.cstring = MQTT_USERNAME;
    connectData.password.cstring = MQTT_PASS;

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
    {
        mqtt_connect = 0;
        mqtt_exam_log("Return code from MQTT connect is %d", rc);
        NetworkDisconnect(&network);
        MQTTClientDeinit(&client);
        goto exit;
    }
    else
        mqtt_exam_log("MQTT Connected");
    mqtt_connect = 1;

#if defined(MQTT_TASK)
    if ((rc = MQTTStartTask(&client)) != 0)
    {
        mqtt_connect = 0;
        mqtt_exam_log("Return code from start tasks is %d", rc);
        NetworkDisconnect(&network);
        MQTTClientDeinit(&client);
        goto exit;
    }
#endif

    // if ((rc = MQTTSubscribe(&client, "EC800M/AMOUNT", 2, messageArrived)) != 0)
    // {
    //     mqtt_connect = 0;
    //     mqtt_exam_log("Return code from MQTT subscribe is %d", rc);

    //     rc = MQTTDisconnect(&client);
    //     if (rc == SUCCESS)
    //         mqtt_exam_log("MQTT Disconnected by client");
    //     else
    //         mqtt_exam_log("MQTT Disconnected failed by client");

    //     NetworkDisconnect(&network);

    //     MQTTClientDeinit(&client);

    //     goto exit;
    // }

    if (sub_mqtt(topic_remote) == 0)
    {
        mqtt_exam_log("SUB OK %s", topic_remote);
        // PlayMP3File("ok.mp3");
    }

    // pub_mqtt("EC800M_REC", "MQTT OK");
    PlayMP3File(maychu_ok);

    while (1)
    {
        mqtt_connect = (client.isconnected == 1) ? 1 : 0;
        printf("\r\nRC:%d MQTT_CONNECT:%d\r\n", client.isconnected, mqtt_connect);
        if (client.isconnected == 0)
        {
            // NetworkDisconnect(&network);
            // MQTTClientDeinit(&client);

            // NetworkInit(&network, &SSL_con, PROFILE_IDX);
            // MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
            if (NetworkConnect(&network, SERVER_DOMAIN, SERVER_PORT) == 0)
            {
                mqtt_exam_log("KET NOI LAI MANG ok");
                if (MQTTConnect(&client, &connectData) == 0)
                {
                    mqtt_exam_log("Return code from MQTT connect is OK");
                    if (MQTTStartTask(&client) == 0)
                    {
                        mqtt_exam_log("Return code from start tasks OK");
                        if (sub_mqtt(topic_remote) == 0)
                        {
                            mqtt_exam_log("SUB OK");
                            PlayMP3File(maychu_ok);
                        }
                    }
                }
                else
                {
                    mqtt_exam_log("Return code from MQTT connect is FAILED");
                }
            }
            else
            {
                mqtt_exam_log("KET NOI LAI MANG failed");
                Play_mp3(matmang);
            }

            ql_rtos_task_sleep_s(5);
        }
        ql_rtos_task_sleep_s(2);
    }

    rc = MQTTDisconnect(&client);
    if (rc == SUCCESS)
        mqtt_exam_log("MQTT Disconnected by client");
    else
        mqtt_exam_log("MQTT Disconnected failed by client");

    NetworkDisconnect(&network);

    MQTTClientDeinit(&client);

exit:

    mqtt_exam_log("========== mqtt test end ==========");
    ql_rtos_task_delete(NULL);
}

void mqtt_publisher_thread(void *arg)
{
    u8 message[256];
    while (1)
    {
        ql_rtos_queue_wait(mqtt_queue, message, 256, QL_WAIT_FOREVER);
        if (strlen(message) >= 2)
        {
            printf("da nhan tu queue:%s\r\n", message);

            // pub_mqtt("EC800M_REC", message);
            cJSON *root = cJSON_Parse((char *)message);
            if (root != NULL)
            {

                cJSON *rsp_obj = cJSON_GetObjectItem(root, "RSP");
                if (rsp_obj != NULL && cJSON_IsString(rsp_obj))
                {
                    printf("RSP value: %s\r\n", rsp_obj->valuestring);
                    if (strcmp(rsp_obj->valuestring, "SN_REQUEST") == 0)
                    {

                        pub_mqtt(TOPIC_SERIAL, message);
                    }
                    // Thực hiện xử lý khác nếu cần
                }

                cJSON *obj = cJSON_GetObjectItem(root, "Type");
                if (obj != NULL)
                {
                    if (cJSON_IsString(obj))
                    {
                        char buff[64] = {0}; // Tăng kích thước để tránh tràn bộ đệm
                        if (strcmp(obj->valuestring, "PlayAmount") == 0)
                        {
                            // Transaction Broadcast
                            obj = cJSON_GetObjectItem(root, "Amount");
                            if (obj != NULL && cJSON_IsString(obj)) // kiểm tra là chuỗi
                            {
                                const char *amountStr = obj->valuestring;
                                char *end;
                                uint64_t num = strtoull(amountStr, &end, 10);
                                printf("tien: %lld\n", num);
                                // play_tien_mqtt(num);
                                // PlayMP3File(ting);
                                readMoney(num);
                                // printf("KET QUA STRING: %s\r\n", amountStr);
                                // unsigned long long number = strtol(amountStr, NULL, 10); // chuyển string -> long
                                // printf("KET QUA: %ld\r\n", number);
                                // // play_tien_mqtt(number);
                                // readMoney(number);
                                sprintf(buff, "{\"RSP\":\"PlayAmount\",\"RESULT\":1,\"ID\":\"%s\",\"Amount\":%lld}", "123456", num);
                                printf("KET QUA: %s\r\n", buff);
                                pub_mqtt(topic_rec, buff);
                            }
                        }

                        else if (strcmp(obj->valuestring, "FOTA") == 0)
                        {
                            obj = cJSON_GetObjectItem(root, "LINK");
                            printf("link tai xuong:%s\r\n", obj->valuestring);
                        }
                        else if (strcmp(obj->valuestring, "GET_INFO") == 0)
                        {

                            // char buff[64] = {0};
                            // sprintf(buff, "{\"RSP\":\"GET_STT\",\"STT\":1,\"ID\":\"%s\"}", "123456");
                            //  pub_mqtt(TOPIC_SERIAL, message);
                        }

                        else if (strcmp(obj->valuestring, "SET_SN") == 0)
                        {
                            obj = cJSON_GetObjectItem(root, "SN");
                            if (obj == NULL || obj->valuestring == NULL)
                            {
                                usb_log("Không tìm thấy hoặc SN không hợp lệ!\r\n");
                                return;
                            }
                            const char *serial_number = obj->valuestring;
                            size_t serial_length = strlen(serial_number);

                            usb_log("ma SN : %s size:%zu", serial_number, serial_length);

                            // Kiểm tra độ dài của SN
                            if (serial_length >= 64)
                            {
                                usb_log("SN quá dài, không thể lưu!\r\n");
                                return;
                            }
                            int ret = nor_flash_erase(start_ad + SERIAL_ADDRESS, 32);
                            if (ret == 0)
                            {
                                printf("flash erase succeed\r\n");
                            }
                            else
                            {
                                printf("flash_erase_error=%d\r\n ", ret);
                            }
                            ret = flash_write(serial_number, SERIAL_ADDRESS);
                            if (ret == 0)
                            {
                                printf("LUU SERIAL : %s THANH CONG!\r\n", serial_number);

                                // Chuẩn bị phản hồi MQTT
                                snprintf(buff, sizeof(buff), "{\"RSP\":\"SET_SN\",\"SN\":\"%s\"}", serial_number);
                                pub_mqtt(topic_rec, buff);
                                ql_power_reset();
                            }
                            else
                            {
                                usb_log("LUU SERIAL THAT BAI\r\n");
                            }
                        }

                        else if (strcmp(obj->valuestring, "LIST_FILE") == 0)
                        {

                            obj = cJSON_GetObjectItem(root, "DISK");
                            printf("DISK :%s\r\n", obj->valuestring);
                            list_dir(obj->valuestring);
                            // char buff[64] = {0};
                            // sprintf(buff, "{\"RSP\":\"GET_STT\",\"STT\":1,\"ID\":\"%s\"}", "123456");
                            // pub_mqtt("EC800M_REC", buff);
                        }
                    }
                }
            }
            // cJSON_free((void *)out); // giai phóng bộ nhớ
            cJSON_Delete(root); // giải phóng con trỏ json
        }
        // ql_rtos_task_sleep_ms(50);
    }
}

void StartMQTTTask(void)
{
    ql_task_t task = NULL;
    ql_task_t mqtt_publisher_task = NULL;
    ql_rtos_task_create(&task, 10 * 1024, 100, "mqtt_test", MQTTEchoTask, NULL);
    ql_rtos_task_create(&mqtt_publisher_task, 15 * 1024, 100, "mqtt_PUB", mqtt_publisher_thread, NULL);

    // ql_rtos_mutex_create(&mutexRef);
    // QlOSStatus status = ql_rtos_queue_create(&mqtt_queue, sizeof(u8 *), 128);
    ql_rtos_queue_create(&mqtt_queue, 256, 8);
}

static void ql_nw_status_callback(int profile_idx, int nw_status)
{
    mqtt_exam_log("profile(%d) status: %d", profile_idx, nw_status);
}
int datacall_start(void)
{
    struct ql_data_call_info info = {0};
    char ip4_addr_str[16] = {0};

    mqtt_exam_log("wait for network register done");

    if (ql_network_register_wait(120) != 0)
    {
        mqtt_exam_log("*** network register fail ***");
        return -1;
    }
    else
    {
        mqtt_exam_log("doing network activating ...");

        ql_wan_start(ql_nw_status_callback);
        ql_set_auto_connect(PROFILE_IDX, TRUE);
        if (ql_start_data_call(PROFILE_IDX, 0, "v-internet", NULL, NULL, 0) == 0)
        {
            ql_get_data_call_info(PROFILE_IDX, 0, &info);

            mqtt_exam_log("info.profile_idx: %d", info.profile_idx);
            mqtt_exam_log("info.ip_version: %d", info.ip_version);
            mqtt_exam_log("info.v4.state: %d", info.v4.state);
            mqtt_exam_log("info.v4.reconnect: %d", info.v4.reconnect);

            inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
            mqtt_exam_log("info.v4.addr.ip: %s", ip4_addr_str);

            inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
            mqtt_exam_log("info.v4.addr.pri_dns: %s", ip4_addr_str);

            inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
            mqtt_exam_log("info.v4.addr.sec_dns: %s", ip4_addr_str);

            return 0;
        }

        mqtt_exam_log("*** network activated fail ***");
        return -1;
    }
}

// static void MQTTTest(void *argv)
// {
//     (void)argv;

//     mqtt_exam_log("========== mqtt test satrt ==========");

//     if (datacall_start() == 0)
//         StartMQTTTask();
//     else
//         mqtt_exam_log("========== mqtt test end ==========");
// }

// application_init(MQTTTest, "mqtttest", 2, 14);
