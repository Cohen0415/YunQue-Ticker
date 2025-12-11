#ifndef _SDKLOG_H_
#define _SDKLOG_H_

#include <execinfo.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int sdk_log_print(int prio, int module_prio, const char *log_tag, const char *log_level, const char *fmt, ...);
int sdk_log_print_without_format(const char *log_level, const char *fmt, ...);
int sdk_log_setlevel(int prio);
void print_stacktrace();

#ifdef USE_LOGCAT

#ifndef LOG_TAG
#define LOG_TAG ""
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL 3
#endif

#define ALOGV(...) sdk_log_print(5, LOG_LEVEL, LOG_TAG, "(V) : ", __VA_ARGS__)
#define ALOGD(...) sdk_log_print(4, LOG_LEVEL, LOG_TAG, "(D) : ", __VA_ARGS__)
#define ALOGI(...) sdk_log_print(3, LOG_LEVEL, LOG_TAG, "(I) : ", __VA_ARGS__)
#define ALOGW(...) sdk_log_print(2, LOG_LEVEL, LOG_TAG, "(W) : ", __VA_ARGS__)
#define ALOGE(...) sdk_log_print(1, LOG_LEVEL, LOG_TAG, "(E) : ", __VA_ARGS__)

#define F_LOG ALOGV("%s, line: %d", __FUNCTION__, __LINE__);

#else

#define ALOGV(...)
#define ALOGD(...)
#define ALOGI(...)
#define ALOGW(...) \
    do { \
        sdk_log_print_without_format("(W) : ", __VA_ARGS__); \
    } while (0)
#define ALOGE(...) \
    do { \
        sdk_log_print_without_format("(E) : ", __VA_ARGS__); \
    } while (0)

#endif // #ifdef USE_LOGCAT

#ifdef __cplusplus
} // extern "C"
#endif
#endif
