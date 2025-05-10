
#include <stdio.h>
#include "ql_gpio.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_power.h"
#include "uart_usb.h"
#include "main.h"
#include "play_amount.h"
#include "monitor.h"
#define test_log(fmt, args...) usb_log(fmt, ##args)

#define KEY_HIGH_FLAG 0x01
#define KEY_LOW_FLAG 0x02
#define QL_KEY_MASK_FLAG (KEY_HIGH_FLAG | KEY_LOW_FLAG)
#define K2_DN_FLAG (0x04)
#define K2_UP_FLAG (0x08)
#define PTT_DN_FLAG (0x10)
#define PTT_UP_FLAG (0x20)
#define UP_DN_FLAG (0x40)
#define UP_UP_FLAG (0x80)
#define DN_DN_FLAG (0x100)
#define DN_UP_FLAG (0x200)

#define ANY_KEY_FLAG (0x3FF)

static int g_test01_cnt = 0;
static int g_test02_cnt = 0;
static int g_test03_cnt = 0;
static int g_test04_cnt = 0;
static int g_test05_cnt = 0;

static ql_flag_t key_flag;

// static void __gpio_irq_handler_test01(void)
// {
// 	PIN_LEVEL_E value;

// 	g_test01_cnt++;
// 	ql_gpio_get_level(VOL_KEY_DOWN, &value);
// 	test_log("get gpio[%d] level%d \r\n", VOL_KEY_DOWN, value);

// 	if (value == PIN_LEVEL_LOW)
// 	{
// 		ql_rtos_flag_release(key_flag, KEY_LOW_FLAG, QL_FLAG_OR);
// 	}
// 	else
// 	{
// 		ql_rtos_flag_release(key_flag, KEY_HIGH_FLAG, QL_FLAG_OR);
// 	}
// 	ql_eint_enable(VOL_KEY_DOWN, PIN_FALLING_EDGE); // is needed to active isr
// 													// ql_eint_enable_wakeup(VOL_KEY_DOWN, PIN_BOTH_EDGE);
// }

// static void __gpio_irq_handler_test02(void)
// {
// 	PIN_LEVEL_E value;

// 	g_test02_cnt++;
// 	ql_gpio_get_level(GPIO_PIN_NO_121, &value);
// 	test_log("get gpio[%d] level%d \r\n", GPIO_PIN_NO_121, value);

// 	if(value == PIN_LEVEL_HIGH)
// 	{
// 		ql_rtos_flag_release(key_flag, K2_DN_FLAG, QL_FLAG_OR);
// 	}
// 	else
// 	{
// 		ql_rtos_flag_release(key_flag, K2_UP_FLAG, QL_FLAG_OR);
// 	}
// 	//ql_eint_enable(GPIO_PIN_NO_121, PIN_BOTH_EDGE);	//is needed to active isr
// 	ql_eint_enable_wakeup(GPIO_PIN_NO_121, PIN_BOTH_EDGE);
// }

// static void __gpio_irq_handler_test03(void)
// {
// 	PIN_LEVEL_E value;

// 	g_test03_cnt++;
// 	ql_gpio_get_level(GPIO_PIN_NO_11, &value);
// 	test_log("get gpio[%d] level%d \r\n", GPIO_PIN_NO_11, value);

// 	if(value == PIN_LEVEL_HIGH)
// 	{
// 		ql_rtos_flag_release(key_flag, PTT_DN_FLAG, QL_FLAG_OR);
// 	}
// 	else
// 	{
// 		ql_rtos_flag_release(key_flag, PTT_UP_FLAG, QL_FLAG_OR);
// 	}
// 	ql_eint_enable(GPIO_PIN_NO_11, PIN_BOTH_EDGE);	//is needed to active isr
// }

// static void __gpio_irq_handler_test04(void)
// {
// 	PIN_LEVEL_E value;

// 	g_test04_cnt++;
// 	ql_gpio_get_level(GPIO_PIN_NO_3, &value);
// 	test_log("get gpio[%d] level%d \r\n", GPIO_PIN_NO_3, value);

