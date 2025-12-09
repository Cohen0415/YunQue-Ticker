#include "wifi.h"
#include "log.h"
#include "rpc_server.h"
#include "cmd_register.h"
#include "cmd_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int run_cmd(const char *cmd)
{
    return system(cmd);
}

int wifi_scan(wifi_ap_t *result, int max_result)
{
    if (!result) 
        return -1;

    // 确保网卡已开启
    system("ifconfig wlan0 up");

    // 停掉 wpa_supplicant 避免占用扫描
    system("killall wpa_supplicant 2>/dev/null");

    FILE *fp = popen("iw dev wlan0 scan 2>/dev/null", "r");
    if (!fp) 
    {
        LOGE("Failed to run iw scan command");
        return -1;
    }

    char line[256];
    int count = 0;
    wifi_ap_t tmp = {0};

    while (fgets(line, sizeof(line), fp) && count < max_result) 
    {
        if (strstr(line, "SSID:")) 
        {
            sscanf(line, "SSID: %63[^\n]", tmp.ssid);
        } 
        else if (strstr(line, "signal:")) 
        {
            sscanf(line, "signal: %d", &tmp.signal_level);
            result[count++] = tmp;
            memset(&tmp, 0, sizeof(tmp));
        }
    }

    pclose(fp);
    return count;
}

int wifi_connect(const char *ssid, const char *password)
{
    FILE *fp = fopen(WPA_CONF_PATH, "w");
    if (!fp) 
    {
        LOGE("Failed to open wpa_supplicant.conf for writing");
        return -1;
    }

    /*
        ctrl_interface=/var/run/wpa_supplicant
        ap_scan=1

        network={
            ssid="15C-room"
            psk="10338782"
            scan_ssid=1
            proto=RSN
            key_mgmt=WPA-PSK
            pairwise=CCMP
        }
    */
    fprintf(fp,
        "ctrl_interface=/var/run/wpa_supplicant\n"
        "ap_scan=1\n\n"
        "network={\n"
        "    ssid=\"%s\"\n"
        "    psk=\"%s\"\n"
        "    scan_ssid=1\n"
        "    proto=RSN\n"
        "    key_mgmt=WPA-PSK\n"
        "    pairwise=CCMP\n"
        "}\n",
        ssid, password
    );
    fclose(fp);

    run_cmd("killall wpa_supplicant 2>/dev/null");
    run_cmd("ifconfig wlan0 up");
    run_cmd("wpa_supplicant -i wlan0 -Dnl80211 -c /etc/wpa_supplicant.conf -B");
    return run_cmd("udhcpc -i wlan0 -n -q -t 10 -T 3 -A 5");
}

int wifi_disconnect(void)
{
    run_cmd("killall wpa_supplicant");
    return run_cmd("ifconfig wlan0 down");
}

int wifi_status(wifi_status_t *st)
{
    if (!st)
    {
        return -1;
    }

    memset(st, 0, sizeof(*st));
    st->connected = false;

    // 解析连接信息
    FILE *fp = popen("iw dev wlan0 link", "r");
    if (!fp) 
    {
        LOGE("Failed to run iw link command");
        return -1;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        char *p;

        if ((p = strstr(line, "SSID:"))) 
        {
            sscanf(p, "SSID: %63[^\n]", st->ssid);   // 读取整行SSID（支持含空格、特殊字符）
        }
        else if ((p = strstr(line, "signal:"))) 
        {
            sscanf(p, "signal: %d", &st->signal_level); // 读取 -69
        }
        else if (strstr(line, "Connected"))
            st->connected = true;
    }
    pclose(fp);

    // 如果未连接，直接返回
    if (!st->connected)
        return 0;

    // 解析 IP
    fp = popen("ip addr show wlan0 | grep 'inet ' | awk '{print $2}'", "r");
    if (fp && fgets(line, sizeof(line), fp))
    {
        sscanf(line, "%31s", st->ip);
        char *p = strchr(st->ip, '/');
        if (p) 
            *p = 0;
        pclose(fp);
        return 0;
    }
    LOGE("Failed to get IP address");
    
    return -1;
}

