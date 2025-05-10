/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
/**
 ******************************************************************************
 * @file    example_wav.c
 * @author  carola.zhang
 * @version V1.0.0
 * @date    2020/02/13
 * @brief   This file contains the audio play wav functions's definitions
 ******************************************************************************
 */
#include <string.h>
#include "ql_type.h"
#include "ql_func.h"
#include "ql_audio.h"
#include "ql_rtos.h"
#include "ql_fs.h"
#include "ql_application.h"
#include <stdio.h>
#include "Received_mp3.h"
#include "NUM_ONE_MOTJ_mp3.h"
#include "NUM_TWO_mp3.h"
#include "NUM_THREE_mp3.h"
#include "NUM_FOUR_BON_mp3.h"
#include "NUM_FIVE_NAM_mp3.h"
#include "NUM_UNIT_VND_mp3.h"
static Mp3PlaybackHandle mp3_handle;
static Mp3PlayConfigInfo mp3_configInfo;

static void ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if (event == MP3_PLAYBACK_EVENT_STATUS)
    {
        if (val == MP3_PLAYBACK_STATUS_STARTED)
        {
            printf("mp3 play start");
        }
        if (val == MP3_PLAYBACK_STATUS_ENDED)
        {
            printf("mp3 play end");
            ql_stop_mp3_play(mp3_handle, 0);
        }
    }
}
void play_mp3(const unsigned char *file_data, size_t data_size)
{
    if (file_data == NULL || data_size == 0)
    {
        printf("Error: Invalid MP3 data.\r\n");
        return;
    }

    int ret = ql_play_mp3_stream_buffer(mp3_handle, file_data, data_size);
    printf("Playing MP3, ret = %d ...\r\n", ret);

    while (ret == -1)
    {
        printf("Error: Failed to play MP3. Retrying...\r\n");
        // ret = ql_play_mp3_stream_buffer(mp3_handle, file_data, data_size);
        ql_rtos_task_sleep_ms(100); // Thêm độ trễ để tránh lặp vô tận
        break;
    }
}

#define FILE_NAME "wel2.mp3"

#define U_DISK "U:"
#define U_ROOT_PATH "" U_DISK "/"
#define U_FILE_UNDER_ROOT "" U_DISK "/" FILE_NAME ""

