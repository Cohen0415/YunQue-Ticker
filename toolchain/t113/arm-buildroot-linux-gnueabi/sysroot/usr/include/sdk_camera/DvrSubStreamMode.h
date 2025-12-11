#ifndef __DVR_SUBSTREAM_MODEL_H
#define __DVR_SUBSTREAM_MODEL_H

#include <sdk_camera/type_camera.h>
#include <sdk_camera/Recorder.h>
#include <sdk_camera/CameraHardware2.h>
#include <sdk_camera/CameraManager.h>
#include <sdk_camera/DvrNormalMode.h>

using namespace android;

class DvrSubStreamMode : public DvrNormalMode
{
public:
    DvrSubStreamMode(int cameraId)
        : DvrNormalMode(cameraId)
    {

    }

    int recordInit();
};

#endif
