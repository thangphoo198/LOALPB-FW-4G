/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include <string.h>
#include "ql_bt.h"
#include "ql_rtos.h"
#include "ql_log.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_gpio.h"
#include "ql_bt_types.h"
#include "ql_fs.h"

typedef int (*le_att_event_handle_t)(struct bt_task_event * msg);

static struct bt_user_init_cfg init_cfg = {
    .name="QUECTEL_BT",
	.inquiry_scan_interval= 0x1000,
	.inquiry_scan_window=0x12,
	.page_scan_interval=0x1000,   //Range: 0x0012 to 0x1000; only even values are valid
	.page_scan_window=0x12,
	.io_capability=3,

};

static bt_uart_cfg ql_bt_uart_port =
{
    0,     //you can change it, if you use other UART port
    1,                  //do not change it
    0,                  //do not change it
    0,                  //do not change it
    bt_rx_int,          //do not change it
    0,                  //do not change it
    0,                  //do not change it
    BAUDRATE_115200,    //do not change it. it is a default baudrate. stack will change baudrate accroding bt_user_cfg
    BT_DATA_8,          //do not change it
    BT_PARITY_NONE,     //do not change it
    NULL,               //do not change it. host stack will use it
    NULL,               //do not change it. host stack will use it
};

struct bt_user_cfg cfg = {
    .bt_mode = BT_MODE_BR_LE,
    .h4_h5 = BT_HCI_MODE_H5,
    .firmware_where = BT_FIRMWARE_AT_FLASH,
    .hw.baudrate = 0,
    .hw.uart_cfg = &ql_bt_uart_port,
    .hw.reset_pin = -1,
    .hw.pwdn_pin = -1,
    .hw.host_wakeup_bt_pin = -1,
    .hw.bt_wakeup_host_pin = -1,
      .br_common.cod = 0x2540,
      .br_common.vohci = 0,
    .profile = {
        .hfp = 1,
        .a2dp = 1,
        .avrcp = 0,
        .spp = 1,
        .obex = 0,
        .hfp_hf = 0,
        .hid = 1,
        .a2dp_sink = 0,
    },
    .bt_chip_id = 5801,
    .xip = 0,
};

/****************************************************************************************************/

static ql_task_t p_quec_bt_task = NULL;
static bt_event_handle_t ql_bt_user_event_handler = NULL;
static void *ql_usr_task_ref = NULL;
static void * ql_usr_task_msgq = NULL;
#define MAX_BTUSRTASK_MSG_SIZE (sizeof(void *))
#define MAX_BTUSRTASK_MSGQ_SIZE 128

struct bt_app_context {
    struct bt_addr addr;
    int current_process;
    int status; // in process or not. 1: disconnect or connect; 2: disconnect and unbond
    int type; // 0: disconnect or 1: connect
};

static struct bt_app_context bt_connect_context = {0};
static struct audio_track_config wav_config;
static audio_track_handle wav_handle;
static Mp3PlayConfigInfo  mp3_configInfo;
static Mp3PlaybackHandle  mp3_handle;
static unsigned char is_wav_file_playing;
static unsigned char is_mp3_playing;
static void wav_play_callback(acm_audio_track_handle handle, acm_audio_track_event_t event)
{
    if(handle == wav_handle)
    {
        if(event == AUDIO_TRACK_EVENT_STARTED)
        {
            is_wav_file_playing = 1;
        }
        if(event == AUDIO_TRACK_EVENT_UNDERRUN)
        {
            is_wav_file_playing = 0;
        }
    }
    
}
static void ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if(event == MP3_PLAYBACK_EVENT_STATUS){
       if(val == MP3_PLAYBACK_STATUS_STARTED)
       {
            is_mp3_playing = 1;
       }
       if(val == MP3_PLAYBACK_STATUS_ENDED)
       {
            is_mp3_playing = 0;          
       }
    }
}

