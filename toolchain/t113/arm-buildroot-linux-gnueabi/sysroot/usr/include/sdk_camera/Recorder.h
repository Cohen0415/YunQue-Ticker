#ifndef __RECORDER_BASE_H__
#define __RECORDER_BASE_H__

class Recorder
{

public:
    virtual int encodeInit(void *param)  = 0;
    virtual int encode(void *frame) = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual int pause() = 0;
    virtual int release() = 0;
};

#endif
