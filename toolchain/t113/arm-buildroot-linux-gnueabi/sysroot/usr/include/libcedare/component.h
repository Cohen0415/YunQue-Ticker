#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "common.h"

typedef struct comp_registry
{
    char             name[20];
    component_init   comp_init_fn;
}comp_registry;

error_type comp_get_handle(
    PARAM_OUT comp_handle* pHandle,
    PARAM_IN  char* cComponentName,
    PARAM_IN  void* pAppData,
    PARAM_IN  comp_callback_type* pCallBacks);

error_type comp_free_handle(
    PARAM_IN  comp_handle component);

uint64_t get_cur_time_us(void);

#endif

