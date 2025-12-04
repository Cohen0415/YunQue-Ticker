#include "cmd_register.h"
#include "backlight.h"
#include "log.h"

#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>

static int write_int(const char *path, int value)
{
    FILE *f = fopen(path, "w");
    if (!f) 
        return -1;

    fprintf(f, "%d", value);
    fclose(f);
    return 0;
}

static int read_int(const char *path)
{
    int v = 0;

    FILE *f = fopen(path, "r");
    if (!f) 
        return -1;

    fscanf(f, "%d", &v);
    fclose(f);
    return v;
}

static int backlight_set(char *value) 
{
    int level = atoi(value);
    if (level < 0 || level > 255) 
    {
        return -1;
    }

    return write_int(BRIGHTNESS_PATH, level);
}

static int backlight_get(char *value) 
{
    int ret;

    ret = read_int(BRIGHTNESS_PATH);
    if (ret < 0) 
    {
        return -1;
    }
    
    sprintf(value, "%d", ret);

    return 0;
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