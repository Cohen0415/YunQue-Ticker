#ifndef _YUV_COMPONENT_H_
#define _YUV_COMPONENT_H_

#include "common.h"
#define YUV_IN_BUFFER_LIST_NODE_NUM  (16)
#define YUV_OUT_BUFFER_LIST_NODE_NUM (32)

typedef struct yuv_inbuf_manager
{
    struct listnode     empty_frame_list;
    struct listnode     valid_frame_list;
    pthread_mutex_t     mutex;
    int                 empty_num;

} yuv_inbuf_manager;

typedef struct yuv_outbuf_manager
{
    struct listnode     empty_frame_list;
    struct listnode     valid_frame_list;
    struct listnode     pts_list;
    pthread_mutex_t     mutex;
    int                 empty_num;
} yuv_outbuf_manager;


typedef struct yuv_comp_ctx {
    common_comp_ctx*    base_ctx;
    comp_state_type     state;
    comp_callback_type* callback;
    void*               callback_data;
    yuv_inbuf_manager   in_buf_manager;
    //yuv_outbuf_manager  out_buf_manager;
    component_type*     self_comp;
    comp_tunnel_info    in_port_tunnel_info;
    struct listnode     out_port_tunnel_info_list;
    pthread_t           yuv_thread;
    int                 out_port_tunnel_number;
    pthread_mutex_t     mutex;
} yuv_comp_ctx;

typedef struct pts_node_entry {
    int             count;
    uint64_t        pts;
    struct listnode node;
} pts_node_entry;

error_type yuv_comp_component_init(PARAM_IN comp_handle component);
#endif