int user_wav_file_play(char *file_name)
{
    if(!file_name){
	    printf("%s:file_name NULL\r\n", __func__);
		return -1;
	}
    if(is_wav_file_playing == 1)
    {
       printf("%s:wav file is in playing\r\n", __func__);
       return -2; 
    }
    wav_config.event_cb = wav_play_callback;
	wav_config.prepare_threshold = 64;
	wav_config.option = 0x3;
    return ql_wav_play(file_name,&wav_config,&wav_handle); 
}

int user_mp3_file_play(char *file_name)
{
     if(!file_name){
		printf("mp3Start_nameNULL\n");
		return -1;
	}
	
    if(is_mp3_playing == 1) {
	    printf("error, mp3 is in playing\n");
        return -2;
    }
    mp3_configInfo.listener = ql_Mp3PlaybackEvent;
	mp3_configInfo.option = 0x3;
    ql_play_mp3(file_name,&mp3_handle,&mp3_configInfo);
}

int ql_event_handle(le_att_event_handle_t handle)
{
	ql_bt_user_event_handler = handle;
	return 0;
}
int ql_send_bt_event(struct bt_task_event *event)
{
    QuecOSStatus err = kNoErr; 

	if(ql_usr_task_msgq == NULL)
	{
		ql_bt_free(event);
		event = NULL;
		err = -1;
		ql_app_log("%s status:%d\r\n", __func__, err );
	}
	else
	{
		err = ql_rtos_queue_release(ql_usr_task_msgq, MAX_BTUSRTASK_MSG_SIZE, (void *)&event, 0);
		if(err != kNoErr)
		{
			ql_app_log("%s status:%d\r\n", __func__, err );
		}
	}

}

void ql_bt_usr_handle_acl(struct bt_task_event * msg)
{
    switch(msg->event_id)
    {
        case BTTASK_IND_ACL_CONNECTED:
        {
        	struct bt_addr addr;
            struct bt_event_acl_connect *acl = (struct bt_event_acl_connect *)msg->payload;
            ql_app_log("[QEUC_BT]handle ACL connection: %02x%02x%02x%02x%02x%02x @ %04x\r\n",
                        acl->addr[0], acl->addr[1], acl->addr[2], 
                        acl->addr[3], acl->addr[4], acl->addr[5],
                        acl->handle);

			ql_bt_sort_device_record(addr);
            break;
        }

        case BTTASK_IND_ACL_CONNECT_FAILED:
        {
            int *reason = (int *)msg->payload;
            ql_app_log("[QEUC_BT]handle ACL connect failed %d\r\n", *reason);
            break;
        }

        case BTTASK_IND_ACL_DISCONNECTED:
        {
            struct bt_event_acl_disconnect *acl = (struct bt_event_acl_disconnect *)msg->payload;
            ql_app_log("[QEUC_BT]handle ACL disconnect: handle %04x, reason %04x\r\n",
                        acl->handle, acl->reason);

            break;
        }

        default:
            ql_app_log("[QEUC_BT]handle unknow ACL event %d", msg->event_id);
            break;
    }
}

