#ifndef __AW_THREAD__
#define __AW_THREAD__
#include <functional>
#include <sdk_misc/Thread.h>

/**
 * author:xubaipei@allwinnertech.com
 * easy use thread
*/
// typedef bool (*RunThread)();

class AWThread: public android::Thread
{
private:
    char *mName;
    std::function<bool(void *)> mFunc;
    void *mPrivData;
    bool mExit = false;
public:
    AWThread(char *name, std::function<bool(void *)> fun, void *privData)
        : mName(name),
          mPrivData(privData),
          mFunc(fun)
    {

    }

    virtual bool threadLoop()
    {
        if (mExit) {
            return false;
        }
        return mFunc(mPrivData);
    }
    void start()
    {
        run(mName, android::PRIORITY_URGENT_DISPLAY);
    }
    void stop()
    {
        mExit = true;
        join();
    }
};

#endif
