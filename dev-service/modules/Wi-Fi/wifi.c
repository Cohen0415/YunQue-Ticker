#include "wifi.h"
#include "log.h"
#include "rpc_server.h"
#include "cmd_register.h"
#include "cmd_table.h"
#include "cJSON.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h> 
#include <poll.h>    

static struct wpa_ctrl *g_wpa_ctrl = NULL;          // Control connection for commands
static struct wpa_ctrl *g_wpa_ctrl_event = NULL;    // Separate connection for events

static pthread_t g_event_thread = 0;
static bool g_event_thread_running = false;
static pthread_mutex_t g_wifi_mutex = PTHREAD_MUTEX_INITIALIZER; 

static wpa_wifi_conn_status_t g_connect_status = WPA_WIFI_INACTIVE;

static bool g_wifi_initialized = false;

static int wifi_open_connections(const char *ifname)
{
    char ctrl_path[128];
    struct wpa_ctrl *tmp_ctrl, *tmp_ctrl_event;

    // 命令连接接口，用于 wpa_cli 向 wpa_supplicant 发送命令
    snprintf(ctrl_path, sizeof(ctrl_path), WIFI_CTRL_PATH_FORMAT, ifname);
    tmp_ctrl = wpa_ctrl_open(ctrl_path);
    if (!tmp_ctrl) 
    {
        LOGE("Failed to open wpa_supplicant command interface at %s", ctrl_path);
        return -1;
    }

    // 事件连接接口
    tmp_ctrl_event = wpa_ctrl_open(ctrl_path);
    if (!tmp_ctrl_event) 
    {
        LOGE("Failed to open wpa_supplicant event interface at %s", ctrl_path);
        wpa_ctrl_close(tmp_ctrl);
        tmp_ctrl = NULL;
        return -1;
    }

    // 使用 g_wpa_ctrl_event 来监听 wpa_supplicant 发出的事件
    if (wpa_ctrl_attach(tmp_ctrl_event) < 0) 
    {
        LOGE("Failed to attach to wpa_supplicant event interface.");
        wpa_ctrl_close(tmp_ctrl_event);
        tmp_ctrl_event = NULL;
        wpa_ctrl_close(tmp_ctrl);
        tmp_ctrl = NULL;
        return -1;
    }

    pthread_mutex_lock(&g_wifi_mutex);
    g_wpa_ctrl = tmp_ctrl;
    g_wpa_ctrl_event = tmp_ctrl_event;
    pthread_mutex_unlock(&g_wifi_mutex);

    return 0;
}

static int wifi_send_cmd(const char *cmd, char *reply, size_t *reply_len)
{
    int ret;

    pthread_mutex_lock(&g_wifi_mutex);

    if (!g_wpa_ctrl)
    {
        LOGE("wpa_ctrl command handle is NULL");
        pthread_mutex_unlock(&g_wifi_mutex);
        return -1;
    }

    ret = wpa_ctrl_request(g_wpa_ctrl, cmd, strlen(cmd), reply, reply_len, NULL);
    pthread_mutex_unlock(&g_wifi_mutex);

    if (ret < 0) 
    {
        LOGE("wpa_ctrl_request failed for command: %s", cmd);
        return -1;
    }
    reply[*reply_len] = '\0'; 

    return 0;
}

static void wifi_parse_status(const char *status_reply)
{
    pthread_mutex_lock(&g_wifi_mutex);

    // 解析 Wi-Fi 连接状态
    if (strstr(status_reply, "wpa_state=COMPLETED") != NULL) 
    {
        g_connect_status = WPA_WIFI_CONNECT;
    } 
    else if (strstr(status_reply, "wpa_state=DISCONNECTED") != NULL) 
    {
        g_connect_status = WPA_WIFI_DISCONNECT;
    } 
    else if (strstr(status_reply, "wpa_state=SCANNING") != NULL) 
    {
        g_connect_status = WPA_WIFI_SCANNING;
    } 
    else if (strstr(status_reply, "wpa_state=INACTIVE") != NULL) 
    {
        g_connect_status = WPA_WIFI_INACTIVE;
    } 
    else 
    {
        g_connect_status = WPA_WIFI_UNKNOWN;
        LOGW("Unknown or unhandled wpa_state in STATUS reply: %s", status_reply);
    }

    pthread_mutex_unlock(&g_wifi_mutex);
}

