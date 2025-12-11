#ifndef _VENC_COMPONENT_H_
#define _VENC_COMPONENT_H_

#include "common.h"
#include <errno.h>
#include <stdint.h>
#include <vencoder.h>

#define VENC_IN_BUFFER_LIST_NODE_NUM  (16)
#define VENC_OUT_BUFFER_LIST_NODE_NUM (32)

#define REQUEST_EXIT "EXIT"

typedef struct VBV_BUFFER_INFO
{
    int handle_id;
    int size;
}VBV_BUFFER_INFO;


typedef struct venc_comp_header_data {
    unsigned char*  pBuffer;
    unsigned int    nLength;
}venc_comp_header_data;


typedef struct VENC_VBV_BUFFER_INFO
{
    int handle_id;
    int size;
}VENC_VBV_BUFFER_INFO;

typedef enum VENC_COMP_RC_MODE
{
    VENC_COMP_RC_MODE_H264CBR = 1,
    VENC_COMP_RC_MODE_H264VBR,

    VENC_RC_MODE_BUTT,

}VENC_COMP_RC_MODE;


typedef enum {
    VENC_COMP_CODEC_H264 = 0,
    VENC_COMP_CODEC_H265 = 1,
    VENC_COMP_CODEC_JPEG = 2,
} VENC_COMP_CODEC_TYPE;

typedef enum VENC_COMP_PIXEL_FMT {
    VENC_COMP_PIXEL_YUV420SP  = 0,
    VENC_COMP_PIXEL_YVU420SP  = 1,
    VENC_COMP_PIXEL_YUV420P   = 2,
    VENC_COMP_PIXEL_YVU420P   = 3,
    VENC_COMP_PIXEL_ARGB      = 4,
    VENC_COMP_PIXEL_RGBA      = 5,
    VENC_COMP_PIXEL_ABGR      = 6,
    VENC_COMP_PIXEL_BGRA      = 7,
    VENC_COMP_PIXEL_AFBC_AW   = 8,
    VENC_COMP_PIXEL_LBC_AW    = 9,
}VENC_COMP_PIXEL_FMT;

typedef enum VENC_COMP_LBC_SUBFMT {
    VENC_COMP_LBC_LOSSLESS   = 0,
    VENC_COMP_LBC_LOSSY_2X   = 1,
    VENC_COMP_LBC_LOSSY_2_5X = 2,
}VENC_COMP_LBC_SUBFMT;

typedef struct venc_comp_vbr_param{
    unsigned int max_bit_rate;
    unsigned int moving_th;      //range[1,31], 1:all frames are moving,
                                 //            31:have no moving frame, default: 20
    int          quality;        //range[1,10], 1:worst quality, 10:best quality
}venc_comp_vbr_param;

typedef struct venc_variable_frame_rate
{
    char                     bEnable;
    unsigned int             nOri_frame_rate;
    unsigned int             nFrame_rate_var;

    unsigned int             nDecNum;
    unsigned int             *pSelect_idx;
    uint64_t                 nCur_change_pts;
    uint64_t                 change_pts_internel;
}venc_variable_frame_rate;

typedef struct venc_scale_config {
    char                     bEnable;
    long long             nInterval_Pts;
    long long             nCur_pts;
} venc_scale_config;

typedef struct venc_comp_base_config
{
    VENC_COMP_CODEC_TYPE codec_type;
    unsigned int         src_width;
    unsigned int         src_height;
    unsigned int         dst_width;
    unsigned int         dst_height;
    int                  bit_rate;
    int                  frame_rate;
    VENC_COMP_PIXEL_FMT  input_pixel_format;
    int                  max_keyframe_interval;
    int                  qp_min;
    int                  qp_max;
    VENC_PIXEL_FMT       input_format;

    VENC_COMP_RC_MODE    rc_mode;
    /* should config the param when rc_mode is vbr*/
    venc_comp_vbr_param  vbr_param;

    /* should config the param when pixel_format is lbc*/
    VENC_COMP_LBC_SUBFMT lbc_sub_format;
    venc_variable_frame_rate var_frm_rate;
    venc_scale_config       scale_config;
}venc_comp_base_config;

