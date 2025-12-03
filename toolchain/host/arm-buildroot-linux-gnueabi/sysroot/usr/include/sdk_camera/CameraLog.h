#ifndef __HAL_CAMERA_LOG_H__
#define __HAL_CAMERA_LOG_H__

#include <sdk_base/sdklog.h>

#ifndef LOG_NDEBUG
#define LOG_NDEBUG 1
#else
#define LOG_NDEBUG 0
#endif

#define ALOGNONE(...) do {\
    char buffer[256]; \
    snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
    (void)(buffer); \
} while(0)

#if LOG_NDEBUG
#define CameraLogV(fmt, arg...) \
    ALOGNONE("<%s:%u>: " fmt, __FUNCTION__, __LINE__, ##arg)
#else
#define CameraLogV(fmt, arg...) \
    ALOGV("\033[40;32m" "<%s:%u>: " fmt "\033[0m", __FUNCTION__, __LINE__, ##arg)
#endif

#define CameraLogD(fmt, arg...) \
    ALOGD("<%s:%u>: " fmt, __FUNCTION__, __LINE__, ##arg)

#define CameraLogI(fmt, arg...) \
    ALOGI("\033[40;34m"  "<%s:%u>: " fmt "\033[0m", __FUNCTION__, __LINE__, ##arg)

#define CameraLogW(fmt, arg...) \
    ALOGW("\033[40;33m"  "<%s:%u>: " fmt "\033[0m", __FUNCTION__, __LINE__, ##arg)

#define CameraLogE(fmt, arg...) \
    ALOGE("\033[40;31m"  "<%s:%u>: " fmt "\033[0m", __FUNCTION__, __LINE__, ##arg)

#define CAMERA_LOG CameraLogV("");

#endif // __HAL_CAMERA_LOG_H__