static void wifi_update_status()
{
    char reply_buf[WIFI_MAX_REPLY_LEN];
    size_t reply_len = sizeof(reply_buf);

    if (wifi_send_cmd("STATUS", reply_buf, &reply_len) == 0) 
    {
        wifi_parse_status(reply_buf);
        LOGD("Updated Wi-Fi Connection Status");
    } 
    else 
    {
        LOGE("Failed to get STATUS from wpa_supplicant.");
    }
}

static void wifi_handle_event(const char *event)
{
    LOGI("Received Wi-Fi Event: %s", event);

    // Wi-Fi 连接成功
    if (strstr(event, "CTRL-EVENT-CONNECTED") != NULL)  
    {
        LOGI("CTRL-EVENT-CONNECTED Detected.");

        char dhcp_cmd[128];
        snprintf(dhcp_cmd, sizeof(dhcp_cmd), WIFI_UDHCPC_CMD_FORMAT, WIFI_DEV_IFNAME);
        LOGI("Triggering DHCP: %s", dhcp_cmd);
        int dhcp_res = system(dhcp_cmd);

        char save_reply[WIFI_MAX_REPLY_LEN];
        size_t save_len = sizeof(save_reply);
        if (wifi_send_cmd("SAVE_CONFIG", save_reply, &save_len) == 0) 
        {
            LOGI("Configuration saved.");
        } 
        else 
        {
            LOGW("Failed to save configuration after connect.");
        }
    } 
    // Wi-Fi 断开连接
    else if (strstr(event, "CTRL-EVENT-DISCONNECTED") != NULL)
    {
        LOGI("CTRL-EVENT-DISCONNECTED Event Detected.");
    } 
    // Wi-Fi 密码错误
    else if (strstr(event, "CTRL-EVENT-SSID-TEMP-DISABLED") != NULL)
    {
        LOGW("CTRL-EVENT-SSID-TEMP-DISABLED Event Detected.");
    } 
    // Wi-Fi 终端终止
    else if (strstr(event, "CTRL-EVENT-TERMINATING") != NULL)
    {
        LOGW("CTRL-EVENT-TERMINATING Event Detected.");
    }
    // Wi-Fi 扫描结果
    else if (strstr(event, "CTRL-EVENT-SCAN-RESULTS") != NULL)
    {
        LOGW("CTRL-EVENT-SCAN-RESULTS Event Detected.");
    }
    // 其他 Wi-Fi 事件
    else 
    {
        LOGD("Unhandled Wi-Fi Event: %s", event);
    }

    // 更新连接状态
    wifi_update_status();
}

static void *wifi_event_thread_func(void *arg)
{
    char event_buf[WIFI_EVENT_BUF_SIZE];
    size_t event_len;
    struct pollfd pfd;
    int ret;

    LOGI("Wi-Fi Event Thread Started.");

    while (1)
    {   
        bool running_flag;
        pthread_mutex_lock(&g_wifi_mutex);
        running_flag = g_event_thread_running;
        pthread_mutex_unlock(&g_wifi_mutex);

        if (!running_flag) 
        {
            LOGI("Wi-Fi Event Thread Stopping.");
            break;
        }

        // 获取 g_wpa_ctrl_event
        pthread_mutex_lock(&g_wifi_mutex);
        struct wpa_ctrl *local_event_ctrl = g_wpa_ctrl_event;
        pthread_mutex_unlock(&g_wifi_mutex);
        if (!local_event_ctrl) 
        {
            LOGE("Event connection lost, stopping thread.");
            break;
        }

        // 使用 poll 监听事件
        pfd.fd = wpa_ctrl_get_fd(local_event_ctrl); 
        pfd.events = POLLIN;

        ret = poll(&pfd, 1, 1000); // 1 second timeout
        if (ret < 0) 
        {
            LOGE("poll() failed in event thread: %m");
            continue;
        } 
        else if (ret == 0) 
        {
            continue;
        }

        // 再次获取 g_wpa_ctrl_event
        pthread_mutex_lock(&g_wifi_mutex);
        local_event_ctrl = g_wpa_ctrl_event;
        pthread_mutex_unlock(&g_wifi_mutex);
        if (!local_event_ctrl) 
        {
            LOGE("Event connection lost, stopping thread.");
            break;
        }

        // 监测到事件通知
        if (wpa_ctrl_pending(local_event_ctrl) > 0) 
        {
            event_len = sizeof(event_buf) - 1;
            if (wpa_ctrl_recv(local_event_ctrl, event_buf, &event_len) == 0) 
            {
                event_buf[event_len] = '\0';
                // 处理事件
                wifi_handle_event(event_buf);
            } 
            else 
            {
                LOGE("wpa_ctrl_recv failed in event thread.");
            }
        } 
        else 
        {
            LOGW("poll() indicated data, but wpa_ctrl_pending() returned <= 0.");
        }
    }

    LOGI("Wi-Fi Event Thread Stopped.");
    return NULL;
}

