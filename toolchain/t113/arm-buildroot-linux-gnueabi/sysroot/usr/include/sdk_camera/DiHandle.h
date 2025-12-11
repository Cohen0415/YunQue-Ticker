#include <pthread.h>

#include <sdk_memory/sunxiMemInterface.h>
#include <sdk_camera/deinterlace2.h>
#include <sdk_camera/type_camera.h>
#include <sdk_camera/DiProcess.h>

#define DI_HANDLE_BUFFER 3
#define MAX_DI_WIDTH 736
#define MAX_DI_HEIGHT 576

//namespace android {

class DiHandle
{

public:
    DiHandle();
    ~DiHandle();
    int mDiReadId;
    int DiHandleInit(int in_w, int in_h);
    int DiHandleUnInit();
    V4L2BUF_t *DiProcessLoop(V4L2BUF_t *buf, int savePicBeforeDi, int enableTimeLog);
    V4L2BUF_t mdiBuffer[DI_HANDLE_BUFFER];

private:
    int mSkipCount;
    int mDiWidth;
    int mDiHeight;
    int mAbandonFrameCnt;
    int mBuffInitFlag;  //use to note if Di has alloc ion
    DiProcess  *mDiProcess;
    V4L2BUF_t mCamBuff[DI_HANDLE_BUFFER];
    int saveframe(char *str, void *p, int length, int is_oneframe);
};

//};
