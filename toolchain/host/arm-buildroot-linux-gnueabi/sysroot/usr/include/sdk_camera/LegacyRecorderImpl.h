#ifndef __LEGACY_REC_CAMERA_H__
#define __LEGACY_REC_CAMERA_H__

#include <sdk_misc/Mutex.h>
#include <sdk_camera/aut_audioenc.h>
#include <sdk_camera/audio_hal.h>
#include <sdk_camera/LegacyRecorder.h>
#include <sdk_camera/type_camera.h>
#include <sdk_camera/DvrMode.h>
#include <sdk_camera/CameraHardware2.h>

using namespace android;

class LegacyRecorderImpl: public DvrMode
{

public:
    LegacyRecorderImpl(int cameraId);
    ~LegacyRecorderImpl();

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

    int getTVINSystemType()
    {
        mHardwareCamera->getTVINSystemType();
    }

    int setGpsData(void *gpsData)
    {
        return mHardwareCamera->sendCommand(CAMERA_CMD_SET_GPSDATA_RECORDER, (unsigned long)gpsData, 0);
    }

    int setWhiteBlanceData(int32_t Data)
    {
        return mHardwareCamera->sendCommand(CAMERA_CMD_SET_WHITE_BALANCE, Data, 0);
    }

    int setColorEffectData(int32_t Data)
    {
        return mHardwareCamera->sendCommand(CAMERA_CMD_SET_COLOR_EFFECT, Data, 0);
    }

    int setExposureCompensationData(int32_t Data)
    {
        return mHardwareCamera->sendCommand(CAMERA_CMD_SET_EXPOSURE_COMPENSATION, Data, 0);
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

    CameraManager *m360CameraManager;

    CameraHardware **m360Hardware;

    CameraHardware *mHardwareCamera;
private:
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
protected:
    AudioCapThread *mAudioDevice;
    // static status_t audioRecMuxerCb(int32_t msgType,
    //                                 char *dataPtr, int dalen,
    //                                 void *user);
};

#endif
