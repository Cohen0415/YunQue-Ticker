
#ifndef _CEDARE_RECORDER_
#define _CEDARE_RECORDER_

#define ENCODER_CHANNEL_NUM (2)

#define SAVE_FILE_PATH_MAX_LEN (64)

#ifdef __cplusplus
extern "C" {
#endif

typedef void* recorder;

typedef enum RECORDER_STREAM_OUTPUT_MODE
{
    STREAM_OUTPUT_MODE_WRITE_TO_FILE, //* write to file
    STREAM_OUTPUT_MODE_CALLBACK,      //* callback stream data to caller
}RECORDER_STREAM_OUTPUT_MODE;

typedef enum RECORDER_AENCODER_TYPE
{
    RECORDER_AENCODER_TYPE_AAC,
    RECORDER_AENCODER_TYPE_PCM,
    RECORDER_AENCODER_TYPE_MP3,
    RECORDER_AENCODER_TYPE_G711A,
}RECORDER_AENCODER_TYPE;

typedef enum RECORDER_YUV_FORMAT
{
    YUV_FORMAT_YUV420SP,
    YUV_FORMAT_YVU420SP,
    YUV_FORMAT_YUV420P,
    YUV_FORMAT_YVU420P,

    YUV_FORMAT_AFBC,
} RECORDER_YUV_FORMAT;

typedef enum RECORDER_VENCODER_TYPE
{
    RECORDER_VENCODER_TYPE_H264,
    RECORDER_VENCODER_TYPE_H265,
    RECORDER_VENCODER_TYPE_JPEG,
}RECORDER_VENCODER_TYPE;

typedef enum RECORDER_MUXER_TYPE
{
    RECORDER_MUXER_TYPE_MP4,
    RECORDER_MUXER_TYPE_TS,
}RECORDER_MUXER_TYPE;

typedef struct var_frame_rate_config {
    char                     bEnable;
    unsigned int             nOri_frame_rate;
    unsigned int             nFrame_rate_var;
} var_frame_rate_config_s;

typedef struct video_encoder_config_s {
    int enable;
    int scale_encoder_enable;
    int width;    // resolution width
    int height; // resolution height
    int frame_rate;    // fps
    int bitrate;// bitrate
    int I_frame_interval;
    int qp_min; //* 1 ~ 51, default 10
    int qp_max; //* 1 ~ 51, default 45
    int vbr;// VBR=1, CBR=0
    var_frame_rate_config_s    var_frame_rate_config;
    RECORDER_VENCODER_TYPE      encoder_type;
    RECORDER_STREAM_OUTPUT_MODE stream_output_mode;
} video_encoder_config_s;

typedef struct audio_encoder_config_s {
    int enable;
    int sample_rate;
    int sample_bits;
    int channels;
    int bit_rate;
    RECORDER_AENCODER_TYPE encoder_type;
} audio_encoder_config_s;

typedef struct yuv_config_s {
    int                 width;
    int                 height;
    RECORDER_YUV_FORMAT format;
} yuv_config_s;

typedef struct muxer_config_s {
    int                 enable;
    RECORDER_MUXER_TYPE muxer_type;
} muxer_config_s;

typedef struct recorder_config
{
    yuv_config_s           yuv_config;
    video_encoder_config_s venc_config[ENCODER_CHANNEL_NUM];
    audio_encoder_config_s aenc_config;
    muxer_config_s         muxer_config;
}recorder_config;

typedef struct recorder_video_stream_s
{
    int            id;
    long long      pts;
    unsigned int   flag;
    unsigned int   size0;
    unsigned int   size1;

    unsigned char* data0;
    unsigned char* data1;
    int            is_sps_pps_flag;

}recorder_video_stream_s;

typedef struct recorder_audio_stream_s
{
    int            id;
    long long      pts;
    unsigned int   size;
    unsigned char* data;

}recorder_audio_stream_s;

typedef struct recorder_yuv_buf_s
{
    unsigned int   id;
    long long      pts;
    unsigned char* phy_addr;
    unsigned char* vir_addr;
    unsigned char* phy_addr_c;
    unsigned char* vir_addr_c;
}recorder_yuv_buf_s;

typedef void (*video_stream_callback)(recorder_video_stream_s* video_stream,
                                               void* privateData);

typedef void (*audio_stream_callback)(recorder_audio_stream_s* audio_stream,
                                              void* privateData);

typedef void (*yuv_buffer_callback)(recorder_yuv_buf_s* yuv_buf, void* privateData);

//* create --> setconfig --> start --> stop --> destroy
recorder* recorder_create();

void recorder_destroy(recorder* recorder);

int recorder_setconfig(recorder* recorder, recorder_config* config);

int recorder_set_video_stream_callback(recorder* recorder, video_stream_callback cb,
                                                    int channel_id, void* privateData);

int recorder_set_audio_stream_callback(recorder* recorder, audio_stream_callback cb,
                                                    int channel_id, void* privateData);

int recorder_set_yuv_buffer_callback(recorder* recorder, yuv_buffer_callback cb,
                                                  void* privateData);
int recorder_start_next_file(recorder* recorder, char* file_name, int channel_id);

int recorder_start_next_file_by_fd(recorder* recorder, int fd, int channel_id);

int recorder_start(recorder* recorder, int channel_id);

int recorder_stop(recorder* recorder, int channel_id);

int recorder_pause(recorder* recorder, int channel_id);

int recorder_queue_yuv_buffer(recorder* recorder, recorder_yuv_buf_s* yuv_buf);

//* get the header(sps/pps) data, return the data len
//* 0: function error; -1: buf overflow, max_buf_size is not enought
int recorder_get_video_stream_header_data(recorder* recorder, int channel_id,
                                                         char* dst_buf, int max_buf_size);

void* get_aenc_comp_in_recorder(recorder* recorder);

#ifdef __cplusplus
}
#endif

#endif
