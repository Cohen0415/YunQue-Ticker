#ifndef __DVR_MODEL_H
#define __DVR_MODEL_H

#include <sdk_camera/type_camera.h>
#include <sdk_camera/CameraHardware2.h>
using namespace android;
class DvrMode
{

public:
    virtual ~DvrMode()
    {

    };

    virtual int openDevice() = 0;
    virtual int startPriview(view_info vv) = 0;
    virtual int stopPriview() = 0;

    virtual int recordInit() = 0;
    virtual int startRecord() = 0;
    virtual int stopRecord() = 0;
    virtual int takePicture() = 0;
    virtual void setCallbacks(notify_callback notify_cb, data_callback data_cb, data_callback_timestamp data_cb_timestamp, void *user) = 0;
    virtual bool enableWaterMark() = 0;
    virtual bool disableWaterMark() = 0;
    virtual int setWaterMarkMultiple(const char *str) = 0;
    virtual int release() = 0;
    virtual int getCameraId() = 0;
    virtual int SetDataCB(usr_data_cb cb, void *user) = 0;

};

//}

#endif
