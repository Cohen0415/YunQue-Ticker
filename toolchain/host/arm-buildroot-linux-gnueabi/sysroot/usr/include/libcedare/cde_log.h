
/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : log.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2015/04/13
*   Comment :
*
*
*/

#ifndef LOG_H
#define LOG_H

// option for debug level.
#define OPTION_LOG_LEVEL_CLOSE      0
#define OPTION_LOG_LEVEL_ERROR      1
#define OPTION_LOG_LEVEL_WARNING    2
#define OPTION_LOG_LEVEL_DEFAULT    3
#define OPTION_LOG_LEVEL_DETAIL     4


#define CONFIG_LOG_LEVEL_E  OPTION_LOG_LEVEL_WARNING

#ifndef LOG_TAG
#define LOG_TAG "cedare"
#endif

#ifdef __ANDROID__
    #include <cutils/log.h>

    #define LOG_LEVEL_ERROR_E     ANDROID_LOG_ERROR
    #define LOG_LEVEL_WARNING_E   ANDROID_LOG_WARN
    #define LOG_LEVEL_INFO_E      ANDROID_LOG_INFO
    #define LOG_LEVEL_VERBOSE_E   ANDROID_LOG_VERBOSE
    #define LOG_LEVEL_DEBUG_E     ANDROID_LOG_DEBUG

    #define CDELOG(level, fmt, arg...)  \
        LOG_PRI(level, LOG_TAG, "<%s:%u>: " fmt, __FUNCTION__, __LINE__, ##arg)

#define CC_LOG_ASSERT(e, fmt, arg...)                               \
        LOG_ALWAYS_FATAL_IF(                                        \
                !(e),                                               \
                "<%s:%d>check (%s) failed:" fmt,                    \
                __FUNCTION__, __LINE__, #e, ##arg)                  \

#ifdef AWP_DEBUG
#define CDX_CHECK(e)                                            \
    LOG_ALWAYS_FATAL_IF(                                        \
            !(e),                                               \
            "<%s:%d>CDX_CHECK(%s) failed.",                     \
            __FUNCTION__, __LINE__, #e)

#else
#define CDX_CHECK(e)
#endif

#else

#include <stdio.h>
#include <string.h>

#define LOG_LEVEL_ERROR_E     "error  "
#define LOG_LEVEL_WARNING_E   "warning"
#define LOG_LEVEL_INFO_E      "info   "
#define LOG_LEVEL_VERBOSE_E   "verbose"
#define LOG_LEVEL_DEBUG_E     "debug  "

#define CDELOG(level, fmt, arg...)  \
    printf("%s: %s <%s:%u>: "fmt"\n", level, LOG_TAG, __FUNCTION__, __LINE__, ##arg)

#define CC_LOG_ASSERT(e, fmt, arg...)                                       \
                do {                                                        \
                    if (!(e))                                               \
                    {                                                       \
                        cde_loge("check (%s) failed:"fmt, #e, ##arg);           \
                        assert(0);                                          \
                    }                                                       \
                } while (0)

#ifdef AWP_DEBUG
#define CDX_CHECK(e)                                            \
                        do {                                                        \
                            if (!(e))                                               \
                            {                                                       \
                                CDX_LOGE("check (%s) failed.", #e);                 \
                                assert(0);                                          \
                            }                                                       \
                        } while (0)
#else
#define CDX_CHECK(e)
#endif

#endif

#define cde_logd(fmt, arg...) CDELOG(LOG_LEVEL_DEBUG_E, fmt, ##arg)

#if CONFIG_LOG_LEVEL_E == OPTION_LOG_LEVEL_CLOSE

#define cde_loge(fmt, arg...)
#define cde_logw(fmt, arg...)
#define cde_logi(fmt, arg...)
#define cde_logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL_E == OPTION_LOG_LEVEL_ERROR

#define cde_loge(fmt, arg...) CDELOG(LOG_LEVEL_ERROR_E, "\033[40;31m" fmt "\033[0m", ##arg)
#define cde_logw(fmt, arg...)
#define cde_logi(fmt, arg...)
#define cde_logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL_E == OPTION_LOG_LEVEL_WARNING

#define cde_loge(fmt, arg...) CDELOG(LOG_LEVEL_ERROR_E, "\033[40;31m" fmt "\033[0m", ##arg)
#define cde_logw(fmt, arg...) CDELOG(LOG_LEVEL_WARNING_E, fmt, ##arg)
#define cde_logi(fmt, arg...)
#define cde_logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL_E == OPTION_LOG_LEVEL_DEFAULT

#define cde_loge(fmt, arg...) CDELOG(LOG_LEVEL_ERROR_E, "\033[40;31m" fmt "\033[0m", ##arg)
#define cde_logw(fmt, arg...) CDELOG(LOG_LEVEL_WARNING_E, fmt, ##arg)
#define cde_logi(fmt, arg...) CDELOG(LOG_LEVEL_INFO_E, fmt, ##arg)
#define cde_logv(fmt, arg...)

#elif CONFIG_LOG_LEVEL_E == OPTION_LOG_LEVEL_DETAIL

#define cde_loge(fmt, arg...) CDELOG(LOG_LEVEL_ERROR_E, "\033[40;31m" fmt "\033[0m", ##arg)
#define cde_logw(fmt, arg...) CDELOG(LOG_LEVEL_WARNING_E, fmt, ##arg)
#define cde_logi(fmt, arg...) CDELOG(LOG_LEVEL_INFO_E, fmt, ##arg)
#define cde_logv(fmt, arg...) CDELOG(LOG_LEVEL_VERBOSE_E, fmt, ##arg)

#else
    #error "invalid configuration of debug level."
#endif

#endif

