#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>

#define WPA_CONF_PATH       "/etc/wpa_supplicant.conf"

#define WIFI_MAX_SSID_LEN     64
#define WIFI_MAX_PSK_LEN      64
#define WIFI_SCAN_LIST_MAX    32

// 扫描结果结构体
typedef struct {
    char ssid[WIFI_MAX_SSID_LEN];
    int  signal_level;     // dBm
    bool encrypted;        // 是否加密
} wifi_ap_t;

typedef struct {
    bool connected;                // 是否连接
    char ssid[WIFI_MAX_SSID_LEN];  // 当前连接SSID
    char ip[32];                   // IP地址
    int  signal_level;             // 信号强度 dBm
} wifi_status_t;

int wifi_cmd_register(void);

#endif // WIFI_H
