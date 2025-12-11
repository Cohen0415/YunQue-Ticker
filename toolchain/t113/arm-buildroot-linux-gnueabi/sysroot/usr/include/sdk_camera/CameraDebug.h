#ifndef __HAL_CAMERA_DEBUG_H__
#define __HAL_CAMERA_DEBUG_H__

#include <stdarg.h>

#include <libcedarc/memoryAdapter.h>
#include <libcedarc/sc_interface.h>

#include <sdk_base/sdklog.h>

#define CAMERA_HAL_VERSION      "3000130327_V1.0"

//enable 4in1 for 360
#define CAMERA_MANAGER_ENABLE

#define CAMERA_ID_STEP 4

#define NB_CAMERA 4     //number of camera

#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))
#define ALIGN_8B(x) (((x) + (7)) & ~(7))

#define DBG_CAMERA_HARDWARE     0
#define DBG_V4L2_CAMERA         0
#define DBG_PREVIEW             0
#define DBG_CALLBACK            0
#define DBG_DVR_FACTORY         0
#define DBG_CAMERA_CONFIG       0
#define DBG_BUFFER_LIST         0
#define DBG_CAMERA_WATERMARK    0
#define DBG_CAMERA_RECORD       0

#define V4L2_SAVEFRAME       0
#define V4L2_SAVEFRAME_COUNT 10
#define PREVIEW_SAVEFRAME    0

/* Defines whether we should trace parameter changes. */
#define DEBUG_PARAM 1

#define DEBUG_MSG   1

// enable all print information
//#define LOG_NDEBUG 0


// performance debug
#define DBG_TIME_ENABLE     0
#if DBG_TIME_ENABLE
#define LOG_TIME            ALOGD
#define DBG_TIME_BEGIN(inf, en)                                         \
    int64_t llt_ms = systemTime() / 1000000;                            \
    int64_t lt_ms = llt_ms;                                             \
    int64_t nt_ms = 0;                                                  \
    if (en) LOG_TIME("\t[T_DBG_bgn] %s timestamp: %lld", inf, lt_ms);

#define DBG_TIME_DIFF(inf)                                              \
    nt_ms = systemTime() / 1000000;                                     \
    LOG_TIME("\t[T_DBG_dff] %s use: %lld(ms)", inf, (nt_ms - lt_ms));   \
    lt_ms = nt_ms;

#define DBG_TIME_END(inf, en)                                           \
    nt_ms = systemTime() / 1000000;                                     \
    LOG_TIME("\t[T_DBG_end] %s use: %lld(ms)", inf, (nt_ms - llt_ms));  \
    if (en) LOG_TIME("\t[T_DBG_end] %s timestamp: %lld", inf, nt_ms);
#else
#define DBG_TIME_BEGIN(inf, en)
#define DBG_TIME_DIFF(inf)
#define DBG_TIME_END(inf, en)
#endif

#define DBG_TIME_AVG_ENABLE     0
#if DBG_TIME_AVG_ENABLE
#define LOG_TIME_AVG            ALOGD
#define DBG_TIME_AVG_BEGIN(tag)                                         \
    static int64_t tag##_time = 0;                                      \
    static int tag##_cnt = 0;

#define DBG_TIME_AVG_INIT(tag)                                          \
    tag##_time = 0; tag##_cnt = 0;

#define DBG_TIME_AVG_AREA_IN(tag)                                       \
    int64_t tag##_diff = systemTime();

#define DBG_TIME_AVG_AREA_OUT(tag)                                      \
    tag##_diff = systemTime() - tag##_diff;                             \
    tag##_time += tag##_diff;                                           \
    tag##_cnt++;

#define DBG_TIME_AVG_END(tag, inf)                                      \
    if (tag##_cnt > 0) LOG_TIME_AVG("\t[T_DBG_avg] %s cnt: %d, average use: %lld(us)", inf, tag##_cnt, tag##_time/tag##_cnt/1000);
#else
#define DBG_TIME_AVG_BEGIN(tag)
#define DBG_TIME_AVG_INIT(tag)
#define DBG_TIME_AVG_AREA_IN(tag)
#define DBG_TIME_AVG_AREA_OUT(tag)
#define DBG_TIME_AVG_END(tag, inf)
#endif

#define UNUSED(x) (void)(x)

#endif // __HAL_CAMERA_DEBUG_H__

