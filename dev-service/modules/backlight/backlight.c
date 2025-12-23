#include "cmd_register.h"
#include "backlight.h"
#include "log.h"
#include "cJSON.h"
#include "cmd_table.h"
#include "rpc_server.h"

#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>
#include <math.h>

// 将0~100的逻辑值转换为0~255的实际值
static int logic_to_actual(int logical)
{
    if (logical < MIN_BRIGHTNESS_LOGIC) 
        logical = MIN_BRIGHTNESS_LOGIC;
    if (logical > MAX_BRIGHTNESS_LOGIC) 
        logical = MAX_BRIGHTNESS_LOGIC;

    return (int) round((logical / 100.0) * 255);
}

// 将0~255的实际值转换为0~100的逻辑值
static int actual_to_logic(int actual)
{
    if (actual < MIN_BRIGHTNESS_ACTUAL) 
        actual = MIN_BRIGHTNESS_ACTUAL;
    if (actual > MAX_BRIGHTNESS_ACTUAL) 
        actual = MAX_BRIGHTNESS_ACTUAL;
    return (int) round((actual / 255.0) * 100);
}

static int backlight_set(const char *path, int value)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        LOGE("Failed to open %s for writing", path);
        return -1;
    }

    fprintf(f, "%d", value);
    fclose(f);
    return 0;
}

static int backlight_get(const char *path)
{
    int v = 0;

    FILE *f = fopen(path, "r");
    if (!f)
    {
        LOGE("Failed to open %s for reading", path);
        return -1;
    }

    fscanf(f, "%d", &v);
    fclose(f);
    return v;
}

static rpc_result_t rpc_backlight_set(cJSON *params) 
{   
    // 请求
    /* 
    cmd：string；brightness.set；命令名称
    params
    | - value：int；0~100；要设置的亮度值
    */

    // 响应
    /* 
    status：int；0 表示成功，其它表示失败
    msg：string；提示信息
    data
    | - value：int；0~100；当前亮度值
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "read brightness failed, inspect log for details", 
        .data_json = NULL 
    };

     cJSON *value_item = cJSON_GetObjectItem(params, "value");
    if (!cJSON_IsNumber(value_item))
    {
        LOGE("Invalid or missing 'value' parameter");
        res.msg = "value parameter missing or invalid";
        return res;
    }

    int logical_value = value_item->valueint;
    if (logical_value < 0 || logical_value > 100)
    {
        LOGE("'value' parameter out of range: %d", logical_value);
        res.msg = "value parameter out of range (0-100)";
        return res;
    }
    int actual_value = logic_to_actual(logical_value);
    LOGD("Setting brightness, logical=%d, actual=%d", logical_value, actual_value);

    int ret = backlight_set(BRIGHTNESS_PATH, actual_value);
    if (ret != 0)
    {
        LOGE("Failed to set brightness value");
        return res;
    }

    // 读取实际值再转回逻辑值反馈
    int current_actual = backlight_get(BRIGHTNESS_PATH);
    if (current_actual < 0)
    {
        LOGE("Failed to read brightness value");
        return res;
    }
    int current_logic = actual_to_logic(current_actual);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "value", current_logic);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

static rpc_result_t rpc_backlight_get(cJSON *params)  
{
    // 请求
    /* 
    cmd：string；brightness.get；命令名称
    params：
    | - NULL
    */

    // 响应
    /* 
    status：int；0 表示成功，其它表示失败
    msg：string；提示信息
    data
    | - value：int；0~100；当前亮度值
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "read brightness failed, inspect log for details", 
        .data_json = NULL 
    };

    int actual = backlight_get(BRIGHTNESS_PATH);
    if (actual < 0)
    {
        LOGE("Failed to read brightness value");
        return res;
    }
    int logic = actual_to_logic(actual);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "value", logic);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

int backlight_cmd_register(void)
{
    int ret;

    command_t cmd_set_brightness, cmd_get_brightness;

    cmd_set_brightness.name = CMD_SET_BRIGHTNESS;
    cmd_set_brightness.handler = rpc_backlight_set;

    cmd_get_brightness.name = CMD_GET_BRIGHTNESS;
    cmd_get_brightness.handler = rpc_backlight_get;

    ret = command_register(&cmd_set_brightness);
    if (ret != 0) 
    {
        LOGE("Failed to register %s\n", CMD_SET_BRIGHTNESS);
        return -1;
    }

    ret = command_register(&cmd_get_brightness);
    if (ret != 0)
    {
        LOGE("Failed to register %s\n", CMD_GET_BRIGHTNESS);
        return -1;
    }

    return 0;
}