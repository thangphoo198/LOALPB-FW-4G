#include <stdio.h>
#include <string.h>
#include "mp3.h"
#include "name_mp3.h"
#include "ql_audio.h"
#include "ql_fs.h"
#include "ql_rtos.h"
#include "ql_application.h"
#include "ql_uart.h"

#define FILE_NAME "mp3test.mp3"

#define U_DISK "U:"
#define U_ROOT_PATH "" U_DISK "/"
#define U_FILE_UNDER_ROOT "" U_DISK "/" FILE_NAME ""

static Mp3PlaybackHandle mp3_handle;
static Mp3PlayConfigInfo mp3_configInfo;

void list_dir(const char *path)
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
void qlasdasd_cb_on_speakerpa(unsigned int on)
{
    printf("pa %d\r\n", on);
}

signed short DSP_gain[12] = {-36, -31, -27, -24, -21, -18, -15, -12, -9, -6, -3, 0};
static volatile int mp3_play_stop = 0;
static void ql_Mp3PlaybackEvent(Mp3PlayEventType event, int val)
{
    if (event == MP3_PLAYBACK_EVENT_STATUS)
    {
        if (val == MP3_PLAYBACK_STATUS_STARTED)
        {
            printf("MP3 playback started\n");
        }
        else if (val == MP3_PLAYBACK_STATUS_ENDED)
        {
            printf("MP3 playback ended\n");
            ql_stop_mp3_play(mp3_handle, 0); // Dừng phát khi hoàn tất
            mp3_play_stop = 0;
        }
    }
}

// Hàm phát MP3
void Play_mp3(char *name)
{
    while (mp3_play_stop)
    {
        //printf("MP3 is already playing, please wait until it finishes.\n");
        ql_rtos_task_sleep_ms(50);
        // return;
    }

    // Cấu hình sự kiện phản hồi
    mp3_configInfo.listener = ql_Mp3PlaybackEvent;
    mp3_configInfo.sync_mode = 0;

    printf("Attempting to play MP3: %s\n", name);

    // Bắt đầu phát tệp MP3
    int ret = ql_play_mp3(name, &mp3_handle, &mp3_configInfo);
    if (ret == 0)
    {
        mp3_play_stop = 1;
        printf("Playing MP3: %s\n", name);
    }
    else
    {
        printf("Failed to play MP3: %s (Error Code: %d)\n", name, ret);
    }
}

void mp3_init()
{
    int ret = 0;
    int size = 0;
    // QFILE *fp;
    // unsigned char *buffer = malloc(8 * 4096);
    Mp3PlaybackHandle handle;
    Mp3PlayConfigInfo mp3_configInfo;
    memset(&mp3_configInfo, 0, sizeof(mp3_configInfo));
    // list "U:/"
    list_dir(U_ROOT_PATH);
    while (!ql_get_audio_state())
    {
        ql_rtos_task_sleep_ms(50);
    }
    ql_codec_choose(AUD_INTERNAL_CODEC, NULL);
    ql_set_audio_path_receiver();
    // ql_set_rxcodec_gain_table(1, 0, 36, 1, 1, 0, 2);
    // ql_set_rxcodec_gain_table(1, 1, 36, 1, 1, 0, 2);
    // ql_set_rxcodec_gain_table(1, 2, 36, 1, 2, 0, 2);
    // ql_set_rxcodec_gain_table(1, 3, 36, 1, 2, 0, 2);
    // ql_set_rxcodec_gain_table(1, 4, 36, 1, 3, 0, 2);
    // ql_set_rxcodec_gain_table(1, 5, 36, 1, 3, 1, 2);
    // ql_set_rxcodec_gain_table(1, 6, 36, 1, 4, 1, 2);
    // ql_set_rxcodec_gain_table(1, 7, 36, 1, 4, 1, 2);
    // ql_set_rxcodec_gain_table(1, 8, 36, 1, 4, 1, 2);
    // ql_set_rxcodec_gain_table(1, 9, 36, 1, 4, 1, 2);
    // ql_set_rxcodec_gain_table(1, 10, 36, 1, 4, 1, 2);
    // ql_set_rxcodec_gain_table(1, 11, 36, 1, 4, 1, 2);
    //ql_set_dsp_gain_table(1, DSP_gain);
    ql_bind_speakerpa_cb(qlasdasd_cb_on_speakerpa);
    ql_set_volume(AUDIOHAL_SPK_VOL_QTY);
    printf("quec mp3 test start\r\n");
    PlayMP3File(welcome);
} 