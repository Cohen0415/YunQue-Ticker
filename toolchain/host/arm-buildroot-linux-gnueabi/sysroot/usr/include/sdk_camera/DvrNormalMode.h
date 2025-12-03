#ifndef __DVR_NOR_MODEL_H
#define __DVR_NOR_MODEL_H

#include <sdk_misc/Mutex.h>
#include <sdk_camera/audio_hal.h>
#include <sdk_camera/type_camera.h>
#include <sdk_camera/DvrMode.h>
#include <sdk_camera/CameraHardware2.h>

#ifdef DVR_USE_CEDARE
#include <sdk_camera/AWRecorder.h>
#else
#include <sdk_camera/Recorder.h>
#endif

using namespace android;

class DvrNormalMode: public DvrMode
{
public:
    DvrNormalMode(int cameraId);
    ~DvrNormalMode();
public:
    int openDevice();
    int startPriview(view_info vv);
    int stopPriview();
    int recordInit();
    int startRecord();
    int stopRecord();
    int takePicture();
    void setCallbacks(notify_callback notify_cb, data_callback data_cb, data_callback_timestamp data_cb_timestamp, void *user);
    bool enableWaterMark();
    bool disableWaterMark();
    int setWaterMarkMultiple(const char *str);
    int release();
    int getCameraId();
    int SetDataCB(usr_data_cb cb, void *user);


protected:
    int updateHardwareInfo(CameraHardware *p, int id);
    bool initializeDev(CameraHardware *pHardware);
    bool uninitializeDev();

    static void notifyCallback(int32_t msgType, int32_t ext1,
                               int32_t ext2, void *user);

    static void __notify_cb(int32_t msg_type, int32_t ext1,
                            int32_t ext2, void *user);

    static void __data_cb(int32_t msg_type,
                          char *data,
                          camera_frame_metadata_t *metadata,
                          void *user);

    static void __data_cb_timestamp(nsecs_t timestamp, int32_t msg_type,
                                    char *data,
                                    void *user);


    #ifdef DVR_USE_CEDARE
public:
    static void dvr_yuv_buffer_cb(recorder_yuv_buf_s *yuv_buf, void *privateData);
    static void dvr_main_stream_callback(recorder_video_stream_s *video_stream, void *privateData);
    static void dvr_sub_stream_callback(recorder_video_stream_s *video_stream, void *privateData);
    static void dvr_audio_stream_callback(recorder_audio_stream_s *audio_stream, void *privateData);
    Config mEncodeConfig;

    #endif
    static void dvr_audiodata_callback(int32_t msgType, nsecs_t timestamp, int card, int device, char *dataPtr, int dsize, void *user);

protected:
    notify_callback         mNotifyCb;
    data_callback           mDataCb;
    data_callback_timestamp mDataCbTimestamp;
    void *mCbUser;
    //h264 data cb
    usr_data_cb usrDatCb;
    void *mCbUserDat;

    int mCameraId;
    AudioCapThread *mAudioDevice;
    Recorder *mRecorder;
    Mutex   mObjectLock;
public:
    CameraHardware *mHardwareCamera;


};

//}

#endif
