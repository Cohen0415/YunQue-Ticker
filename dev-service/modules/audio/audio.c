#include "audio.h"
#include "cJSON.h"
#include "log.h"
#include "rpc_server.h"
#include "cmd_register.h"
#include "cmd_table.h"

#include <stdio.h>
#include <alsa/asoundlib.h>

#define CARD_INDEX 0      // 对应 -c 0
#define NUMID_L 17        // DACL Volume
#define NUMID_R 18        // DACR Volume

// 设置音量 0~255
int audio_volume_set(int volume)
{
    if (volume < 0) 
    {
        LOGW("Volume less than 0, set to 0");
        volume = 0;
    }
    if (volume > 255) 
    {
        LOGW("Volume greater than 255, set to 255");
        volume = 255;
    }

    snd_ctl_t *ctl;
    snd_ctl_elem_value_t *ctl_value;

    char card_name[16];
    snprintf(card_name, sizeof(card_name), "hw:%d", CARD_INDEX);

    if (snd_ctl_open(&ctl, card_name, 0) < 0) 
    {
        LOGE("snd_ctl_open failed");
        return -1;
    }
    snd_ctl_elem_value_alloca(&ctl_value);

    // L 通道
    snd_ctl_elem_value_set_numid(ctl_value, NUMID_L);
    snd_ctl_elem_value_set_integer(ctl_value, 0, volume);
    if (snd_ctl_elem_write(ctl, ctl_value) < 0) 
    {
        LOGE("snd_ctl_elem_write L channel failed");
        goto FAIL;
    }

    // R 通道
    snd_ctl_elem_value_set_numid(ctl_value, NUMID_R);
    snd_ctl_elem_value_set_integer(ctl_value, 0, volume);
    if (snd_ctl_elem_write(ctl, ctl_value) < 0) 
    {
        LOGE("snd_ctl_elem_write R channel failed");
        goto FAIL;
    }

    snd_ctl_close(ctl);
    return 0;

FAIL:
    snd_ctl_close(ctl);
    return -1;
}

// 读取音量
int audio_volume_get(int *volume)
{
    if (!volume) 
        return -1;

    snd_ctl_t *ctl;
    snd_ctl_elem_value_t *ctl_value;
    char card_name[16];

    snprintf(card_name, sizeof(card_name), "hw:%d", CARD_INDEX);
    if (snd_ctl_open(&ctl, card_name, 0) < 0) 
    {
        LOGE("snd_ctl_open failed");
        return -1;
    }
    snd_ctl_elem_value_alloca(&ctl_value);

    snd_ctl_elem_value_set_numid(ctl_value, NUMID_L);
    if (snd_ctl_elem_read(ctl, ctl_value) < 0) 
    {
        LOGE("snd_ctl_elem_read failed");
        goto FAIL;
    }

    *volume = snd_ctl_elem_value_get_integer(ctl_value, 0);

    snd_ctl_close(ctl);
    return 0;

FAIL:
    snd_ctl_close(ctl);
    return -1;
}

int audio_play(const char *filename)
{
    if (!filename)
        return -1;

    // 检查文件是否存在
    if (access(filename, F_OK) != 0)
    {
        LOGE("Audio file not found: %s", filename);
        return -1;
    }

    // 检查 aplay 是否存在
    if (system("command -v aplay >/dev/null 2>&1") != 0) 
    {
        LOGE("aplay not found in PATH");
        return -1;
    }

    // 调用 aplay 播放
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "aplay %s &", filename);
    int ret = system(cmd);
    if (ret != 0) 
    {
        LOGE("Failed to execute aplay, ret=%d", ret);
        return -1;
    }

    return 0;
}

int audio_stop(void)
{
    int ret = system("pkill aplay");
    if (ret != 0) 
    {
        LOGE("Failed to stop aplay, ret=%d", ret);
        return -1;
    }
    return 0;
}