// 	if(value == PIN_LEVEL_HIGH)
// 	{
// 		ql_rtos_flag_release(key_flag, UP_DN_FLAG, QL_FLAG_OR);
// 	}
// 	else
// 	{
// 		ql_rtos_flag_release(key_flag, UP_UP_FLAG, QL_FLAG_OR);
// 	}
// 	ql_eint_enable(GPIO_PIN_NO_3, PIN_BOTH_EDGE);	//is needed to active isr
// }

// static void __gpio_irq_handler_test05(void)
// {
// 	PIN_LEVEL_E value;

// 	g_test05_cnt++;
// 	ql_gpio_get_level(GPIO_PIN_NO_4, &value);
// 	test_log("get gpio[%d] level%d \r\n", GPIO_PIN_NO_4, value);

// 	if(value == PIN_LEVEL_HIGH)
// 	{
// 		ql_rtos_flag_release(key_flag, DN_DN_FLAG, QL_FLAG_OR);
// 	}
// 	else
// 	{
// 		ql_rtos_flag_release(key_flag, DN_UP_FLAG, QL_FLAG_OR);
// 	}
// 	ql_eint_enable(GPIO_PIN_NO_4, PIN_BOTH_EDGE);	//is needed to active isr
// }

// static void quec_gpio_task(void* argv)
// {
//     ql_gpio_init(GPIO_PIN_NO_36,PIN_DIRECTION_OUT,PIN_PULL_PD,PIN_LEVEL_HIGH);
//     unsigned int cnt=0;
//     ql_rtos_task_sleep_ms(10000);
//     while(1)
//     {
//         ql_gpio_set_level(GPIO_PIN_NO_36,PIN_LEVEL_LOW);
//         cnt++;
//         printf("\r\ngpio[36]------cnt %d\r\n",cnt);
//         ql_rtos_task_sleep_ms(1000);
//         ql_gpio_set_level(GPIO_PIN_NO_36,PIN_LEVEL_HIGH);
//         cnt++;
//         printf("\r\ngpio[36]------cnt %d\r\n",cnt);
//         ql_rtos_task_sleep_ms(1000);
//     }

// }
#define DEBOUNCE_DELAY 20 // Độ trễ chống dội phím, đơn vị ms (có thể điều chỉnh)