void ql_bt_usr_handle_common(struct bt_task_event * msg)
{
	char buffer[256] = {0};
    switch(msg->event_id)
    {
        case BTTASK_IND_INQUIRY_RESULT://扫描结果上报
        {
            struct bt_event_inquiry *inquiry = 
                          (struct bt_event_inquiry *)msg->payload;
#if 0//直接通过判断蓝牙地址来选择
			if((inquiry->addr[0] == 0x3d && inquiry->addr[5] == 0x34) || (inquiry->addr[0] == 0x2c && inquiry->addr[5] == 0x3c)
		    ||(inquiry->addr[0] == 0x34 && inquiry->addr[5] == 0x3d) || (inquiry->addr[0] == 0x3c && inquiry->addr[5] == 0x2c))
			{
                ql_bt_inquiry_cancel();
				memcpy(&bt_connect_context.addr, inquiry->addr, 6);
				bt_connect_context.status = 2;
			}
#else//通过比较蓝牙名称来获取地址
			ql_app_log("[QEUC_BT]handle inquiry result: %s, %02x%02x%02x%02x%02x%02x\r\n",
                            inquiry->name,
                            inquiry->addr[0], inquiry->addr[1], inquiry->addr[2], 
                            inquiry->addr[3], inquiry->addr[4], inquiry->addr[5]);
			if(memcmp(inquiry->name,"TWS",sizeof("TWS")) == 0){
				ql_bt_inquiry_cancel();//停止扫描
				memcpy(&bt_connect_context.addr, inquiry->addr, 6);
				bt_connect_context.status = 2;

			}
#endif
			break;
        }

        case BTTASK_IND_INQUIRY_COMPLETE:
        {
            ql_app_log("[QEUC_BT]handle inquiry complete\r\n");
            break;
        }

        case BTTASK_IND_PAIRING_REQUEST:
        {
            struct bt_event_pairing_request *request = 
                    (struct bt_event_pairing_request *)msg->payload;
            ql_app_log("[QEUC_BT]handle pairing request %02x%02x%02x%02x%02x%02x. "
                        "cod %08x. IO capability %d. Numeric %d\r\n",
                        request->addr.bytes[0], request->addr.bytes[1], 
                        request->addr.bytes[2], request->addr.bytes[3],
                        request->addr.bytes[4], request->addr.bytes[5], 
                        request->cod, request->io_capability, 
                        request->numeric_value);
            // user can reject or accept pairing request here
            break;
        }

        case BTTASK_IND_PAIRED:
        {
            // save link key, update record file
            struct bt_event_paired *paired = 
                        (struct bt_event_paired *)msg->payload;
            struct bt_device_record record;

            memset(&record, 0, sizeof(record));
            memcpy(record.addr, paired->addr, 6);
            memcpy(record.linkkey, paired->linkey, 16);
            memcpy(record.name, paired->name, MAX_BT_NAME);
            record.linkkey_type = paired->linkkey_type;
            record.cod = paired->cod;
            
             if((paired->addr[0] == 0x3d && paired->addr[5] == 0x34) || (paired->addr[0] == 0x2c && paired->addr[5] == 0x3c)
		    ||(paired->addr[0] == 0x34 && paired->addr[5] == 0x3d) || (paired->addr[0] == 0x3c && paired->addr[5] == 0x2c))
			{
                 ql_app_log("BTTASK_IND_PAIRED");
				 
			}
           
			ql_bt_add_device_record(&record);

            break;
        }
        case BTTASK_IND_NAME:
        {
            struct bt_event_name_indication *name =
                          (struct bt_event_name_indication *)msg->payload;

            if(name->name[0] > 0x7f)
            {
                int i = 0;
                ql_app_log("handle name indication result(chinese name)%d: %02x%02x%02x%02x%02x%02x\r\n",
                            name->name_length,
                            name->addr.bytes[0], name->addr.bytes[1], name->addr.bytes[2],
                            name->addr.bytes[3], name->addr.bytes[4], name->addr.bytes[5]);
                for(i = 0; i < name->name_length; i++)
                {
                    ql_app_log("%02x", name->name[i]);
                }
                ql_app_log("\r\n");
            }
            else
            {
                ql_app_log("handle name indication result %d: %s, %02x%02x%02x%02x%02x%02x\r\n",
                            name->name_length,
                            name->name,
                            name->addr.bytes[0], name->addr.bytes[1], name->addr.bytes[2],
                            name->addr.bytes[3], name->addr.bytes[4], name->addr.bytes[5]);
            }
            break;
        }

        case BTTASK_IND_RSSI:
        {
            signed char *value_rssi = (signed char *)msg->payload;
            ql_app_log("handle Read RSSI message %d\r\n", *value_rssi);
            break;
        }

        case BTTASK_IND_PIN_REQUEST:
        {
            struct bt_event_pin_request *temp = 
                    (struct bt_event_pin_request *)msg->payload;
            // user add pin code repley here
            char *test_pin_code = "0000";
            ql_app_log("[QEUC_BT]handle PIN code request. from device %s. cod %08x\r\n",
                        temp->name, temp->cod);

            break;
        }

        case BTTASK_IND_POWERUP_FAILED:
        {
            ql_app_log("[QEUC_BT]bluetooth power on failed\r\n");
            // should do something. Close BT?
            ql_bt_close();
            break;
        }

        case BTTASK_IND_POWERUP_COMPLETE:
        {
            ql_app_log("[QEUC_BT]bluetooth power on success\r\n");
            // user action. such as: connect to recent device
            bt_connect_context.status = 1;

            if(cfg.bt_mode == BT_MODE_BR_LE || cfg.bt_mode == BT_MODE_LE)
            {
                struct bt_addr addr = {0};
                ql_bt_get_bt_address(&addr);
                 ql_app_log("[QEUC_BT]ql_bt_get_bt_address %x %x %x %x %x %x\r\n",addr.bytes[0],addr.bytes[1],addr.bytes[2],addr.bytes[3],addr.bytes[4],addr.bytes[5]);
                addr.bytes[2] = (addr.bytes[2]+3);
                addr.bytes[3] = (addr.bytes[3]+3);
                addr.bytes[4] = (addr.bytes[4]+4);
                addr.bytes[5] = (addr.bytes[5]+1)|0xC2;
                ql_bt_le_set_random_address(addr);
            }
			break;
        }

        case BTTASK_IND_SHUTDOWN_COMPLETE:
        {
            struct bt_event_shutdown_complete *shutdown = 
                    (struct bt_event_shutdown_complete *)msg->payload;
            
            ql_app_log("[QEUC_BT]bluetooth power off complete\r\n");

            ql_bt_close_set();
            break;
        }
        default:
            ql_app_log("[QEUC_BT]handle unknow COMMON event %d", msg->event_id);
            break;
    }    
}



