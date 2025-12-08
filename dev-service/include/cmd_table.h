#ifndef CMD_TABLE_H
#define CMD_TABLE_H

/* COMMAND LIST */
/* BACKLIGHT */
#define CMD_SET_BRIGHTNESS       "brightness.set"
#define CMD_GET_BRIGHTNESS       "brightness.get"
/* SYSINFO */
#define CMD_GET_BJTIME           "sysinfo.bjtime.get"
#define CMD_GET_CPU_TEMP         "sysinfo.temp.get"
/* AUDIO */
#define CMD_AUDIO_VOLUME_SET     "audio.volume.set"
#define CMD_AUDIO_VOLUME_GET     "audio.volume.get"
#define CMD_AUDIO_PLAY           "audio.play"
#define CMD_AUDIO_STOP           "audio.stop"
/* WIFI */
#define CMD_WIFI_SCAN            "wifi.scan"
#define CMD_WIFI_CONNECT         "wifi.connect"
#define CMD_WIFI_STATUS          "wifi.status.get"
#define CMD_WIFI_DISCONNECT      "wifi.disconnect"

#endif // CMD_TABLE_H