static int wifi_init(const char *ifname)
{
    int ret;
    char ctrl_path[128];

    pthread_mutex_lock(&g_wifi_mutex);
    if (g_wifi_initialized)
    {
        LOGW("Wi-Fi already initialized.");
        return 0; 
    }
    pthread_mutex_unlock(&g_wifi_mutex);

    // 检查 wpa_supplicant 是否在运行
    snprintf(ctrl_path, sizeof(ctrl_path), WIFI_CTRL_PATH_FORMAT, ifname);
    if (access(ctrl_path, F_OK) != 0) 
    {
        LOGE("wpa_supplicant control socket did not appear at %s", ctrl_path);
        return -1;
    }

    // 打开命令接口和事件接口
    wifi_open_connections(ifname);

    // 更新 Wi-Fi 状态
    wifi_update_status();

    // 启动事件监听线程
    pthread_mutex_lock(&g_wifi_mutex);
    g_event_thread_running = true;
    ret = pthread_create(&g_event_thread, NULL, wifi_event_thread_func, NULL);
    if (ret != 0)
    {
        LOGE("Failed to create Wi-Fi event thread: %d", ret);
        wpa_ctrl_close(g_wpa_ctrl_event);
        g_wpa_ctrl_event = NULL;
        wpa_ctrl_close(g_wpa_ctrl);
        g_wpa_ctrl = NULL;
        g_event_thread_running = false;
        return -1;
    }
    g_wifi_initialized = true;
    pthread_mutex_unlock(&g_wifi_mutex);

    LOGI("Wi-Fi subsystem initialized successfully.");
    return 0;
}

