#include "sysinfo.h"
#include "cJSON.h"
#include "log.h"
#include "cmd_register.h"
#include "rpc_server.h"
#include "cmd_table.h"

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

static int sysinfo_get_version(char *buf, int buf_size)
{
    /*
        cat /proc/cpuinfo

        version : v1.0.0-21-g66826ff-dirty
        processor       : 0
        BogoMIPS        : 48.00
        Features        : fp asimd evtstrm aes pmull sha1 sha2 crc32 atomics fphp asimdhp cpuid asimdrdm lrcpc dcpop asimddp
        CPU implementer : 0x41
        CPU architecture: 8
        CPU variant     : 0x2
        CPU part        : 0xd05
        CPU revision    : 0

    */

    if (!buf || buf_size < 64)
        return -1;

    FILE *fp = fopen(VER_PATH, "r");
    if (!fp)
        return -1;

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "version", 7) == 0)
        {
            char *colon = strchr(line, ':');
            if (!colon)
                continue;

            char *p = colon + 1;
            // 去掉前导空格
            while (*p == ' ' || *p == '\t') p++;

            char version_tag[32] = {0};
            int distance = 0;

            // 解析 vX.Y.Z-DIST-gHASH[-dirty]
            // 先提取 tag
            char *dash1 = strchr(p, '-');
            if (dash1)
            {
                int tag_len = dash1 - p;
                if (tag_len >= (int)sizeof(version_tag))
                    tag_len = sizeof(version_tag) - 1;
                strncpy(version_tag, p, tag_len);
                version_tag[tag_len] = '\0';

                // 跳到 dash1 + 1
                char *dash2 = dash1 + 1;
                distance = atoi(dash2);  // 自动遇到非数字停止
            }
            else
            {
                // 没有 dash，直接就是 tag
                strncpy(version_tag, p, sizeof(version_tag) - 1);
                version_tag[sizeof(version_tag) - 1] = '\0';
            }
            
            char *last_dot = strrchr(version_tag, '.');
            if (last_dot && strcmp(last_dot, ".0") == 0)
                *last_dot = '\0';
            
            // 生成最终版本 vX.Y.N
            if (distance > 0)
                snprintf(buf, buf_size, "%s.%d", version_tag, distance);
            else
                snprintf(buf, buf_size, "%s", version_tag);

            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return -1;
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

static rpc_result_t rpc_sysinfo_get_version(cJSON *params)
{
    // 请求
    /* 
        cmd：string；sysinfo.version.get；命令名称
        params
        | - NULL
    */

    // 响应
    /* 
        cmd：string；返回请求时使用的命令
        status：int；0 表示成功，其它表示失败
        msg：string；提示信息
        data
        | - version：string；v1.0.0
    */

    rpc_result_t res = { 
        .status = -1, 
        .msg = "get system version failed, inspect log for details", 
        .data_json = NULL 
    };

    char version_buf[64];
    if (sysinfo_get_version(version_buf, sizeof(version_buf)) != 0)
    {
        LOGE("Failed to get system version");
        return res;
    }

    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data, "version", version_buf);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

int sysinfo_cmd_register()
{
    int ret;

    command_t cmd_get_bjtime, cmd_get_cpu_temp, cmd_get_version;

    cmd_get_bjtime.name = CMD_GET_BJTIME;
    cmd_get_bjtime.handler = rpc_sysinfo_get_bjtime;

    cmd_get_cpu_temp.name = CMD_GET_CPU_TEMP;
    cmd_get_cpu_temp.handler = rpc_sysinfo_get_cpu_temp;

    cmd_get_version.name = CMD_GET_VERSION;
    cmd_get_version.handler = rpc_sysinfo_get_version;

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

    ret = command_register(&cmd_get_version);
    if (ret != 0)
    {
        LOGE("Failed to register %s\n", cmd_get_version.name);
        return -1;
    }

    return 0;
}