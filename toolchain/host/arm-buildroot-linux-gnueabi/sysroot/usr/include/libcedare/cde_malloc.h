/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : CdcMalloc.h
* Description :
* History :
*   Author  :
*   Date    : 2018/07/03
*   Comment :
*
*
*/

#ifndef  CDE_MALLOC_H
#define  CDE_MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

#define cde_malloc(size)            _cde_malloc (size, __FUNCTION__, __LINE__)
#define cde_calloc(elements, size)  _cde_calloc (elements, size, __FUNCTION__, __LINE__)


void *_cde_malloc(unsigned int size, const char * file, unsigned int line);
void *_cde_calloc(unsigned int elements, unsigned int size, const char * file, unsigned int line);
void cde_free(void * mem_ref);

void CdeMallocReport(void);
int CdeMallocGetInfo(char*  pDst, int nDstBufsize);
int CdeMallocCheckFlag();

#ifdef __cplusplus
}
#endif


#endif