typedef struct venc_comp_normal_config
{
    int tmp;
}venc_comp_normal_config;

//* dynamic config

//typedef struct video_stream_s
//{
//    int            id;
//    long long      pts;
//    unsigned int   flag;
//    unsigned int   size0;
//    unsigned int   size1;
//    unsigned int   offset0;
//    unsigned int   offset1;
//
//    unsigned int   size2;
//    unsigned int   offset2;
//
//    unsigned char* data0;
//    unsigned char* data1;
//
//}video_stream_s;
//
//typedef struct video_stream_node
//{
//    video_stream_s video_stream;
//    struct listnode mList;
//}video_stream_node;

typedef struct venc_outbuf_manager
{
    struct listnode    empty_stream_list;
    struct listnode    valid_stream_list;
    pthread_mutex_t     mutex;
    int                 empty_num;
    int                 no_empty_stream_flag;
}venc_outbuf_manager;


//typedef struct video_frame_s
//{
//    //unsigned int    width;
//    //unsigned int    weight;
//
//    unsigned int    phy_addr[3];/* Y, U, V; Y, UV; Y, VU */
//    void*           vir_addr[3];
//    //unsigned int    stride[3];
//
//    //unsigned int    header_phy_addr[3];
//    //void*           header_vir_addr[3];
//    //unsigned int    header_stride[3];
//
//    //short           offset_top;     /* top offset of show area */
//    //short           offset_nottom;  /* bottom offset of show area */
//    //short           offset_left;    /* left offset of show area */
//    //short           offset_right;   /* right offset of show area */
//
//    uint64_t        pts;          /* unit:us */
//    //unsigned int    exposure_time; /* every frame exp time */
//    //unsigned int    frame_cnt;     /* rename mPrivateData to Framecnt_exp_start */
//    //int env_lv;                    /* environment luminance value */
//
//    //unsigned int    who_set_flag;   /* reserve(8bit)|COMP_TYPE(8bit)|DEV_NUM(8bit)|CHN_NUM(8bit) */
//    //uint64_t        flag_pts;      /* when generate this flag, unit(us) */
//    //unsigned int    frm_flag;
//} video_frame_s;
//
//
//typedef struct video_frame_info_s
//{
//    video_frame_s VFrame;
//    //unsigned int mPoolId;
//    unsigned int mId;   //id identify frame uniquely
//} video_frame_info_s;
//
//
//typedef struct video_frame_info_node
//{
//    video_frame_info_s VFrame;
//    struct listnode mList;
//}video_frame_info_node;



typedef struct venc_inbuf_manager
{
    struct listnode    empty_frame_list;
    struct listnode    valid_frame_list;
    pthread_mutex_t     mutex;
    int                 empty_num;
    int                 no_empty_frame_flag;
}venc_inbuf_manager;

typedef struct venc_comp_ctx
{
    int                     vencoder_init_flag;
    common_comp_ctx         *base_ctx;
    VideoEncoder*           vencoder;
    pthread_mutex_t         vencoder_mutex;
    comp_state_type         state;
    pthread_t               venc_thread;
    comp_callback_type*     callback;
    void*                   callback_data;
    component_type*         self_comp;
    venc_inbuf_manager      in_buf_manager;
    venc_outbuf_manager     out_buf_manager;
    comp_tunnel_info        in_port_tunnel_info;
    comp_tunnel_info        out_port_tunnel_info;
    venc_comp_base_config   base_config;
    venc_comp_normal_config normal_config;
    unsigned char*          vbvStartAddr;
    int                     had_get_sps_pps_data_flag;
    VencHeaderData          sps_pps_data;
    FILE*                   m_outFile;
    int                     bSaveStreamFlag;

}venc_comp_ctx;

error_type venc_comp_component_init(PARAM_IN comp_handle comp_param);
#endif