static void list_dir(const char *path)
{
    QDIR *dp = NULL;
    struct lfs_info info = {0};
    int ret;

    dp = ql_opendir(path);
    if (dp == NULL)
    {
        printf("[FS] *** dir open fail: %s ***\r\n", path);
        return;
    }
    printf("[FS] dir opened: %s\r\n", path);

    printf("[FS] ----- start dir list -----\r\n");
    printf("[FS] type\tsize\tname\r\n");
    while (1)
    {
        ret = ql_readdir(dp, &info);
        if (ret < 0)
        {
            printf("[FS] *** dir read fail: %s ***\r\n", path);
            break;
        }
        else if (ret == 0)
        {
            printf("[FS] ----- end of dir list -----\r\n");
            break;
        }
        else
        {
            printf("[FS] %-4d\t%-4d\t%s\r\n", info.type, info.size, info.name);
        }
    }

    if (dp)
    {
        ql_closedir(dp);
        printf("[FS] dir closed: %s\r\n", path);
    }
}
void mp3_stream(void *argv)
{
    // �ȴ�audio��ʼ����
    while (!ql_get_audio_state())
    {
        ql_rtos_task_sleep_ms(5);
    }

    printf("mp3_test start ...\r\n");
    ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
    ql_set_audio_path_earphone();
    // sql_bind_speakerpa_cb(qlasdasd_cb_on_speakerpa);
    ql_set_volume(10);
    mp3_configInfo.listener = ql_Mp3PlaybackEvent;
    /*������ʱfile_nameΪ��*/
    ql_play_mp3(0, &mp3_handle, &mp3_configInfo);
    int ret = ql_play_mp3(U_FILE_UNDER_ROOT, &mp3_handle, &mp3_configInfo);
    printf("ql_mp3_file_play ret = %d\r\n", ret);
    ql_rtos_task_sleep_s(5);
    while (1)
    {
        // play_mp3(Received_mp3, sizeof(Received_mp3));
        // play_mp3(NUM_ONE_MOTJ_mp3, sizeof(NUM_ONE_MOTJ_mp3));
        // play_mp3(NUM_TWO_mp3, sizeof(NUM_TWO_mp3));
        // play_mp3(NUM_THREE_mp3, sizeof(NUM_THREE_mp3));
        // play_mp3(NUM_FOUR_BON_mp3, sizeof(NUM_FOUR_BON_mp3));
        // play_mp3(NUM_FIVE_NAM_mp3, sizeof(NUM_FIVE_NAM_mp3));
        // play_mp3(NUM_UNIT_VND_mp3, sizeof(NUM_UNIT_VND_mp3));
        int ret = ql_play_mp3(U_FILE_UNDER_ROOT, &mp3_handle, &mp3_configInfo);
        printf("ql_mp3_file_play ret = %d\r\n", ret);
        ret = ql_play_mp3("U:/01.mp3", &mp3_handle, &mp3_configInfo);
        printf("ql_mp3_file_play ret = %d\r\n", ret);
        ret = ql_play_mp3("U:/02.mp3", &mp3_handle, &mp3_configInfo);
        ret = ql_play_mp3("U:/02.mp3", &mp3_handle, &mp3_configInfo);
        ret = ql_play_mp3("U:/03.mp3", &mp3_handle, &mp3_configInfo);
        ret = ql_play_mp3("U:/dong.mp3", &mp3_handle, &mp3_configInfo);
        printf("ql_mp3_file_play ret = %d\r\n", ret);
        // ql_rtos_task_sleep_s(5);
        //  int ret = ql_play_mp3_stream_buffer(mp3_handle, Received_mp3, sizeof(Received_mp3));
        //  printf("ret %d ...\r\n", ret);
        //  ret = ql_play_mp3_stream_buffer(mp3_handle, NUM_TWO_mp3, sizeof(NUM_TWO_mp3));

        // printf("ret %d ...\r\n", ret);
        // ret = ql_play_mp3_stream_buffer(mp3_handle, NUM_THREE_mp3, sizeof(NUM_THREE_mp3));

        // printf("ret %d ...\r\n", ret);
        // ret = ql_play_mp3_stream_buffer(mp3_handle, NUM_FOUR_BON_mp3, sizeof(NUM_FOUR_BON_mp3));

        // printf("ret %d ...\r\n", ret);
        // ret = ql_play_mp3_stream_buffer(mp3_handle, NUM_FIVE_NAM_mp3, sizeof(NUM_FIVE_NAM_mp3));

        // printf("ret %d ...\r\n", ret);
        // ql_rtos_task_sleep_s(2);

        // ql_play_mp3_stream_buffer(mp3_handle, esedbby_map, sizeof(esedbby_map));
        // ql_rtos_task_sleep_s(2);

        // ql_play_mp3_stream_buffer(mp3_handle, dqdlbfzyb_map, sizeof(dqdlbfzyb_map));

        // ql_rtos_task_sleep_s(2);

        // ql_play_mp3_stream_buffer(mp3_handle, dqdlbfzyb_map, 5 * 1024);

        // ql_rtos_task_sleep_s(2);
        // ql_play_mp3_stream_buffer(mp3_handle, dqdlbfzyb_map + 5 * 1024, sizeof(dqdlbfzyb_map) - 5 * 1024);
    }
}

//application_init(mp3_stream, "mp3_stream_test", 5, 2);

#define FILE_PATH "U:/testamr.amr"

static void quec_AmrPlaybackEvent(AmrPlaybackEventType event, int val)
{
    if (event == AMR_PLAYBACK_EVENT_STATUS)
    {
        if (val == AMR_FILE_STATUS_STARTED)
        {
            printf("amr play start\r\n");
        }
        if (val == AMR_FILE_STATUS_ENDED)
        {
            printf("amr play end\r\n");
        }
    }
}

void ql_amr_stream_test(void *argv)
{
    int i = 0;
    int size = 0;
    QFILE *fp;
    unsigned char *buffer = malloc(4096);
    AmrPlaybackHandle handle;
    AmrPlaybackConfigInfo amr_configInfo;
    memset(&amr_configInfo, 0, sizeof(amr_configInfo));
    ql_rtos_task_sleep_s(10);
    while (!ql_get_audio_state())
    {
        ql_rtos_task_sleep_ms(50);
    }
    ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
    ql_set_audio_path_receiver();
    ql_set_volume(10);
    printf("quec amr stream test start\r\n");

    amr_configInfo.listener = quec_AmrPlaybackEvent;
    ql_play_amr(0, &handle, &amr_configInfo); // ��ʼ������ͨ����filename=0
    fp = ql_fopen(FILE_PATH, "rb");
    do
    {
        size = ql_fread(buffer, 4096, 1, fp);
        ql_play_amr_stream_buffer(handle, buffer, 4096);
        ql_rtos_task_sleep_ms(20);
    } while (size > 0);
    ql_stop_amr_play(handle, 1); // drain=1,�ȴ������ݲ�����Ϻ�ֹͣ
    printf("stop!!");
    ql_fclose(fp);
}

// application_init(ql_amr_stream_test, "amr_stream_test", 5, 2);