rpc_result_t rpc_wifi_scan(cJSON *params)
{
    /* 请求
        cmd：string；wifi.scan
        params：{}
    */

    /* 响应
        status：int 0成功
        msg：string
        data:
        | - list：array
            | - ssid：string
            | - signal：int(dBm)
            | - encrypted：bool
    */

    rpc_result_t res = {
        .status = -1, 
        .msg = "wifi scan failed", 
        .data_json = NULL 
    };

    wifi_ap_t list[32];
    int count = wifi_scan(list, 32);

    if (count < 0)
        return res;

    cJSON *data = cJSON_CreateObject();
    cJSON *arr  = cJSON_CreateArray();

    for (int i = 0; i < count; i++)
    {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "ssid", list[i].ssid);
        cJSON_AddNumberToObject(item, "signal", list[i].signal_level);
        cJSON_AddBoolToObject(item, "encrypted", list[i].encrypted);
        cJSON_AddItemToArray(arr, item);
    }

    cJSON_AddItemToObject(data, "list", arr);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);
    return res;
}

rpc_result_t rpc_wifi_connect(cJSON *params)
{
    /* 请求
        cmd：string；wifi.connect
        params：
        | - ssid：string
        | - password：string 可空(开放网络)
    */

    /* 响应
        status：int 0成功
        msg：string
        data：null
    */

    rpc_result_t res = {
        .status = -1, 
        .msg = "invalid param", 
        .data_json = NULL 
    };

    if (!params)
        return res;

    cJSON *ssid = cJSON_GetObjectItem(params, "ssid");
    cJSON *pwd  = cJSON_GetObjectItem(params, "password");

    if (!cJSON_IsString(ssid))
    {
        res.msg = "missing 'ssid'";
        return res;
    }

    const char *password = (cJSON_IsString(pwd)) ? pwd->valuestring : NULL;

    if (wifi_connect(ssid->valuestring, password) != 0)
    {
        res.msg = "wifi_connect failed";
        return res;
    }

    cJSON *data = cJSON_CreateObject();
    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);
    cJSON_Delete(data);

    return res;
}

rpc_result_t rpc_wifi_status(cJSON *params)
{
    /* 请求
        cmd：string；wifi.status
        params：{}
    */

    /* 响应
        status：0成功
        msg：string
        data:
        | - connected：bool
        | - ssid：string
        | - ip：string
        | - signal：int；信号强度 dBm
    */

    rpc_result_t res = {
        .status = -1,
        .msg = "status read failed",
        .data_json = NULL
    };

    wifi_status_t st;
    if (wifi_status(&st) != 0)
        return res;

    cJSON *data = cJSON_CreateObject();
    cJSON_AddBoolToObject(data, "connected", st.connected);
    cJSON_AddStringToObject(data, "ssid", st.connected ? st.ssid : "");
    cJSON_AddStringToObject(data, "ip", st.connected ? st.ip : "");
    cJSON_AddNumberToObject(data, "signal", st.connected ? st.signal_level : 0);

    res.status = 0;
    res.msg = "ok";
    res.data_json = cJSON_PrintUnformatted(data);

    cJSON_Delete(data);
    return res;
}

rpc_result_t rpc_wifi_disconnect(cJSON *params)
{
    /* 请求
        cmd：string；wifi.disconnect
        params：{}
    */

    /* 响应
        status：0成功
        msg：string
        data：{}
    */

    rpc_result_t res = {
        .status = -1,
        .msg = "disconnect failed",
        .data_json = NULL
    };

    if (wifi_disconnect() != 0)
        return res;

    res.status = 0;
    res.msg = "ok";
    res.data_json = strdup("{}");
    return res;
}

int wifi_cmd_register()
{
    int ret;

    command_t cmd_wifi_scan, cmd_wifi_connect, cmd_wifi_status, cmd_wifi_disconnect;

    cmd_wifi_scan.name = CMD_WIFI_SCAN;
    cmd_wifi_scan.handler = rpc_wifi_scan;

    cmd_wifi_connect.name = CMD_WIFI_CONNECT;
    cmd_wifi_connect.handler = rpc_wifi_connect;

    cmd_wifi_status.name = CMD_WIFI_STATUS;
    cmd_wifi_status.handler = rpc_wifi_status;

    cmd_wifi_disconnect.name = CMD_WIFI_DISCONNECT;
    cmd_wifi_disconnect.handler = rpc_wifi_disconnect;

    ret = command_register(&cmd_wifi_scan);
    if (ret != 0) 
        return ret;

    ret = command_register(&cmd_wifi_connect);
    if (ret != 0) 
        return ret;

    ret = command_register(&cmd_wifi_status);
    if (ret != 0) 
        return ret;
    
    ret = command_register(&cmd_wifi_disconnect);
    if (ret != 0) 
        return ret;
    
    return 0;
}