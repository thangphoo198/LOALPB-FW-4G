#ifndef __main__h__
#define __main__h__
#include <stdio.h>
#include "ql_type.h"
#include "ql_gpio.h"
#define LED_STT GPIO_PIN_NO_6
#define STT GPIO_PIN_NO_70

#define VOL_KEY_DOWN GPIO_PIN_NO_9 // BOOT
#define WIFI_EN GPIO_PIN_NO_31 // BOOT
#define SERIAL_ADDRESS 0 //10 ki tu
#define VOLUME_ADDRESS 11 // 1
#define WIFISSID_ADDRESS 43 // max 32 ki tu
#define WIFIPASS_ADDRESS 75//

uint32_t start_ad;
u64 boot_time;
char mac_id[64];
char topic_rec[64];
char topic_remote[64];
char SN[64];
bool sn_mode; // =0 chua co serial, = 1 da co
 typedef enum
 {
	 NET_NONE		   = 0,
	 NET_LOW,
	 NET_MID,
	 NET_HIGH,

 } NET_LEVEL;
#endif