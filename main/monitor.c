/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_timer.c
 * @author  Juson.zhang
 * @version V1.0.0
 * @date    2020/04/02
 * @brief   This file tests timer's APIs
 ******************************************************************************
 */

#include <stdio.h>
#include "ql_rtos.h"
#include "ql_type.h"
#include "ql_application.h"
#include "ql_timer.h"
#include "uart_usb.h"
#include "ql_gpio.h"
#include "mqtt.h"
#include "main.h"
#include "ql_power.h"
#include "ql_nw.h"
#include "cJSON.h"

#define PIN_VOL_MIN 3500
#define PIN_VOL_MAX 4200
#define test_log(fmt, args...) usb_log(fmt, ##args)
bool led = 0;
bool led2 = 0;

int bat_read()
{
    // Lấy điện áp pin
    int bat_level;
    unsigned short bat_vol = ql_get_battery_vol();
    test_log("vbat_vol: %d mV \r\n", bat_vol);
    if (bat_vol <= PIN_VOL_MIN)
    {
        bat_level = 0; // Pin yếu
    }
    else if (bat_vol >= PIN_VOL_MAX)
    {
        bat_level = 100; // Pin đầy
    }
    else
    {
        // Chuyển đổi từ 3500mV - 4200mV thành mức 0 - 100
        bat_level = (bat_vol - PIN_VOL_MIN) * 100 / (PIN_VOL_MAX - PIN_VOL_MIN);
    }

    // Làm tròn mức pin về bội số gần nhất của 10
    bat_level = ((bat_level + 5) / 10) * 10;

    // Ghi log mức pin
    test_log("\r\nBattery Level: %d%%\r\n", bat_level);
    return bat_level;
}
NET_LEVEL net_status_csq()
{
    int csq;
    int ret = ql_nw_get_csq(&csq);
    printf("\nql_nw_get_csq ret=%d\n csq:%d\n", ret, csq);
    NET_LEVEL network_status;
    if (csq <= 10)
    {
        network_status = NET_LOW;
    }
    else if (csq >= 11 && csq <= 20)
    {
        network_status = NET_MID;
    }
    else if (csq >= 21 && csq <= 31)
    {
        network_status = NET_HIGH;
    }
    else
    {
        network_status = NET_NONE;
    }
    return network_status;
}
void timer_test_cb(unsigned int param)
{
    // Cập nhật thời gian boot
    boot_time += 10;
    test_log("timer_id=%d \n", boot_time);
    test_log("[HEAP ](%d)\r\n", ql_rtos_get_free_heap_size());
    led2 ^= 1;
    ql_gpio_set_level(STT, led2 == 0 ? PIN_LEVEL_LOW : PIN_LEVEL_HIGH);
    // cJSON *pRoot = cJSON_CreateObject();
    // cJSON_AddStringToObject(pRoot, "Type", "GET_STT");
    // cJSON *pValue = cJSON_CreateObject();
    // //  cJSON_AddStringToObject(pValue, "imei", imei);
    // // cJSON_AddStringToObject(pValue, "sim_name", oper_info->long_oper_name);
    // cJSON_AddStringToObject(pValue, "MODE", "4G");
    // cJSON_AddNumberToObject(pValue, "CSQ", net_status_csq());
    // cJSON_AddNumberToObject(pValue, "BAT_LV", bat_read());
    // cJSON_AddNumberToObject(pValue, "BOOT_TIME", boot_time);
    // cJSON_AddItemToObject(pRoot, "DATA", pValue);
    // char *stt_info = cJSON_PrintUnformatted(pRoot);

    cJSON *pRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(pRoot, "RSP", "SN_REQUEST");
    //cJSON *pValue = cJSON_CreateObject();
    //  cJSON_AddStringToObject(pValue, "imei", imei);
    // cJSON_AddStringToObject(pValue, "sim_name", oper_info->long_oper_name);
    cJSON_AddStringToObject(pRoot, "MAC_ID", mac_id);
    // cJSON_AddNumberToObject(pValue, "CSQ", net_status_csq());
    // cJSON_AddNumberToObject(pValue, "BAT_LV", bat_read());
    // cJSON_AddNumberToObject(pValue, "BOOT_TIME", boot_time);
    // cJSON_AddItemToObject(pRoot, "DATA", pValue);
    char *stt_info = cJSON_PrintUnformatted(pRoot);
    
    test_log("\n%s\n", stt_info);
    // Gửi dữ liệu vào hàng đợi MQTT
    ql_rtos_queue_release(mqtt_queue, sizeof(u8 *), (u8 *)stt_info, QL_NO_WAIT);
    cJSON_free((void *)stt_info);
    cJSON_Delete(pRoot);
    // Nếu kết nối MQTT đã sẵn sàng, gửi dữ liệu trực tiếp
    /*
    if (mqtt_connect == 1)
    {
        pub_mqtt("EC800M_REC", buff);
    }
    */
}

static void quec_timer_test(void *argv)
{
    ql_rtos_task_sleep_s(5);
    ql_gpio_init(LED_STT, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
    ql_gpio_init(STT, PIN_DIRECTION_OUT, PIN_PULL_DISABLE, PIN_LEVEL_LOW);
    ql_gpio_init(WIFI_EN ,PIN_DIRECTION_OUT, PIN_PULL_PD, PIN_LEVEL_LOW);
    ql_gpio_set_level(WIFI_EN, PIN_LEVEL_LOW);
    int timer_id = 0;
   // timer_id = ql_start_Acctimer(QL_TIMER_PERIOD, 5000000, timer_test_cb, 0);

    while (1)
    {
        // usb_log("g_timer_cnt=%d LEVEL:%d \n", g_timer_cnt, ql_gpio_get_level(LED_STT, &level));
        if (mqtt_connect == 0)
        {
            led ^= 1;
            ql_gpio_set_level(LED_STT, led == 0 ? PIN_LEVEL_LOW : PIN_LEVEL_HIGH);
            ql_rtos_task_sleep_ms(2000);
        }
        else
        {
            // led ^= 1;
            // ql_gpio_set_level(LED_STT, led == 0 ? PIN_LEVEL_LOW : PIN_LEVEL_HIGH);
            ql_gpio_set_level(LED_STT, PIN_LEVEL_HIGH);
            ql_rtos_task_sleep_ms(2000);
            ql_gpio_set_level(LED_STT, PIN_LEVEL_LOW);
            ql_rtos_task_sleep_ms(200);
            // u8 vbus_state = ql_get_vbus_state();
            // test_log("ql_get_vbus_state: 0x%d\n", vbus_state);
        }
    }
}

application_init(quec_timer_test, "quec_timer_test", 10, 0);
