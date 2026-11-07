#pragma once
#include <mutex>
#include "freertos/FreeRTOS.h"
using SemaphoreHandle_t=std::mutex*;
inline SemaphoreHandle_t xSemaphoreCreateMutex(){return new std::mutex;}
inline int xSemaphoreTake(SemaphoreHandle_t m,TickType_t){m->lock();return 1;}
inline int xSemaphoreGive(SemaphoreHandle_t m){m->unlock();return 1;}
