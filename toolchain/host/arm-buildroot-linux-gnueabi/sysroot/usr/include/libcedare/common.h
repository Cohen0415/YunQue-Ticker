#ifndef _COMMON_H_
#define _COMMON_H_

#include "cde_list.h"
#include "tmessage.h"
#include <pthread.h>
#include "cde_log.h"
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/prctl.h>


#define ALIGN(x, a)              ((a) * (((x) + (a) - 1) / (a)))

#ifndef PARAM_IN
#define PARAM_IN
#endif

#ifndef PARAM_OUT
#define PARAM_OUT
#endif

#ifndef PARAM_INOUT
#define PARAM_INOUT
#endif

typedef int error_type;
#define ERROR_TYPE_OK            (0)
#define ERROR_TYPE_ERROR         (-1)
#define ERROR_TYPE_NOMEM         (-2)
#define ERROR_TYPE_UNEXIST       (-3)
#define ERROR_TYPE_STATE_ERROR   (-4)
#define ERROR_TYPE_ILLEGAL_PARAM (-5)

#if 0

typedef struct yuv_frame
{
    unsigned char* phy_addrY;
    unsigned char* phy_addrC;
    unsigned char* vir_addrY;
    unsigned char* vir_addrC;
    long long            pts;
    unsigned int   mId;
} yuv_frame;

typedef struct yuv_frame_info
{
    yuv_frame   yuvFrame;
    //unsigned int mId;
} yuv_frame_info;

typedef struct yuv_frame_info_node
{
    yuv_frame_info* yuvFrame_info;
    struct listnode mList;
} yuv_frame_info_node;

#endif

typedef struct video_frame_s
{
    unsigned long   phy_addr[3];/* Y, U, V; Y, UV; Y, VU */
    void*           vir_addr[3];
    uint64_t        pts;          /* unit:us */
    unsigned int    mId;   //id identify frame uniquely
    unsigned int    ref_cnt;
} video_frame_s;

#if 0
typedef struct video_frame_info_s
{
    video_frame_s VFrame;
    unsigned int mId;   //id identify frame uniquely
} video_frame_info_s;
#endif

typedef struct video_frame_info_node
{
    video_frame_s VFrame;
    struct listnode mList;
}video_frame_info_node;

typedef struct video_stream_s
{
    int            id;
    long long      pts;
    unsigned int   flag;
    unsigned int   size0;
    unsigned int   size1;
    unsigned int   offset0;
    unsigned int   offset1;

    unsigned int   size2;
    unsigned int   offset2;

    unsigned char* data0;
    unsigned char* data1;
    int            is_sps_pps_flag;

}video_stream_s;

typedef struct video_stream_node
{
    video_stream_s video_stream;
    struct listnode mList;
}video_stream_node;

typedef enum comp_port_type
{
    COMP_INPUT_PORT,
    COMP_OUTPUT_PORT
} comp_port_type;


typedef void*   comp_handle;

typedef struct comp_tunnel_info
{
    comp_handle  tunnel_comp;
    unsigned int valid_flag;
}comp_tunnel_info;

typedef struct comp_tunnel_info_node
{
    struct listnode     tunnel_node;
    comp_tunnel_info   tunnel_info;
}comp_tunnel_info_node;


typedef enum comp_command_type
{
    COMP_COMMAND_INIT   = 0 ,
    COMP_COMMAND_START  = 1,
    COMP_COMMAND_PAUSE  = 2,
    COMP_COMMAND_STOP   = 3,
    COMP_COMMAND_QUIT   = 4,
    COMP_COMMAND_DESTORY = 5,

    COMP_COMMAND_YUV_INPUT_FRAME_VALID,
    COMP_COMMAND_VENC_INPUT_FRAME_VALID,
    COMP_COMMAND_AENC_INPUT_FRAME_VALID,
    COMP_COMMAND_CANCEL_THREAD,

    COMP_COMMAND_MAX = 0X7FFFFFFF
} comp_command_type;

#define WAIT_REPLY_NUM (COMP_COMMAND_CANCEL_THREAD + 1)

/*
 * The state machine of the media.
 */

typedef enum comp_state_type {
    // Error state.
    COMP_STATE_ERROR                 = 0,

    // media was just created.
    COMP_STATE_IDLE                  = 1,

    // media has been initialized.
    COMP_STATE_INITIALIZED           = 2,

    // media is in progress.
    COMP_STATE_EXECUTING             = 3,

    // media is in pause.
    COMP_STATE_PAUSE                 = 4,
}comp_state_type;

