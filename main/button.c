#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_power.h"
#include "uart_usb.h"
#include "main.h"
#include "play_amount.h"
#include "monitor.h"
#include "mqtt.h"
#include "flash.h"
#define test_log(fmt, args...) usb_log(fmt, ##args)
static ql_flag_t key_flag;
static int g_test01_cnt = 0;
#define KEY_HIGH_FLAG 0x01
#define KEY_LOW_FLAG 0x02
#define QL_KEY_MASK_FLAG (KEY_HIGH_FLAG | KEY_LOW_FLAG)
static void ngat1(void)
{
    ql_eint_disable(VOL_KEY_DOWN);
    PIN_LEVEL_E value;
    g_test01_cnt++;
    ql_gpio_get_level(VOL_KEY_DOWN, &value);
    test_log("get gpio[%d] level%d \r\n", VOL_KEY_DOWN, value);

    if (value == PIN_LEVEL_LOW)
    {
        ql_rtos_flag_release(key_flag, KEY_LOW_FLAG, QL_FLAG_OR);
    }
    else
    {
        ql_rtos_flag_release(key_flag, KEY_HIGH_FLAG, QL_FLAG_OR);
    }
    ql_eint_enable(VOL_KEY_DOWN, PIN_BOTH_EDGE); // is needed to active isr
                                                 // ql_eint_enable_wakeup(VOL_KEY_DOWN, PIN_BOTH_EDGE);
}
static void quec_eint_test(void *argv)
{
    int ret = -1;
    int j = 0;
    unsigned int event = 0;
    unsigned int pwrkey_ticks = 0;
    ql_rtos_flag_create(&key_flag);
    ql_autosleep_enable(QL_ALLOW_SLEEP);
    // ql_gpio_init(VOL_KEY_DOWN, PIN_DIRECTION_IN, PIN_PULL_PU, PIN_LEVEL_HIGH);
    //	ret = ql_eint_register(VOL_KEY_DOWN, PIN_FALLING_EDGE, PIN_PULL_PU, __gpio_irq_handler_test01, NULL);
    ret = ql_eint_register(VOL_KEY_DOWN, PIN_BOTH_EDGE, PIN_PULL_PU, ngat1, NULL);
    if (ret)
    {
        test_log("eint register failed, ret = %d\n", ret);
        return;
    }
    while (1)
    {
        ret = ql_rtos_flag_wait(key_flag, QL_KEY_MASK_FLAG, QL_FLAG_OR_CLEAR, &event, QL_WAIT_FOREVER);
        if (ret)
        {
            test_log("quec_rtos_flag_wait err!\r\n");
            return;
        }

        if (KEY_LOW_FLAG == event)
        {
            test_log("PWRKEY has been pressed! \r\n");
            pwrkey_ticks = ql_rtos_get_systicks();
        }
        else if (KEY_HIGH_FLAG == event)
        {
            u32 press_duration = ql_rtos_get_systicks() - pwrkey_ticks;
            test_log("PWRKEY has been released! \r\n");
            if (press_duration > 400)
            { // 130*5=650ms,Quectel defined powerkey powndown 650ms
                test_log("pwrkey long press, ticks=%d \r\n", press_duration);
                PlayMP3File("voldown.mp3");
                ql_gpio_set_level(WIFI_EN, PIN_LEVEL_LOW);
                //flash_write("11111111111111",SERIAL_ADDRESS);
                //flash_write("Loaisound",WIFISSID_ADDRESS);
                // ql_power_down(1);
            }
            else
            {
                ql_gpio_set_level(WIFI_EN, PIN_LEVEL_HIGH);
                test_log("pwrkey short press, ticks=%d \r\n", press_duration);
                char buff[64] = {0};
                sprintf(buff, "{\"RSP\":\"SN_REQUEST\",\"MAC_ID\":\"%s\"}", SN);
                printf("Generated buffer: %s\n", buff);
                ql_rtos_queue_release(mqtt_queue, sizeof(u8 *), (u8 *)buff, QL_NO_WAIT);
                NET_LEVEL t = net_status_csq();
                switch (t)
                {
                case NET_LOW:
                    PlayMP3File("mangkon.mp3");
                    break;
                case NET_MID:
                    PlayMP3File("mangtb.mp3");
                    break;
                case NET_HIGH:
                    PlayMP3File("mangtot.mp3");
                    break;
                case NET_NONE:
                    PlayMP3File("matmang.mp3");
                    break;
                default:
                    break;
                }
                // PlayMP3File("voldown.mp3");
                play_pin(bat_read());
            }
        }

        // ql_rtos_task_sleep_s(10);
    }
}
application_init(quec_eint_test, "quec_eint_test", 10, 0);