ql_flag_t a2dp_test_flag = NULL;
int appbt_a2dp_get_buffer_size(int rate, int chan, int* min, int* max)
{
    if(rate > 48000 || chan > 2 || !min || !max)
    {
        a2dp_debug("rate: %d, chan: %d, min: %p, max: %p\r\n",
                    rate, chan, min, max);
        return -1;
    }

    *min = 40 * rate / 1000 * chan * 2;  // 40ms data in bytes
    *max = 80 * rate / 1000 * chan * 2;  // 80ms data in bytes
    return 0;
}

static void a2dp_test_send_continue(void)
{
	int min_buf_size = 0, max_buf_size = 0, error_code = -1;
	ql_rtos_flag_release(a2dp_test_flag, 0x2, QL_FLAG_OR_CLEAR);
#if 1	
	appbt_a2dp_get_buffer_size(44100, 2, &min_buf_size, &max_buf_size);
	unsigned char* buf = (unsigned char*)malloc(min_buf_size);
    unsigned int buf_size = min_buf_size;
    error_code = ql_audio_track_pipeline_read(buf, &buf_size);
    if (error_code == 0) {
        ql_bt_a2dp_send_pcm_data(buf, buf_size);
    }
    free(buf);
#endif	
}

void ql_bt_usr_a2dp_handle(struct bt_task_event * msg)
{
	struct appbt_a2dp a2dp_info = {44100,2,a2dp_test_send_continue};
	ql_app_log("a2dp event:%d", msg->event_id);
    switch(msg->event_id)
    {
        case BTTASK_IND_A2DP_DISCONNECTED:
            ql_bt_a2dp_disconnected_handler();
            break;
		case BTTASK_IND_A2DP_CONNECTED:
			ql_bt_a2dp_connected_handler((unsigned char*)&bt_connect_context.addr);
            break;
		case BTTASK_IND_A2DP_MEIDA_CONNECTED:
            ql_bt_a2dp_send_start(&a2dp_info);
            break;
		case BTTASK_IND_A2DP_START:
			ql_bt_a2dp_start_handler();
			ql_rtos_flag_release(a2dp_test_flag, 0x2, QL_FLAG_OR_CLEAR);
            break;
		case BTTASK_IND_REMOTE_SBC_CAPABILITIES:
			ql_bt_a2dp_remote_sbc_capabilities_handler(*(unsigned int *)msg->payload);

			break;
		case BTTASK_IND_A2DP_START_PRIVATE:
			ql_bt_a2dp_start_handler();
            break;
        default:
            //ql_app_log("a2dp event not parsed:%d", msg->event_id);
            break;
    }
}


