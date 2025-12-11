/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : aencoder.h
 * Description : audio encoder api
 * History :
 *
 */

#ifndef AUDIO_ENCODER_COMPONENT_H
#define AUDIO_ENCODER_COMPONENT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "common.h"
#include "aencoder.h"
#include "tmessage.h"

#define REQUEST_EXIT "EXIT"
#define ENABLE_CAPTURE_THREAD (0)
#define AENC_OUT_BUFFER_LIST_NODE_NUM (32)
typedef struct AudioInputBuffer
{
    char            *pData;          //data buff
    int             nLen;            //data len
    long long       nPts;            //pts
}AudioInputBuffer;

typedef struct VideoOutBuffer
{
    unsigned int   nSize0;
    unsigned int   nSize1;
    unsigned char* pData0;
    unsigned char* pData1;
}VideoOutBuffer;

typedef struct AudioEncOutBuffer
{
    char       *pBuf;
    unsigned int        len;
    long long  pts;
    int        bufId;
}AudioEncOutBuffer;

enum AUDIOENCODERNOTIFY
{
    AUDIO_ENCODE_NOTIFY_ERROR,
    AUDIO_ENCODE_NOTIFY_CRASH,
    AUDIO_ENCODE_NOTIFY_EOS,
};

typedef struct audio_stream_node
{
    AudioEncOutBuffer audio_stream;
    struct listnode mList;
}audio_stream_node;

typedef struct aenc_outbuf_manager
{
    struct listnode    empty_stream_list;
    struct listnode    valid_stream_list;
    pthread_mutex_t     mutex;
    int                 empty_num;
    int                 no_empty_stream_flag;
}aenc_outbuf_manager;

struct aenc_cap_config
{
    unsigned int card;
    unsigned int device;
    unsigned int channels;
    unsigned int rate;
    unsigned int bits;
    unsigned int period_size;
    unsigned int period_count;
};

typedef struct _AwAudioEncoderItf
{
    void *mLibHandle;
    AudioEncoder* libencoder;
    AudioEncoder* (*CreateAudioEncoder)(void);
    void (*DestroyAudioEncoder)(AudioEncoder* pEncoder);
    int (*InitializeAudioEncoder)(AudioEncoder *pEncoder, AudioEncConfig *pConfig);
    int (*ResetAudioEncoder)(AudioEncoder* pEncoder);
    int (*EncodeAudioStream)(AudioEncoder *pEncoder);
    int (*WriteAudioStreamBuffer)(AudioEncoder *pEncoder, char* pBuf, int len);
    int (*RequestAudioFrameBuffer)(AudioEncoder *pEncoder, char **pOutBuf,
                                unsigned int *size, long long *pts, int *bufId);
    int (*ReturnAudioFrameBuffer)(AudioEncoder *pEncoder, char *pOutBuf,
                                unsigned int size, long long pts, int bufId);
} AwAudioEncoderItf;

typedef struct aenc_comp_ctx
{
    common_comp_ctx*        base_ctx;
    AudioEncoder*           aencoder;
    AudioEncConfig          config;
    struct aenc_cap_config  aenc_cap_config;
    comp_callback_type*     callback;
    void*                   callback_data;
    aenc_outbuf_manager     out_buf_manager;
    component_type*         self_comp;
    pthread_t               aenc_thread;
#if ENABLE_CAPTURE_THREAD
    pthread_t               aenc_cap_thread;
#endif
    comp_state_type         state;
    int                     mThreadCreated;
    unsigned int            bDestory;
    AwAudioEncoderItf       AencItf;
}aenc_comp_ctx;


/*******************aencoder pcm capture start********************/
#define ID_RIFF 0x46464952
#define ID_WAVE 0x45564157
#define ID_FMT  0x20746d66
#define ID_DATA 0x61746164

#define FORMAT_PCM 1

struct wav_header {
    unsigned int riff_id;
    unsigned int  riff_sz;
    unsigned int  riff_fmt;
    unsigned int  fmt_id;
    unsigned int  fmt_sz;
    unsigned short audio_format;
    unsigned short num_channels;
    unsigned int  sample_rate;
    unsigned int  byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
    unsigned int  data_id;
    unsigned int  data_sz;
};
/*******************aencoder pcm capture end********************/

error_type aenc_comp_get_state(
        PARAM_IN  comp_handle comp_param,
        PARAM_OUT comp_state_type* pState);

error_type aenc_comp_empty_this_in_buffer(
        PARAM_IN  comp_handle comp_param,
        PARAM_IN  comp_buffer_header_type* pBuffer);

error_type aenc_comp_set_config(
        PARAM_IN  comp_handle comp_param,
        PARAM_IN  comp_index_type index,
        PARAM_IN  void* param_data);

error_type aenc_comp_init(
        PARAM_IN  comp_handle comp_param);

error_type aenc_comp_start(
        PARAM_IN  comp_handle comp_param);

error_type aenc_comp_component_init(PARAM_IN comp_handle component);
void * thread_process_aenc(void *param);
void* thread_aenc_capture_sample(void *param);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_ENC_API_H

