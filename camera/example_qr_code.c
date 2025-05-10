/*---------------------------------------------------------------------------
Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
Quectel Wireless Solution Proprietary and Confidential.
---------------------------------------------------------------------------*/

#include <stdio.h>
#include "ql_application.h"

#include "ql_rtos.h"
#include "ql_gpio.h"
#include "ql_camera.h"
#include "ql_uart.h"



#define APP_DEBUG(fmt, args...)	printf(fmt, ##args)
#define SERIAL_RX_BUFFER_LEN  2048
static u8 m_RxBuf_Uart1[SERIAL_RX_BUFFER_LEN];

#define DECODER_COUNT 20

static ql_timer_t timerRef;
bool camera_close = 0;
int camera_open = 1;
ql_task_t  taskRef;
#define __align(x) __attribute__ ((aligned (x)))

//static __align(32) unsigned char camdecodebuf[640*480*3/2*2]={0};
#define CAMERA_PREVIEW_TASK_PRIORITY	218 
#define CAMERA_DECODE_TASK_PRIORITY		227  
#define PREVIEW_BUF_CNT 2
void cam_callback_preview(P_CAL_IDENTITY_RESULT_STRUCT outdata);



static CAM_DECODE_CONFIG_CTX camconfig_test =
{
	.camwidth      =240,     //
	.camheight     =320,
	.dectaskprio   =CAMERA_DECODE_TASK_PRIORITY,
	.decbufcnt     =1,      //����buffer������
	.decbufaddr    =NULL,   //����buffer���׵�ַ
	.decodecb      =cam_callback_preview,   //����ɹ��Ļص�
	.preview       =0,      //�Ƿ���Ҫ����Ԥ��
	.pretaskprio   =CAMERA_PREVIEW_TASK_PRIORITY,//Ԥ����������ȼ�
	.prebufcnt     =PREVIEW_BUF_CNT, //Ԥ����buffer����
	.prebufaddr    =NULL,   //Ԥ����buffer�׵�ַ---����CameraԤ�����ݣ�NV12������
	.lcdmemeryaddr =NULL,   //LCD���Դ��׵�ַ,Ԥ����ʱ��,��Ҫ��Ԥ��������ת������buffer,600n�˴�����Ҫ����
	.lcdprewidth   =320,    //Ԥ��ͼ��Ŀ��
	.lcdpreheight  =240,    //Ԥ��ͼ��ĸ߶�
};

/******************************************************************
ENGLISH:Please do not run time-consuming functions in this function
���ģ��벻Ҫ�ڴ˺��������бȽϺ�ʱ�Ĵ�����ߺ���
���ɨ��ɹ����˻ص��������ᱻ����
attention:please don't "use ql_qr_camera_close" as callback in "ql_qr_camera_open".
ע�⣺��Ҫ��use ql_qr_camera_close������Ϊql_qr_camera_open�Ļص�������
�ص�����Ϊtask����
******************************************************************/
void cam_callback_preview(P_CAL_IDENTITY_RESULT_STRUCT outdata)
{
	printf("<-- result=%d\r\n",outdata->result);
	printf("<-- databuf=%s\r\n",outdata->DataBuf);
	//����ɹ���������Ϣ������ͨѶ�߳̽�������ͨ��,���������ϴ�����������
	//More.....
	//�ر�Camera
	if(outdata->result==0){
	camera_close =1;
    }
}

/*Fias.fan 2021.11.17 ���Խ��빦�ܺͲ���ƿ���*/
//#define TEST_FILL_LIGHT		//���Ե������忿������ͷʱ���򿪲����
static void qr_code_light_test(void *arg)
{
    int ret=0;   
	unsigned int value[10]={0};
	unsigned int threshold=0x5;		//��ֵ���ã���ǰ��������ڵ�ʱ�Ļ������ֵ���ɸ��ݶ�ȡ�Ĵ���0x14������ƽ��ֵ�������趨��
    unsigned int current_val=0,last_val=0,last_last_val=0;
	

	ql_rtos_task_sleep_s(5);			//����Ŀǰ����Ҫ��ʱ����cameraʱ���ʼ����
	printf("<----------Camera qr_code_light_test++++++++++++++>\n");
	ql_gpio_init(GPIO_PIN_NO_31, PIN_DIRECTION_OUT,PIN_PULL_DISABLE,PIN_LEVEL_HIGH);
	ql_gpio_set_level(GPIO_PIN_NO_31, PIN_LEVEL_HIGH);
	ql_gpio_init(GPIO_PIN_NO_32, PIN_DIRECTION_OUT,PIN_PULL_DISABLE,PIN_LEVEL_HIGH);
	ql_gpio_set_level(GPIO_PIN_NO_32, PIN_LEVEL_HIGH);

	//@20221208 ��ʼ��camera IO����ʹ�����ţ�EC600M��
	ql_gpio_init(GPIO_PIN_NO_77, PIN_DIRECTION_IN,PIN_PULL_PU,PIN_LEVEL_HIGH);
	/*�ر�TE-A�ϵĶ�λ�ƺͲ����*/
	// ql_gpio_init(GPIO_PIN_NO_33, PIN_DIRECTION_OUT,PIN_PULL_DISABLE,PIN_LEVEL_LOW);  //��λ��
	ql_gpio_init(GPIO_PIN_NO_36, PIN_DIRECTION_OUT,PIN_PULL_DISABLE,PIN_LEVEL_LOW);// �����
	//Ql_Cam_Identify(1);//�򿪽���
	ql_set_decode_mirror(AUTO_MODE);	 //�˽ӿڱ�ʾ�Զ����Ǿ���;����ά��
	 printf("[OPEN-START] Start Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());

	 ret = ql_qr_camera_open(&camconfig_test); 
       if(ret != CAM_DECODE_SUCESS)
        {
           printf("camera code open failed\r\n");
           printf("[OPEN-CAM] Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
        }
       else
        {
            printf("camera code open successd\r\n");
            printf("[OPEN-CAM]  Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
        }
	
	ql_rtos_task_sleep_s(5);	
	/*

	ret=ql_qr_camera_close();
	   if(ret != CAM_DECODE_SUCESS)
	   {
		  printf("camera code close failed��ret=%d\r\n",ret);
		  printf("[OPEN-CAM]  Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
	   }
	   else
	   {
			printf("camera code close success\r\n");
			printf("[OPEN-CAM]	Camera heap(%d)\r\n",ql_rtos_get_free_heap_size());
	   }
	 */ 


}

//application_init(qr_code_light_test, "qr_code_light_test", 2, 0);