void ql_bt_usr_hfp_handle(struct bt_task_event * msg)
{
	ql_app_log("hfp event:%d", msg->event_id);
    switch(msg->event_id)
    {
        case BTTASK_IND_HFP_DISCONNECTED:
            ql_bt_hfp_disconnect();
            break;
		case BTTASK_IND_HFP_CONNECTED:
			ql_bt_hfp_connect(bt_connect_context.addr, 0);
            break;
        default:
            //ql_app_log("a2dp event not parsed:%d", msg->event_id);
            break;
    }
}

void ql_bt_usr_event_process(void)
{

    do{
        struct bt_task_event * msg = NULL;
		int err =0;
        err = ql_rtos_queue_wait(ql_usr_task_msgq, (void *)&msg, MAX_BTUSRTASK_MSG_SIZE, QL_WAIT_FOREVER);
        if(msg)
        {

	        switch(msg->event_type)
	        {
	            case BTTASK_IND_TYPE_COMMON:
	                ql_bt_usr_handle_common(msg);
	                break;
				case BTTASK_IND_TYPE_A2DP:
					ql_bt_usr_a2dp_handle(msg);
                    break;
				case BTTASK_IND_TYPE_HFP:
					ql_bt_usr_hfp_handle(msg);
                    break;
				case BTTASK_IND_TYPE_ACL:
	                ql_bt_usr_handle_acl(msg);
	                break;
	            default:
	                ql_app_log("[QEUC_BT][USER]handle event TYPE %d\r\n", msg->event_type);
	                break;
	        }
	        ql_bt_mgr_handler(msg);
	        ql_bt_free(msg);
		}
	}while(1);
}
static void ql_bt_process_handler(void)
{
	bt_connect_context.type = 1;
            
	if(bt_connect_context.status == 0 && bt_connect_context.current_process == 0)
	{
	 	ql_rtos_task_sleep_s(10);
		ql_bt_open(ql_bt_user_event_handler, &init_cfg, &cfg);
		bt_connect_context.current_process = 1;
	}

	if(bt_connect_context.status == 1 && bt_connect_context.current_process == 1)
	{
		struct bt_addr addr = {0};
		ql_bt_get_bt_address(&addr);
        ql_app_log("[QEUC_BT]get local bluetooth address: %02x%02x%02x%02x%02x%02x\r\n",
            addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3], addr.bytes[4], addr.bytes[5]);
		ql_bt_inquiry(0x30,8);//启动蓝牙扫描，扫描上限8个
      
		bt_connect_context.current_process = 2;
	}
	
	if(bt_connect_context.status == 2 && bt_connect_context.current_process == 2)
	{
        ql_app_log("[QEUC_BT]ql_bt_connect_headset address: %02x%02x%02x%02x%02x%02x\r\n",
            bt_connect_context.addr.bytes[0], bt_connect_context.addr.bytes[1], bt_connect_context.addr.bytes[2], bt_connect_context.addr.bytes[3], bt_connect_context.addr.bytes[4], bt_connect_context.addr.bytes[5]);

		ql_bt_connect_headset(bt_connect_context.addr,0);
		bt_connect_context.current_process = 3;
	}
  
}

