
#include <stdio.h>
#include "ql_application.h"
#include "ql_log.h"
#include "ql_rtos.h"
#include "netdb.h"
#include "uart_usb.h"
#include "ql_timer.h"
#include "ql_log.h"
#include "mqtt.h"
#include "ql_atcmd.h"
#include "mp3.h"
#include "flash.h"
#include "ql_dev.h"
#include "main.h"
#include <ctype.h>
#include "play_amount_LPB.h"
#define test_log(fmt, args...) usb_log(fmt, ##args)
void send_at(char *cmd)
{
    char response[256]; // Định nghĩa một buffer để chứa phản hồi
    QL_AT_RESP_STATUS_E status = ql_atcmd_send_sync(cmd, response, sizeof(response), NULL, 1000);

    if (status == QL_AT_RESP_OK)
    {
        usb_log("AT command response: %s\n", response);
    }
    else
    {
        usb_log("Error sending AT command\n");
    }
}

static void main_app(void *argv)
{
    int ret = 0;
    uart_usb_init();
    ret = ql_dev_get_sn(mac_id, sizeof(mac_id));
    printf("ql_dev_get_sn ret=%d; sn:%s \n", ret, mac_id);

    // uint32_t start_ad = flash_init();
    // printf("flash start AD= 0x%08x \n", start_ad);
    memset(SN, 0, sizeof(SN));
    strcpy(SN, flash_read(SERIAL_ADDRESS));

    // ret = nor_flash_read(start_ad+SERIAL_ADDRESS, SN, 64);
    if (SN[0] != 0xff && strlen(SN)>8)
    {
        printf("Có serial NUMBER sn: %s\n", SN);
        sn_mode = 1;
    }
    else
    {

        memset(SN, 0, sizeof(SN));
        strcat(SN, mac_id);
        printf("Serial NUMBER K HOP LE, THAY BANG MAC_ID: %s\n", SN);
        sn_mode = 0;
    }


    printf("\n==========START==========\n");
    // ret = ql_dev_get_imei(mac_id, sizeof(data));
    // printf("ql_dev_get_imei ret=%d; IMEI:%s \n", ret, data);
    // memset(data, 0, sizeof(data));

    // ret = ql_dev_get_model(data, sizeof(data));
    // printf("ql_dev_get_model ret=%d; model:%s \n", ret, data);
    // memset(data, 0, sizeof(data));
    // memset(data, 0, sizeof(data));

    // ret = ql_dev_get_firmware_version(data, sizeof(data));
    // printf("ql_dev_get_firmware_version ret=%d; firmware_version:%s \n", ret, data);
    // memset(data, 0, sizeof(data));

    // ret = ql_dev_get_product_id(data, sizeof(data));
    // printf("ql_dev_get_product_id ret=%d; product_id:%s \n", ret, data);
    // memset(data, 0, sizeof(data));
    mp3_init();
   // readMoney(123456778);
    //  dtmf_test();
    // ql_rtos_semaphore_create(&semaRef, 1);
    if (datacall_start() == 0)
    {
        usb_log("MẠNG OK \r\n");
        net_stt = 1; // net work ok
        StartMQTTTask();
        // update_fota();
    }
    else
    {
        net_stt = 0;
    }
    while (1)
    {
        // send_at("AT+CSQ\r\n");
        test_log("[HEAP ](%d)\r\n", ql_rtos_get_free_heap_size());
        ql_rtos_task_sleep_s(2);
    }
}
application_init(main_app, "main_test", 10, 0);
