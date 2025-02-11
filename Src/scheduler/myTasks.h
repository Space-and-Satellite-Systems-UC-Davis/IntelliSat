#ifndef MY_TASKS_H
#define MY_TASKS_H

#include "process.h"


volatile extern EventGroupHandle_t events;

void setter(void *pvParameters);

void reciever(void *pvParameter);

#endif // MY_TASKS_H