#ifndef __DVRFACTORY_H__
#define __DVRFACTORY_H__

#define USE_CDX_LOWLEVEL_API 1
#define USE_CDX_LOWLEVEL_API_AUDIO 1
#define USE_RECORD_AUDIO_API 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>

#include <libcedarc/vencoder.h>
#include <libcedarx/CdxMuxer.h>
#include <libcedarx/CdxParser.h>

#include <sdk_base/sdklog.h>
#include <sdk_storage/StorageManager.h>
#include <sdk_memory/sunxiMemInterface.h>
#include <sdk_camera/audio_hal.h>
#include <sdk_camera/V4L2CameraDevice2.h>
#include <sdk_camera/CallbackNotifier.h>
#include <sdk_camera/CameraHardware2.h>
#include <sdk_camera/MuxerWriter.h>
#include <sdk_camera/Rtc.h>
#include <sdk_camera/CameraManager.h>
#include <sdk_camera/aw_ini_parser.h>
#include <sdk_camera/CameraDebug.h>
#include <sdk_camera/Recorder.h>
#include <sdk_camera/RecordCamera.h>
#include <sdk_camera/LegacyRecorderImpl.h>
#include <sdk_g2d/G2dApi.h>

#ifdef USE_RECORD_AUDIO_API
#include <sdk_camera/aut_audioenc.h>
#endif

using namespace android;

class dvr_factory
{

public:
    dvr_factory(int cameraId);
    ~dvr_factory();

    int start();
    int stop();

    bool initializeDev(CameraHardware *pHardware);
    bool uninitializeDev();

    int recordInit();
    int startRecord();
    int stopRecord();

    int startPriview(struct view_info vv);
    int stopPriview();

    int takePicture();

    int SetDataCB(usr_data_cb cb, void *user)
    {
        // if (mRecordCamera) {
        //     mRecordCamera->SetDataCB(cb, ((dvr_factory*)user)->mRecordCamera);
        // }
        usrDatCb = cb;
        mCbUserDat = user;
        return 0;
    }

    void setCallbacks(notify_callback notify_cb,
                      data_callback data_cb,
                      data_callback_timestamp data_cb_timestamp,
                      void *user);

    //water mark
    bool enableWaterMark();
    bool disableWaterMark();
    status_t setWaterMarkMultiple(const char *str);

    #ifdef ADAS_ENABLE
    bool enableAdas();
    bool disableAdas();
    status_t setAdasParameters(int key, int value);
    bool setAdasCallbacks(camera_adas_data_callback adas_data_cb);
    #endif

    int updateHardwareInfo(CameraHardware *p, int id);
    int add360Hardware();

    int getTVINSystemType()
    {
        mHardwareCameras->getTVINSystemType();
    }

    int setGpsData(void *gpsData)
    {
        return mHardwareCameras->sendCommand(CAMERA_CMD_SET_GPSDATA_RECORDER, (unsigned long)gpsData, 0);
    }

    int setWhiteBlanceData(int32_t Data)
    {
        return mHardwareCameras->sendCommand(CAMERA_CMD_SET_WHITE_BALANCE, Data, 0);
    }

    int setColorEffectData(int32_t Data)
    {
        return mHardwareCameras->sendCommand(CAMERA_CMD_SET_COLOR_EFFECT, Data, 0);
    }

    int setExposureCompensationData(int32_t Data)
    {
        return mHardwareCameras->sendCommand(CAMERA_CMD_SET_EXPOSURE_COMPENSATION, Data, 0);
    }

    int mCameraId;
    int recordwith;
    int recordheith;

    sp<AudioCapThread> mAudioCap;
    int mAudioHdl;
    AUDIO_ENCODER_TYPE mAudioEncType;

    int setAudioOps(sp<AudioCapThread> act, int hdl)
    {
        ALOGV("setAudioOps, hdl:%d", hdl);
        mAudioCap = act;
        mAudioHdl = hdl;
        return 0;
    }

    static void __dvr_audio_data_cb_timestamp(int32_t msgType, nsecs_t timestamp,
                                              int card, int device,
                                              char *dataPtr, int dsize, void *user);

    void SetCurRecordStat(int iCurRedStat)
    {
        mCurRecordStat = iCurRedStat;
    }

    int GetCutRecortStat()
    {
        return mCurRecordStat;
    }

    RecordCamera *mRecordCamera;

    // LegacyRecorder* mRecordCamera;

    Recorder *mRecorder;

    CameraManager *m360CameraManager;

    CameraHardware **m360Hardware;

    CameraHardware *mHardwareCameras;
private:
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

    notify_callback         mNotifyCb;
    data_callback           mDataCb;
    data_callback_timestamp mDataCbTimestamp;
    void *mCbUser;

    //h264 data cb
    usr_data_cb usrDatCb;
    void *mCbUserDat;

    //audio encoder
    AutAudioEnc *mDvrAudioEnc;

    int mCurRecordStat;
    // Config mEncodeConfig;
protected:
    /* Locks this instance for parameters, state, etc. change. */
    // Mutex   mObjectLock;
};

//}

#endif