static int wifi_connect(wpa_ctrl_wifi_info_t *wifi_info)
{
    char reply_buf[WIFI_MAX_REPLY_LEN] = {0};
    size_t reply_len;
    int ret = 0;
    int net_id = -1;
    char cmd_buf[256];

    if (!wifi_info || !wifi_info->ssid[0]) 
    {
         LOGE("Invalid Wi-Fi info provided to wifi_connect.");
         return -1;
    }

    // 清理旧网络
    reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd("REMOVE_NETWORK all", reply_buf, &reply_len);
    if (ret < 0) 
    {
        LOGE("Failed to remove existing networks.");
        return -1;
    }

    // 保存配置
    reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd("SAVE_CONFIG", reply_buf, &reply_len);
    if (ret < 0) 
    {
        LOGE("Failed to save config after REMOVE_NETWORK.");
        return -1;
    }

    // 添加新网络
    reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd("ADD_NETWORK", reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "FAIL", 4) == 0) 
    {
        LOGE("Failed to add new network.");
        return -1;
    }
    reply_buf[reply_len] = '\0';
    net_id = atoi(reply_buf);
    LOGI("Added new network with ID: %d", net_id);

    // 设置新网络的 SSID
    snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d ssid \"%s\"", net_id, wifi_info->ssid);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd(cmd_buf, reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
    {
        LOGE("Failed to set SSID for network %d.", net_id);
        return -1;
    }

    // 设置新网络的密码
    if (wifi_info->psw[0] != '\0')  // 非开放网络，有密码
    { 
        snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d psk \"%s\"", net_id, wifi_info->psw);
        reply_len = sizeof(reply_buf);
        ret = wifi_send_cmd(cmd_buf, reply_buf, &reply_len);
        if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
        {
            LOGE("Failed to set PSK for network %d.", net_id);
            return -1;
        }
    } 
    else  // 开放网络，空密码
    {  
        snprintf(cmd_buf, sizeof(cmd_buf), "SET_NETWORK %d key_mgmt NONE", net_id);
        reply_len = sizeof(reply_buf);
        ret = wifi_send_cmd(cmd_buf, reply_buf, &reply_len);
        if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
        {
            LOGE("Failed to set key_mgmt NONE for open network %d.", net_id);
            return -1;
        }
    }

    // 启用新网络
    snprintf(cmd_buf, sizeof(cmd_buf), "ENABLE_NETWORK %d", net_id);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd(cmd_buf, reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
    {
        LOGE("Failed to enable network %d.", net_id);
        return -1;
    }

    // 选择新网络进行连接
    snprintf(cmd_buf, sizeof(cmd_buf), "SELECT_NETWORK %d", net_id);
    reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd(cmd_buf, reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
    {
        LOGE("Failed to select network %d.", net_id);
        return -1;
    }
    LOGI("Selected network %d (%s) for connection.", net_id, wifi_info->ssid);

    return 0; 
}

static int wifi_disconnect()
{
    int ret;
    char cmd[256];

    // 断开连接
    char reply_buf[WIFI_MAX_REPLY_LEN] = {0};
    size_t reply_len = sizeof(reply_buf);
    ret = wifi_send_cmd("DISCONNECT", reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
    {
        LOGE("Failed to send DISCONNECT command.");
        return -1;
    }
    LOGD("Sent DISCONNECT command.");

    // 关闭旧网络
    reply_len = sizeof(reply_buf); 
    ret = wifi_send_cmd("DISABLE_NETWORK all", reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0) 
    {
        LOGW("Failed to disable networks: %s", reply_buf);
        return -1;
    } 
    LOGD("Disabled all networks.");

    // 移除所有网络配置
    reply_len = sizeof(reply_buf); 
    ret = wifi_send_cmd("REMOVE_NETWORK all", reply_buf, &reply_len);
    if (ret < 0 || strncmp(reply_buf, "OK", 2) != 0)
    {
        LOGW("Failed to remove networks: %s", reply_buf);
        return -1;
    }
    LOGD("Removed all networks.");

    // 刷新接口上的 IP 地址
    snprintf(cmd, sizeof(cmd), "ip addr flush dev %s 2>/dev/null || true", WIFI_DEV_IFNAME);
    LOGD("Flushing IP addresses: %s", cmd);
    system(cmd);

    LOGI("Wi-Fi disconnected");
    return 0;
}

static int wifi_deinit()
{
    pthread_mutex_lock(&g_wifi_mutex);

    if (!g_wifi_initialized) 
    {
        LOGW("Wi-Fi not initialized, cannot deinitialize.");
        pthread_mutex_unlock(&g_wifi_mutex);
        return 0;
    }

    LOGI("Deinitializing Wi-Fi subsystem...");

    // 停止事件线程
    if (g_event_thread_running && g_event_thread != 0) 
    {
        g_event_thread_running = false;
        pthread_join(g_event_thread, NULL); 
        g_event_thread = 0;
        LOGI("Wi-Fi event thread joined.");
    }

    // 关闭事件接口连接和命令接口连接
    if (g_wpa_ctrl_event) 
    {
        wpa_ctrl_detach(g_wpa_ctrl_event);
        wpa_ctrl_close(g_wpa_ctrl_event);
        g_wpa_ctrl_event = NULL;
        LOGD("Closed event connection.");
    }
    if (g_wpa_ctrl) 
    {
        wpa_ctrl_close(g_wpa_ctrl);
        g_wpa_ctrl = NULL;
        LOGD("Closed command connection.");
    }

    g_wifi_initialized = false;
    g_connect_status = WPA_WIFI_INACTIVE;

    pthread_mutex_unlock(&g_wifi_mutex);
    LOGI("Wi-Fi subsystem deinitialized.");
    return 0;
}

static rpc_result_t rpc_wifi_connect(cJSON *params)
{
    rpc_result_t res = {
        .status = -1,
        .msg = "connect failed",
        .data_json = NULL
    };

    if (!params) 
    {
        res.msg = "invalid params";
        return res;
    }

    cJSON *ssid = cJSON_GetObjectItem(params, "ssid");
    cJSON *psw  = cJSON_GetObjectItemCaseSensitive(params, "psw");

    if (!ssid || !cJSON_IsString(ssid)) 
    {
        res.msg = "ssid missing or invalid";
        return res;
    }
    
    if (!psw) 
    {
        psw = cJSON_CreateString("");
        if (!psw) 
        {
            res.msg = "internal error creating psw item";
            return res;
        }
    } 
    else if (!cJSON_IsString(psw)) 
    {
        res.msg = "psw invalid";
        return res;
    }

    wpa_ctrl_wifi_info_t info = {0};
    strncpy(info.ssid, ssid->valuestring, sizeof(info.ssid) - 1);
    strncpy(info.psw, psw->valuestring, sizeof(info.psw) - 1);
    info.ssid[sizeof(info.ssid) - 1] = '\0';
    info.psw[sizeof(info.psw) - 1] = '\0';

    int ret = wifi_connect(&info);
    if (ret == 0) 
    {
        res.status = 0;
        res.msg = "connection initiated"; 
    } 
    else 
    {
        res.msg = "failed to initiate connection";
    }

    return res;
}

static rpc_result_t rpc_wifi_disconnect(cJSON *params)
{
    rpc_result_t res = { 
        .status = -1, 
        .msg = "disconnect failed", 
        .data_json = NULL 
    };

    (void)params; 

    if (wifi_disconnect() == 0) 
    {
        res.status = 0;
        res.msg = "ok";
    } 
    else 
    {
        res.msg = "failed to send disconnect command";
    }

    return res;
}

static rpc_result_t rpc_wifi_status_get(cJSON *params)
{
    rpc_result_t res = {
        .status = -1,
        .msg = "status get failed",
        .data_json = NULL
    };

    (void)params;

    pthread_mutex_lock(&g_wifi_mutex);
    wpa_wifi_conn_status_t conn_status = g_connect_status;
    pthread_mutex_unlock(&g_wifi_mutex);

    const char *wifi_sta_str = "unknown";
    switch (conn_status) 
    {
        case WPA_WIFI_CONNECT: wifi_sta_str = "connected"; break;
        case WPA_WIFI_DISCONNECT: wifi_sta_str = "disconnected"; break;
        case WPA_WIFI_SCANNING: wifi_sta_str = "scanning"; break;
        case WPA_WIFI_INACTIVE: wifi_sta_str = "inactive"; break;
        case WPA_WIFI_WRONG_KEY: wifi_sta_str = "wrong_key"; break;
        case WPA_WIFI_UNKNOWN:
        default: wifi_sta_str = "unknown"; break;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) 
    {
        res.msg = "internal error creating JSON";
        return res;
    }
    cJSON_AddBoolToObject(root, "connected", (conn_status == WPA_WIFI_CONNECT) ? true : false);

    res.status = 0;
    res.msg    = "ok";
    res.data_json = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return res;
}

int wifi_cmd_register()
{
    int ret;

    ret = wifi_init(WIFI_DEV_IFNAME);
    if (ret != 0) 
    {
        LOGE("wifi init failed, canceling wifi command registration");
        return ret;
    }

    command_t cmd_wifi_connect, cmd_wifi_status, cmd_wifi_disconnect;

    cmd_wifi_connect.name = CMD_WIFI_CONNECT;
    cmd_wifi_connect.handler = rpc_wifi_connect;

    cmd_wifi_status.name = CMD_WIFI_STATUS;
    cmd_wifi_status.handler = rpc_wifi_status_get;

    cmd_wifi_disconnect.name = CMD_WIFI_DISCONNECT;
    cmd_wifi_disconnect.handler = rpc_wifi_disconnect;

    ret = command_register(&cmd_wifi_connect);
    if (ret != 0) 
    {
        LOGE("command_register for wifi_connect failed");
        wifi_deinit();
        return ret;
    }

    ret = command_register(&cmd_wifi_status);
    if (ret != 0) 
    {
        LOGE("command_register for wifi_status failed");
        wifi_deinit();
        return ret;
    }

    ret = command_register(&cmd_wifi_disconnect);
    if (ret != 0) 
    {
        LOGE("command_register for wifi_disconnect failed");
        wifi_deinit();
        return ret;
    }

    return 0;
}