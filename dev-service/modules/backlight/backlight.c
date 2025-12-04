#include "cmd_register.h"
#include "backlight.h"
#include "log.h"
#include "cJSON.h"

#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>

static int write_int(const char *path, int value)
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

static int read_int(const char *path)
{
    int v = 0;

    FILE *f = fopen(path, "r");
    if (!f)
    {
        LOGE("Failed to open %s for writing", path);
        return -1;
    }

    fscanf(f, "%d", &v);
    fclose(f);
    return v;
}

static int backlight_set(const char *request, char **response) 
{   
    /* request example:     {"cmd": "CMD_SET_BRIGHTNESS","params": {"value": 200}}  */
    /* response example:    {"status": 0}                                           */

    char *error_response = "{\"status\":\"-1\"}";

    cJSON *root = cJSON_Parse(request);
    if (!root) 
    {
        LOGE("JSON parse failed in backlight_set");
        goto CJSON_ERROR;
    }

    /* inspect CMD */
    cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
    if (!cJSON_IsString(cmd)) 
    {
        LOGE("Invalid or missing cmd field in backlight_set");
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    const char *cmd_name = cmd->valuestring;
    if (strcmp(cmd_name, "CMD_SET_BRIGHTNESS") != 0) 
    {
        LOGE("Unexpected cmd name in backlight_set: %s", cmd_name);
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    /* get params */
    cJSON *params = cJSON_GetObjectItem(root, "params");
    if (!cJSON_IsObject(params)) 
    {
        LOGE("Invalid or missing params field in backlight_set");
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    cJSON *value_json = cJSON_GetObjectItem(params, "value");
    if (!cJSON_IsNumber(value_json))
    {
        LOGE("Invalid or missing value field in backlight_set");
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    int level = value_json->valueint;
    if (level < 0 || level > 255) 
    {
        LOGE("Brightness value out of range (0-255) in backlight_set");
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    cJSON_Delete(root);

    /* set level */
    int ret = write_int(BRIGHTNESS_PATH, level);
    LOGD("Set brightness to %d, ret=%d", level, ret);

    /* set response */
    if (ret == 0) 
    {
        const char *success_response = "{\"status\":\"0\"}";
        *response = strdup(success_response);
    } 
    else 
    {
        const char *error_response = "{\"status\":\"-1\"}";
        *response = strdup(error_response);
        return -1;
    }

    return 0;

CJSON_ERROR:
    *response = strdup(error_response);
    return -1;
}

static int backlight_get(const char *request, char **response)  
{
    /* request example:     {"cmd": "CMD_GET_BRIGHTNESS","params": {}}  */
    /* response example:    {"status": 0,"value": 200}                  */

    char *error_response = "{\"status\":\"-1\"}";

    cJSON *root = cJSON_Parse(request);
    if (!root) 
    {
        LOGE("JSON parse failed in backlight_get");
        goto CJSON_ERROR;
    }

    /* inspect CMD */
    cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
    if (!cJSON_IsString(cmd))
    {
        LOGE("Invalid or missing cmd field in backlight_get");
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    const char *cmd_name = cmd->valuestring;
    if (strcmp(cmd_name, "CMD_GET_BRIGHTNESS") != 0) 
    {
        LOGE("Unexpected cmd name in backlight_get: %s", cmd_name);
        cJSON_Delete(root);     
        goto CJSON_ERROR;
    }

    int ret = read_int(BRIGHTNESS_PATH);
    if (ret < 0)
    {
        LOGE("Failed to read brightness value in backlight_get");
        cJSON_Delete(root);
        goto CJSON_ERROR;
    }

    /* set response */
    char resp_buf[64];
    snprintf(resp_buf, sizeof(resp_buf), "{\"status\":\"0\",\"value\":%d}", ret);
    *response = strdup(resp_buf);

    cJSON_Delete(root);

    return 0;

CJSON_ERROR:
    *response = strdup(error_response);
    return -1;
}

int backlight_cmd_register(void)
{
    int ret;

    command_t cmd_set_brightness, cmd_get_brightness;

    cmd_set_brightness.name = "CMD_SET_BRIGHTNESS";
    cmd_set_brightness.handler = backlight_set;

    cmd_get_brightness.name = "CMD_GET_BRIGHTNESS";
    cmd_get_brightness.handler = backlight_get;

    ret = command_register(&cmd_set_brightness);
    if (ret != 0) 
    {
        LOGE("Failed to register CMD_SET_BRIGHTNESS\n");
        return -1;
    }

    ret = command_register(&cmd_get_brightness);
    if (ret != 0)
    {
        LOGE("Failed to register CMD_GET_BRIGHTNESS\n");
        return -1;
    }

    return 0;
}