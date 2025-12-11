/*
* Copyright (c) 2016-2022 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : cde_debug.h
* Description :
* History :
*   Author  : wangxiwang
*   Date    : 2020/06/15
*   Comment :
*
*
*/

#ifndef  CDE_DEBUG_H
#define  CDE_DEBUG_H

#include "cde_malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PRINTF_FUNCTION cde_logd("func = %s, line = %d", __FUNCTION__, __LINE__)

#define CEDARC_UNUSE(param) (void)param  //just for remove compile warning

#ifdef __cplusplus
}
#endif

#endif

