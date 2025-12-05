#include "cmd_register.h"
#include "backlight.h"
#include "log.h"
#include "cJSON.h"
#include "cmd_table.h"

#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>

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

static int rpc_backlight_set(cJSON *params, char **data_json) 
{   
    /* Just need to construct the "data" data. */
    /*
        "data": {}
    */

    cJSON *v = cJSON_GetObjectItem(params, "value");
    if (!cJSON_IsNumber(v))
        return -1;  // rpc层会自动构造错误返回

    int level = v->valueint;
    if (level < 0 || level > 255) 
    {
        LOGE("Brightness value out of range (0-255)");
        return -1;
    }

    /* set level */
    int ret = backlight_set(BRIGHTNESS_PATH, level);
    LOGD("Set brightness to %d, ret=%d", level, ret);

    *data_json = strdup("{}"); 
    return ret;
}

static int rpc_backlight_get(cJSON *params, char **data_json)  
{
    /* Just need to construct the "data" data. */
    /*
        "data": {
            "value": 200
        }
    */

    int ret = backlight_get(BRIGHTNESS_PATH);
    //int ret = -1;
    if (ret < 0)
    {
        LOGE("Failed to read brightness value");
        return -1;  // rpc层会自动构造错误返回
    }

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "value", ret);

    *data_json = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);
    return 0;
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