typedef enum comp_index_type {
    COMP_INDEX_VENC_CONFIG_Base   = 0x01000000,
    COMP_INDEX_VENC_CONFIG_Normal,
    COMP_INDEX_VENC_CONFIG_Dynamic_ForceKeyFrame,

    COMP_INDEX_VENC_CONFIG_GET_VBV_BUF_INFO,
    COMP_INDEX_VENC_CONFIG_GET_STREAM_HEADER,

    COMP_INDEX_AENC_CONFIG_Base   = 0x01000000,

    COMP_INDEX_VI_CONFIG_Base   = 0x01000000,
    COMP_INDEX_VI_CONFIG_Normal,
    COMP_INDEX_VI_CONFIG_Dynamic_ForceKeyFrame,
}comp_index_type;

typedef enum comp_event_type
{
    COMP_EVENT_CMD_COMPLETE,
    COMP_EVENT_CMD_ERROR,
    COMP_EVENT_MAX = 0x7FFFFFFF
} comp_event_type;

typedef struct comp_buffer_header_type {
    void* privateData;
}comp_buffer_header_type;

typedef struct comp_callback_type
{
   error_type (*EventHandler)(
        PARAM_IN comp_handle component,
        PARAM_IN void* pAppData,
        PARAM_IN comp_event_type eEvent,
        PARAM_IN unsigned int nData1,
        PARAM_IN unsigned int nData2,
        PARAM_IN void* pEventData);

    error_type (*empty_in_buffer_done)(
        PARAM_IN comp_handle component,
        PARAM_IN void* pAppData,
        PARAM_IN comp_buffer_header_type* pBuffer);

    error_type (*fill_out_buffer_done)(
        PARAM_IN comp_handle component,
        PARAM_IN void* pAppData,
        PARAM_IN comp_buffer_header_type* pBuffer);

} comp_callback_type;


//* set_config --> prepare --> start --> stop --> destroy
typedef struct component_type
{
    int   id;
    void* component_private;
    error_type (*init)(
            PARAM_IN  comp_handle component);

    error_type (*start)(
            PARAM_IN  comp_handle component);

    error_type (*pause)(
            PARAM_IN  comp_handle component);

    error_type (*stop)(
            PARAM_IN  comp_handle component);

    error_type (*destroy)(
            PARAM_IN  comp_handle component);

    error_type (*get_config)(
            PARAM_IN  comp_handle component,
            PARAM_IN  comp_index_type index,
            PARAM_INOUT void* param_data);

    error_type (*set_config)(
            PARAM_IN  comp_handle component,
            PARAM_IN  comp_index_type index,
            PARAM_IN  void* param_data);

    error_type (*get_state)(
            PARAM_IN  comp_handle component,
            PARAM_OUT comp_state_type* pState);

    /* please comp empty this in-buffer*/
    error_type (*empty_this_in_buffer)(
            PARAM_IN  comp_handle component,
            PARAM_IN  comp_buffer_header_type* pBuffer);

    /* please comp fill this out-buffer*/
    error_type (*fill_this_out_buffer)(
            PARAM_IN  comp_handle component,
            PARAM_IN  comp_buffer_header_type* pBuffer);

    error_type (*set_callbacks)(
            PARAM_IN  comp_handle component,
            PARAM_IN  comp_callback_type* pCallbacks,
            PARAM_IN  void* pAppData);

    error_type (*setup_tunnel)(
        PARAM_IN  comp_handle component,
        PARAM_IN  comp_port_type port_type,
        PARAM_IN  comp_handle tunnel_comp);

} component_type;

#define comp_init(component) \
            component->init(component)

#define comp_start(component) \
            component->start(component)

#define comp_pause(component) \
            component->pause(component)

#define comp_stop(component) \
            component->stop(component)

#define comp_destroy(component) \
            component->destroy(component)

#define comp_get_config(component, index, param) \
            component->get_config(component, index, param)

#define comp_set_config(component, index, param) \
            component->set_config(component, index, param)

#define comp_get_state(component, pstate) \
            component->get_state(component, pstate)

#define comp_empty_this_in_buffer(component, buffer) \
            ((component_type*)component)->empty_this_in_buffer( component,  buffer)

#define comp_fill_this_out_buffer(component, buffer) \
            ((component_type*)component)->fill_this_out_buffer(component, buffer)

#define comp_set_callbacks(component, pcallback, pappdata) \
            ((component_type *)component)->set_callbacks(component, pcallback, pappdata)

#define comp_setup_tunnel(component, port_type, tunnel_comp) \
            component->setup_tunnel(component, port_type, tunnel_comp)


typedef error_type (*component_init)(PARAM_IN comp_handle component);

typedef struct common_comp_ctx
{
    message_queue_t         msg_queue;
    pthread_mutex_t         mLocked;
    pthread_condattr_t      condAttr;
    pthread_cond_t          mReady;
    int                     condition[WAIT_REPLY_NUM];
}common_comp_ctx;

int post_msg_and_wait(common_comp_ctx* base_comp_ctx, int cmd_id);

#endif