// Biến toàn cục
uint32_t last_debounce_time = 0; // Thời điểm xử lý sự kiện cuối cùng
static void quec_eint_test(void *argv)
{
	int ret = -1;
	int j = 0;
	unsigned int event = 0;
	// ql_rtos_flag_create(&key_flag);
	//  ql_autosleep_enable(QL_ALLOW_SLEEP);
	ql_gpio_init(VOL_KEY_DOWN, PIN_DIRECTION_IN, PIN_PULL_PU, PIN_LEVEL_HIGH);
	ql_rtos_task_sleep_s(10);
	// ret = ql_eint_register(VOL_KEY_DOWN, PIN_FALLING_EDGE, PIN_PULL_PU, __gpio_irq_handler_test01, NULL);
	// // ret = ql_eint_register(VOL_KEY_DOWN, PIN_BOTH_EDGE, PIN_PULL_PU, __gpio_irq_handler_test01, NULL);
	// if (ret)
	// {
	// 	test_log("eint register failed, ret = %d\n", ret);
	// 	return;
	// }
	// ql_eint_enable(VOL_KEY_DOWN, PIN_BOTH_EDGE);
	// //ql_eint_enable_wakeup(VOL_KEY_DOWN, PIN_BOTH_EDGE);

	// ret = ql_eint_register(GPIO_PIN_NO_121, PIN_BOTH_EDGE, PIN_PULL_PU,NULL, __gpio_irq_handler_test02);
	// // ret = ql_eint_register(GPIO_PIN_NO_121, PIN_BOTH_EDGE, PIN_PULL_PU, __gpio_irq_handler_test02, NULL);
	// if (ret) {
	// 	test_log("eint register failed, ret = %d\n", ret);
	// 	return;
	// }
	// ql_eint_enable_wakeup(GPIO_PIN_NO_121, PIN_BOTH_EDGE);
	// // ql_eint_enable(GPIO_PIN_NO_121, PIN_BOTH_EDGE);

	// ret = ql_eint_register(GPIO_PIN_NO_11, PIN_BOTH_EDGE, PIN_PULL_PU, __gpio_irq_handler_test03, NULL);
	// if (ret) {
	// 	test_log("eint register failed, ret = %d\n", ret);
	// 	return;
	// }
	// ql_eint_enable(GPIO_PIN_NO_11, PIN_BOTH_EDGE);
	// ret = ql_eint_register(GPIO_PIN_NO_3, PIN_BOTH_EDGE, PIN_PULL_PU, __gpio_irq_handler_test04, NULL);
	// if (ret) {
	// 	test_log("eint register failed, ret = %d\n", ret);
	// 	return;
	// }
	// ql_eint_enable(GPIO_PIN_NO_3, PIN_BOTH_EDGE);
	// ret = ql_eint_register(GPIO_PIN_NO_4, PIN_BOTH_EDGE, PIN_PULL_PU, __gpio_irq_handler_test05, NULL);
	// if (ret) {
	// 	test_log("eint register failed, ret = %d\n", ret);
	// 	return;
	// }
	// ql_eint_enable(GPIO_PIN_NO_4, PIN_BOTH_EDGE);
	unsigned int pwrkey_ticks = 0;

	while (1)
	{
		PIN_LEVEL_E value;
		ql_gpio_get_level(VOL_KEY_DOWN, &value);
		// test_log("get gpio[%d] level%d \r\n", VOL_KEY_DOWN, value);
		ql_rtos_task_sleep_ms(50);

		if (value == PIN_LEVEL_LOW)
		{
			u32 press_duration = ql_rtos_get_systicks() - pwrkey_ticks;
			test_log("DA BAM \r\n");
			if (press_duration > 200)
			{ // 130*5=650ms,Quectel defined powerkey powndown 650ms
				test_log("Long press, ticks=%d \r\n", press_duration);

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
				ql_rtos_task_sleep_ms(500);

				// ql_power_down(1);
			}
			else
			{
				test_log("short press, ticks=%d \r\n", press_duration);
				PlayMP3File("volup.mp3");
				ql_rtos_task_sleep_ms(500);
			}
		}
		else if (value == PIN_LEVEL_HIGH)
		{
			// test_log("phim k bam \r\n");
			pwrkey_ticks = ql_rtos_get_systicks();
		}
		// int ret = ql_rtos_flag_wait(key_flag, QL_KEY_MASK_FLAG, QL_FLAG_OR_CLEAR, &event, QL_WAIT_FOREVER);
		// j++;
		// if (ret)
		// {
		// 	test_log("quec_rtos_flag_wait err!\r\n");
		// 	return;
		// }

		// if (KEY_HIGH_FLAG == event)
		// {
		// 	test_log("PWRKEY has been pressed! \r\n");
		// 	pwrkey_ticks = ql_rtos_get_systicks();
		// }
		// else if (KEY_LOW_FLAG == event)
		// {
		// 	u32 press_duration = ql_rtos_get_systicks() - pwrkey_ticks;
		// 	test_log("PWRKEY has been released! \r\n");
		// 	if (press_duration > 300)
		// 	{ // 130*5=650ms,Quectel defined powerkey powndown 650ms
		// 		test_log("pwrkey long press, ticks=%d \r\n", press_duration);
		// 		// ql_power_down(1);
		// 	}
		// 	else
		// 	{
		// 		test_log("pwrkey short press, ticks=%d \r\n", press_duration);
		// 	}
		// }
		// ql_rtos_task_sleep_s(3);
	}
}

application_init(quec_eint_test, "quec_eint_test", 10, 0);
// application_init(quec_gpio_task, "quec_gpio_test", 2, 0);
