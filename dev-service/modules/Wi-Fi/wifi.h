#ifndef _WIFI_H_
#define _WIFI_H_

#include <stddef.h> // For size_t
#include "wpa_ctrl.h"

#define WIFI_DEV_IFNAME                 "wlan0"                                 // Wi-Fi interface name
#define WIFI_CTRL_PATH_FORMAT           "/var/run/wpa_supplicant/%s"            // Control socket path format
#define WIFI_WPA_SUPPLICANT_BIN         "wpa_supplicant"                        // wpa_supplicant binary
#define WIFI_UDHCPC_CMD_FORMAT          "udhcpc -i %s -t 5 -T 2 -A 5 -q -n"     // DHCP client command format

#define WIFI_MAX_REPLY_LEN              2048        // Max length for wpa_supplicant replies
#define WIFI_EVENT_BUF_SIZE             2048        // Buffer size for event messages

// --- Enums ---
typedef enum {
    WPA_WIFI_CLOSE = 0,
    WPA_WIFI_OPEN
} wpa_wifi_status_t;

typedef enum {
    WPA_WIFI_INACTIVE = 0,
    WPA_WIFI_SCANNING,
    WPA_WIFI_CONNECT,
    WPA_WIFI_DISCONNECT,
    WPA_WIFI_WRONG_KEY, // Added for better feedback
    WPA_WIFI_UNKNOWN
} wpa_wifi_conn_status_t;

// --- Data Structures ---
typedef struct {
    char ssid[128]; // Adjust size as needed
    char psw[128];  // Adjust size as needed
} wpa_ctrl_wifi_info_t;

int wifi_cmd_register(); 

#endif // _WIFI_H_