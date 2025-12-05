#include "sysinfo.h"
#include "cJSON.h"
#include "log.h"
#include "cmd_register.h"
#include "rpc_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int sysinfo_get_bjtime(char *buf, int buf_size)
{
    if (!buf || buf_size < 25) 
        return -1;

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    if (!strftime(buf, buf_size, "%Y-%m-%dT%H:%M:%S+08:00", tm_info))
        return -1;

    return 0;
}

static int sysinfo_get_cpu_temp(void)
{
    int temp = 0;

    FILE *fp = fopen(CPU_TEMP_PATH, "r");
    if (!fp) 
        return -1;

    if (fscanf(fp, "%d", &temp) != 1)
        temp = -1;

    fclose(fp);
    return temp;   // 示例返回 45493
}

static rpc_result_t rpc_sysinfo_get_bjtime(cJSON *params)
{   
    // 请求
    /* 
    cmd：string；sysinfo.bjtime.get；命令名称
    params
    | - NULL
    */

    // 响应
    /* 
    status：int；0 表示成功，其它表示失败
    msg：string；提示信息
    data
    | - time：string；ISO 8601 标准格式
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "get Beijing time failed, inspect log for details", 
        .data_json = NULL 
    };

    char time_buf[32];
    if (sysinfo_get_bjtime(time_buf, sizeof(time_buf)) != 0)
    {
        LOGE("Failed to get Beijing time");
        return res;
    }
    LOGI("Beijing Time: %s", time_buf);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "time", time_buf);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

static rpc_result_t rpc_sysinfo_get_cpu_temp(cJSON *params)
{
    // 请求
    /* 
    cmd：string；sysinfo.temp.get；命令名称
    params
    | - NULL
    */

    // 响应
    /* 
    status：int；0 表示成功，其它表示失败
    msg：string；提示信息
    data
    | - temp：int；45493；除100就是当前温度，例如45.493℃
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "get CPU temperature failed, inspect log for details", 
        .data_json = NULL 
    };

    int temp = sysinfo_get_cpu_temp();
    if (temp < 0)
    {
        LOGE("Failed to get CPU temperature");
        return res;
    }
    LOGI("CPU Temperature: %d", temp);

    cJSON *data = cJSON_CreateObject();
    cJSON_AddNumberToObject(data, "temp", temp);
    
    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

int sysinfo_cmd_register()
{
    int ret;

    command_t cmd_get_bjtime, cmd_get_cpu_temp;

    cmd_get_bjtime.name = "sysinfo.bjtime.get";
    cmd_get_bjtime.handler = rpc_sysinfo_get_bjtime;

    cmd_get_cpu_temp.name = "sysinfo.temp.get";
    cmd_get_cpu_temp.handler = rpc_sysinfo_get_cpu_temp;

    ret = command_register(&cmd_get_bjtime);
    if (ret != 0) 
    {
        LOGE("Failed to register %s\n", cmd_get_bjtime.name);
        return -1;
    }

    ret = command_register(&cmd_get_cpu_temp);
    if (ret != 0)
    {
        LOGE("Failed to register %s\n", cmd_get_cpu_temp.name);
        return -1;
    }

    return 0;
}