static void ql_user_process_handler(void)
{
	QFILE *fp = NULL;
	int ret = 0,event;
	int min_buf_size = 0, max_buf_size = 0, error_code = -1;
	unsigned char *bt_buff = NULL;


	if(bt_connect_context.current_process == 3)
	{
		ql_rtos_flag_wait(a2dp_test_flag, 0x2, QL_FLAG_OR_CLEAR, &event, 0xffffffff);
		ql_app_log("to play: %d \r\n",event);
#if 0		
		fp = ql_fopen("U:/test.wav", "rb");
		ql_fseek(fp, 117, 0);
#if 0		
		appbt_a2dp_get_buffer_size(16000, 1, &min_buf_size, &max_buf_size);

		unsigned char *bt_buff = malloc(min_buf_size);
		if(bt_buff == NULL){
			ql_app_log("malloc fail \r\n");
			return;
		}
		while(1){
			ql_rtos_flag_wait(a2dp_test_flag, 0x1, QL_FLAG_OR_CLEAR, &event, 0xffffffff);
			memset(bt_buff, 0 ,min_buf_size);
			ret = ql_fread(bt_buff, min_buf_size, 1, fp);
			ql_app_log("read: %d \r\n",ret);
			if(ret <= 0){
				ql_app_log("read fail \r\n");
				break;
			}
			ql_bt_a2dp_send_pcm_data(bt_buff, ret);
		}
#else
			bt_buff = malloc(45*1024);
			if(bt_buff == NULL){
				ql_app_log("malloc fail \r\n");
				return;
			}
			memset(bt_buff, 0 ,45*1024);
			ret = ql_fread(bt_buff, 45*1024, 1, fp);
			ql_app_log("read: %d \r\n",ret);
			if(ret <= 0){
				ql_app_log("read fail \r\n");
				return;
			}
			ql_bt_a2dp_send_pcm_data(bt_buff, ret);
#endif
			
		
		ql_fclose(fp);
		ql_rtos_task_sleep_s(2);
		if(bt_buff){
			free(bt_buff);
		}
#else		
	while(1){
		printf("play wav ...\r\n");
		user_wav_file_play("U:/test.wav");
		ql_rtos_task_sleep_s(5);

		
		printf("play mp3 ...\r\n");
		user_mp3_file_play("U:/test.mp3");
		ql_rtos_task_sleep_s(5);
	}
#endif
		
	}

}

static void ql_bt_task(void)
{
  int ret = 0;
    while (1)
    {
		ql_bt_process_handler();
        ql_user_process_handler();
		ql_rtos_task_sleep_s(1);
        
    }  
}

QuecOSStatus ql_bt_a2dp_task_init(void * argv)
{
    QuecOSStatus err = kNoErr;    

	ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART|QL_LOG_PORT_USB);


    if (NULL != p_quec_bt_task)
    {
        ql_app_log("[QEUC_BT]ql_bt_a2dp_task_init is not NULL");
        return kGeneralErr;
    }

    /* create bt task */
	ql_rtos_flag_create(&a2dp_test_flag);
    err = ql_rtos_task_create(&p_quec_bt_task, 8192, 100, "quec_bt_task", ql_bt_task, NULL);
    if (kNoErr != err)
    {
    	p_quec_bt_task = NULL;
        ql_app_log("[QEUC_BT]ql_bt_a2dp_task_init init failed");
        return err;
    
}
	if(ql_usr_task_msgq == NULL)
	{
		err = ql_rtos_queue_create(&ql_usr_task_msgq, MAX_BTUSRTASK_MSG_SIZE, MAX_BTUSRTASK_MSGQ_SIZE);
		if(err != kNoErr)
		{
			ql_app_log("create user event queue failed\r\n");
			return -8;
		}
	}

	if(ql_usr_task_ref == NULL)
	{
		err = ql_rtos_task_create(&ql_usr_task_ref ,
							   4096,99,
							   "quec_BTUser", ql_bt_usr_event_process, 0);
		if(err != kNoErr)
		{
			ql_app_log("create user task failed\r\n");
			return -3;
		}
	}
	ql_event_handle(ql_send_bt_event);


    ql_app_log("[QEUC_BT]ql_bt_a2dp_task_init init done");
    
    return err;
}

//application_init(ql_bt_a2dp_task_init,"ql_bt_a2dp_task_init", 4, 0);
