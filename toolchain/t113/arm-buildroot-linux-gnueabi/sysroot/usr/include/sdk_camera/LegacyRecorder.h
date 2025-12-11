#ifndef __LEGACY_RECORDER_H__
#define __LEGACY_RECORDER_H__

#include <sdk_camera/type_camera.h>

using namespace android;

class LegacyRecorder
{

public:
    virtual int videoEncDeinit() = 0;
    virtual int videoEncParmInit(int sw, int sh, int dw, int dh, unsigned int bitrate, int framerate, int type, int pix = 0) = 0;
    virtual int setDuration(int sec) = 0;
    virtual int encode(V4L2BUF_t *pdata, char *outbuf, int *outsize, void *user) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int startRecord() = 0;
    virtual int stopRecord() = 0;
    virtual void setCallbacks(notify_callback notify_cb, void *user) = 0;
    virtual void dataCallback(int32_t msgType, char *dataPtr, camera_frame_metadata_t *metadata, void *user) = 0;
    virtual status_t dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, char *dataPtr, void *user) = 0;
    virtual int SetDataCB(usr_data_cb cb, void *user) = 0;
    virtual int getRecordState() = 0;

public:
    usr_data_cb mAudioRecMuxerCb;
    // static status_t audioRecMuxerCb(int32_t msgType,
    //                                 char *dataPtr, int dalen,
    //                                 void *user);
};

#endif
