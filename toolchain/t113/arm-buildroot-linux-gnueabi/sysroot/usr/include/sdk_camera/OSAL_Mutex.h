#ifndef __OSAL_MUTEX__
#define __OSAL_MUTEX__

#include <sdk_camera/OMX_Types.h>
#include <sdk_camera/OMX_Core.h>

#ifdef __cplusplus
extern "C" {
#endif

OMX_ERRORTYPE OSAL_MutexCreate(OMX_HANDLETYPE *mutexHandle);
OMX_ERRORTYPE OSAL_MutexTerminate(OMX_HANDLETYPE mutexHandle);
OMX_ERRORTYPE OSAL_MutexLock(OMX_HANDLETYPE mutexHandle);
OMX_ERRORTYPE OSAL_MutexUnlock(OMX_HANDLETYPE mutexHandle);

#ifdef __cplusplus
}
#endif
#endif
