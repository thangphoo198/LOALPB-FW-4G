/*================================================================
  Copyright (c) 2022, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_tts.h"
#include "ql_iic.h"
#include "ql_gpio.h"


static int rt5616_init_reg[] = {
	0x08080002,	//0002 is the addr, 0808 is the value
	0x08080003,
	0x80000005,
	0x04C0000D,
	0x0808000F,
	0xAFAF0019,
	0x5C5C001C,
	0x0000001E,
	0x38200027,
	0x80800029,
	0x1250002A,
	0x0000003B,
	0x004F003C,
	0x2000003D,
	0x004F003E,
	0x40000045,
	0x0000004D,
	0x0000004E,
	0x0278004F,
	0x00000050,
	0x00000051,
	0x02780052,
	0xC8000053,
	0x98060061,
	0x88000062,
	0xF8FE0063,
	0x0A000064,
	0xCC000065,
	0x3F000066,
	0x003D006A,
	0x3600006C,
	0x80820070,
	0x00000073,
	0x0C000074,
	0x50000080,
	0x0E1C0081,
	0x08000082,
	0x0019008E,
	0x3100008F,
	0x2C000093,
	0x02000094,
	0x208000B0,
	0x000000B1,
	0x001100FA,
};

static uint16  es8311_init_reg[] = {
	0x8000,  //00 is the addr, 0x80 is the value
	0x9f01,
	0x0002,
	0x1003,
	0x1004,
	0x0005,
	0x2006,
	0x0007,
	0xff08,
	0x0f09,
	0x0f0a,
	0x0a0b,
	0x6a0c,
	0x010d,
	0x020e,
	0x000f,
	0x1310,
	0x7f11,
	0x0012,
	0x1013,
	0x1a14,
	0x4015,
	0x2416,
	0xbf17,
	0x0018,
	0x0019,
	0x001a,
	0x0a1b,
	0x6a1c,
	0x001d,
	0x001e,
	0x001f,
	0x0020,
	0x0021,
	0x0022,
	0x0023,
	0x0024,
	0x0025,
	0x0026,
	0x0027,
	0x0028,
	0x0029,
	0x002a,
	0x002b,
	0x002c,
	0x002d,
	0x002e,
	0x002f,
	0x0030,
	0x0031,
	0xbf32,
	0x0033,
	0x0034,
	0x0035,
	0x0036,
	0x4837,
	0x0038,
	0x0039,
	0x003a,
	0x003b,
	0x003c,
	0x003d,
	0x003e,
	0x003f,
	0x0040,
	0x0041,
	0x0042,
	0x0043,
	0x0844,
};

#define RT5616_INIT_REG_LEN sizeof(rt5616_init_reg)/sizeof(rt5616_init_reg[0])
#define ES8311_INIT_REG_LEN sizeof(es8311_init_reg)/sizeof(es8311_init_reg[0])//xu

#define CODEC_IIC_NO 0	//EC600M iic_no 1/EC800M iic_no 0,根据项目修改
#define I2C_SLAVE_ADDR  0x1B	//codec 5616

#define ES8311_REG_I2C_DEVICEID     0xFD
#define ES8311_I2C_SLAVE_ADDR       0x18


static short ql_rt5616_read_reg(unsigned char RegAddr, unsigned short *p_value)
{
	unsigned short status = 1;
	unsigned char temp_buf[2];
	unsigned char retry_count = 5;

	do 
    {
        status = ql_i2c_read(CODEC_IIC_NO, I2C_SLAVE_ADDR, RegAddr, temp_buf, 2);
		if (status != 0)
		{
            printf("Error:rt5616 [%dth] device[0x%x] addr[0x%x] failed\n", retry_count, I2C_SLAVE_ADDR, RegAddr);
        }	
		else
        {
			*p_value = (((unsigned short)temp_buf[0]) << 8) | temp_buf[1];
			break;
		}
	} while (--retry_count);

	return status;
}

static short ql_rt5616_write_reg(unsigned char RegAddr, unsigned short RegData)
{
    unsigned short status = 1;
    unsigned char param_data[3] = {0x00};
    unsigned char retry_count = 5;

    param_data[0] = (unsigned char)((RegData >> 8) & 0xFF);
    param_data[1] = (unsigned char)(RegData & 0xff);

    do 
    {
        status = ql_i2c_write(CODEC_IIC_NO, I2C_SLAVE_ADDR, RegAddr, param_data, 2);
        if (status != 0)
        {
            printf("Error:[%dth] device[0x%x] addr[0x%x] failed\n", retry_count, I2C_SLAVE_ADDR, RegAddr);
        }        
        else 
        {
            break;
        }
    } while (--retry_count);

    return status;
}

static void ql_rt5616_init(void)
{
	char i = 0;
	
	for (i = 0; i < RT5616_INIT_REG_LEN; i++)
	{
		ql_rt5616_write_reg(rt5616_init_reg[i]&0xFFFF, rt5616_init_reg[i]>>16);
	}
}

static void ql_rt5616_open(void)		//初始化5616型号codec
{
	unsigned short status = 0;
	unsigned short  val = 0;
	ql_i2c_init(CODEC_IIC_NO, 0);		//不同项目的使用的IIC_NO有区别，硬件上也不相同，请根据具体项目更改
	status = ql_rt5616_read_reg(0xFE, &val);
	if ((status == 0) && (0x10EC == val)) 
	{
		ql_rt5616_init();
		printf("quec_rt5616_init!!!\n");
	}

}
static void ql_rt5616_close(void)		//给5616款codec下电
{
	static unsigned int reg_5616_power[] = {0x00000061, 0x00000062, 0x00000063, 0x00000064, 0x00000065, 0x00000066};
	char i = 0;
	
	for (i = 0; i < sizeof(reg_5616_power)/sizeof(reg_5616_power[0]); i++)
	{
		ql_rt5616_write_reg(reg_5616_power[i]&0xFFFF, reg_5616_power[i]>>16);
	}
}




static short ql_es8311_read_reg(unsigned char RegAddr, unsigned char *p_value)
{
	unsigned short status = 1;
	unsigned char temp_buf = 0;
	unsigned char retry_count = 5;

	do 
    {
        status = ql_i2c_read(CODEC_IIC_NO, ES8311_I2C_SLAVE_ADDR, RegAddr, &temp_buf, 1);
		if (status != 0)
		{
            printf("Error:es8311 [%dth] device[0x%x] addr[0x%x] failed\n", retry_count, ES8311_I2C_SLAVE_ADDR, RegAddr);
        }	
		else
        {
			*p_value = temp_buf;
			break;
		}
	} while (--retry_count);

	return status;
}


static short ql_es8311_write_reg(unsigned char RegAddr, unsigned char RegData)
{
    unsigned short status = 1;
    unsigned char retry_count = 5;
	unsigned char temp_buf = RegData;

    do 
    {
        status = ql_i2c_write(CODEC_IIC_NO, ES8311_I2C_SLAVE_ADDR, RegAddr, &temp_buf, 1);
        if (status != 0)
        {
            printf("Error:[%dth] device[0x%x] addr[0x%x] failed\n", retry_count, ES8311_I2C_SLAVE_ADDR, RegAddr);
        }        
        else 
        {
            break;
        }
    } while (--retry_count);

    return status;
}

static void ql_es8311_init(void)
{
	char i = 0;
	
	for (i = 0; i < ES8311_INIT_REG_LEN; i++)
	{
		ql_es8311_write_reg(es8311_init_reg[i]&0xFF, es8311_init_reg[i]>>8);
	}
}

static void ql_es8311_open(void)		//初始化es8311型号codec
{
	unsigned short status = 0;
	unsigned char  val = 0;
	ql_i2c_init(CODEC_IIC_NO, 0);		//不同项目的使用的IIC_NO有区别，硬件上也不相同，请根据具体项目更改
	status = ql_es8311_read_reg(ES8311_REG_I2C_DEVICEID, &val);
	if ((status == 0) && (0x83 == val)) 
	{
		ql_es8311_init();
		printf("quec_es8311_init!!!\n");
	}

}

static ql_cb_on_speakerpa play_audio_pa_cb(unsigned int on){
	if(on){
		printf("PA on\r\n");
		ql_gpio_init(GPIO_PIN_NO_74, PIN_DIRECTION_OUT, PIN_PULL_PU, PIN_LEVEL_HIGH);
	}else{
		printf("PA off\r\n");
		ql_gpio_init(GPIO_PIN_NO_74, PIN_DIRECTION_OUT, PIN_PULL_PD, PIN_LEVEL_LOW);
	}
}

static void quec_codec_test(void * argv)
{
	while(!ql_get_audio_state()){
		ql_rtos_task_sleep_ms(50);
	}
	
	printf("quec_codec_test start ...\r\n");
	
	int i = 0;
	ql_codecpcm_config pcm_config[] = {PCM_FS_256,PCM_CODEC_SAMPLE_8000,0};
	ql_set_audio_path_receiver();
	ql_set_volume(4);
	
	ql_bind_speakerpa_cb(play_audio_pa_cb);
	ql_codec_choose(AUD_EXTERNAL_CODEC, &pcm_config);	//模块内部配置后向外置codec输出pcm数据
	ql_es8311_open();									//初始化外置codec
	printf("choose external codec play audio ...\r\n");
	while(1){
		printf("play wav ...\r\n");
		ql_wav_file_play("U:/test.wav");
		ql_rtos_task_sleep_s(5);

		printf("play mp3 ...\r\n");
		ql_mp3_file_play("U:/test.mp3");
		ql_rtos_task_sleep_s(6);
	}

}

//application_init(quec_codec_test, "quec_codec_test", 3, 0);

