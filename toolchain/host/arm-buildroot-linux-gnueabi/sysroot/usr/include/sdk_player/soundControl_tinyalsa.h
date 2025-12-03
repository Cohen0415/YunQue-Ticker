#ifndef SOUNDCONTROL_TINYALSA_H
#define SOUNDCONTROL_TINYALSA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libcedarx/soundControl.h>
#include <sdk_player/tinyalsasoundlib.h>
#include <sdk_player/outputCtrl.h>

typedef int (*autCallback_func)(int32_t msgType, void *user, void *data, int len);

SoundCtrl *SoundDeviceCreate();

int tinyalsaSetCallback(SoundCtrl *s, void *callback, void **pUserData);

#ifdef __cplusplus
}
#endif

#endif
