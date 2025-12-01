#pragma once

#include <FreeRTOS.h>
#include <task.h>

extern void* volatile pxCurrentTCB;

UBaseType_t uxTaskTieBreakerGet();
void vTaskTieBreakerSet(UBaseType_t value);
