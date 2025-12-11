#ifdef DVR_USE_CEDARE
#ifndef __AWRECORDER_H__
#define __AWRECORDER_H__

#include <libcedare/recorder.h>

#include <sdk_misc/Mutex.h>
#include <sdk_camera/Recorder.h>
#include <sdk_camera/AWThread.h>

struct Config {
public:
    int cameraId;
    int input_width;
    int input_height;
    int input_format;
    int bitrate;
    int framerate;
    int duration;
    bool preallocate;

    bool video_channel_enable[2];

    char output_file_name[128];
    char output_dir[128];
    int output_file_fd;
    int output_width;
    int output_height;
    int output_encoder_type;
    int output_muxer_type;

    int sub_output_width;
    int sub_output_height;
    int sub_framerate;
    int sub_bitrate;

    bool yuvstream_cb_enable;
    // void** yuvstream_cb;
    yuv_buffer_callback yuvstream_cb;

    bool mainstream_cb_enable;
    // void** mainstream_cb;
    video_stream_callback mainstream_cb;

    bool substream_cb_enable;
    // void** substream_cb;
    video_stream_callback substream_cb;

    bool audiostream_cb_enable;
    // void** audiostream_cb;
    audio_stream_callback audiostream_cb;

    bool audio_enable;
    int  audio_channel_enable;
    int  audio_channel;
    int audio_sample_rate;
    //预分配
    bool prealloc;
};


class AWRecorder: public Recorder
{
public:
    AWRecorder();
    AWRecorder(char *name);
    ~AWRecorder();
public:
    int encodeInit(void *param);
    int encode(void *frame);
    int start();
    int stop();
    int pause();
    int release();

public:
    Config getConfig();
protected:
    int nextFile();

public:
    static bool onNextRecord(void *privData);
private:
    recorder *mRecorderCtrl;
    android::sp<AWThread> mNextRecordThread;
    Config mConfig;
    android::Mutex   mObjectLock;

public:
    char mName[128];

};

#endif
#endif