static rpc_result_t rpc_audio_volume_set(cJSON *params)
{
    // 请求
    /* 
        cmd：string；audio.volume.set；命令名称
        params
        | - volume：int；0~255；音量值（0静音，255最大）
    */
    // 响应
    /* 
        status：int；0 表示成功，其它表示失败
        msg：string；提示信息
        data
        | - NULL
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "invalid param", 
        .data_json = NULL 
    };

    if (!params) 
        return res;

    cJSON *vol_item = cJSON_GetObjectItem(params, "volume");
    if (!cJSON_IsNumber(vol_item)) 
    {
        res.msg = "Missing or invalid 'volume' parameter";
        return res;
    }

    int volume = vol_item->valueint;
    if (audio_volume_set(volume) != 0) 
    {
        res.msg = "audio_volume_set failed";
        return res;
    }

    cJSON *data = cJSON_CreateObject();
    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);
    return res;
}

static rpc_result_t rpc_audio_volume_get(cJSON *params)
{
    // 请求
    /* 
        cmd：string；audio.volume.get；命令名称
        params
        | - NULL
    */

    // 响应
    /* 
        status：int；0 表示成功，其它表示失败
        msg：string；提示信息
        data
        | - volume：int；0~255；当前音量
    */

    (void)params;

    rpc_result_t res = { 
        .status = -1, 
        .msg = "audio_volume_get failed", 
        .data_json = NULL 
    };

    int volume = 0;
    if (audio_volume_get(&volume) != 0) 
    {
        return res;
    }

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "volume", volume);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

static rpc_result_t rpc_audio_play(cJSON *params)
{
    // 请求
    /* 
        cmd：string；audio.play.set；命令名称
        params
        | - name：string；要播放的音频文件名称
    */

    // 响应
    /* 
        status：int；0 表示成功，其它表示失败
        msg：string；提示信息
        data
        | - NULL
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "invalid param", 
        .data_json = NULL 
    };

    if (!params) 
        return res;

    cJSON *name_item = cJSON_GetObjectItem(params, "name");
    if (!cJSON_IsString(name_item)) 
    {
        res.msg = "Missing or invalid 'name' parameter";
        return res;
    }

    if (audio_play(name_item->valuestring) != 0) 
    {
        res.msg = "audio_play failed";
        return res;
    }

    cJSON *data = cJSON_CreateObject();

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

static rpc_result_t rpc_audio_stop(cJSON *params)
{
    // 请求
    /* 
        cmd：string；audio.stop.set；命令名称
        params
        | - NULL
    */

    // 响应
    /* 
        status：int；0 表示成功，其它表示失败
        msg：string；提示信息
        data
        | - NULL
    */

    (void)params;

    rpc_result_t res = { 
        .status = -1, 
        .msg = "audio_stop failed", 
        .data_json = NULL 
    };

    if (audio_stop() != 0) 
    {
        return res;
    }

    cJSON *data = cJSON_CreateObject();

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

int audio_cmd_register()
{
    int ret;

    command_t cmd_volume_set, cmd_volume_get, cmd_play, cmd_stop;

    cmd_volume_set.name = CMD_AUDIO_VOLUME_SET;
    cmd_volume_set.handler = rpc_audio_volume_set;

    cmd_volume_get.name = CMD_AUDIO_VOLUME_GET;
    cmd_volume_get.handler = rpc_audio_volume_get;

    cmd_play.name = CMD_AUDIO_PLAY;
    cmd_play.handler = rpc_audio_play;

    cmd_stop.name = CMD_AUDIO_STOP;
    cmd_stop.handler = rpc_audio_stop;

    ret = command_register(&cmd_volume_set);
    if (ret != 0) 
        return ret;

    ret = command_register(&cmd_volume_get);
    if (ret != 0) 
        return ret;

    ret = command_register(&cmd_play);
    if (ret != 0) 
        return ret;

    ret = command_register(&cmd_stop);
    if (ret != 0) 
        return ret;

    return 0;
}