#include <stdio.h>     
#include <stdlib.h>     
#include <string.h>

#include "cmd_register.h"
#include "backlight.h"

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

    ret = register_command("CMD_SET_BRIGHTNESS", backlight_set);
    if (ret != 0) 
    {
        printf("Failed to register CMD_SET_BRIGHTNESS\n");
        return -1;
    }

    ret = register_command("CMD_GET_BRIGHTNESS", backlight_get);
    if (ret != 0)
    {
        printf("Failed to register CMD_GET_BRIGHTNESS\n");
        return -1;
    }

    return 0;
}