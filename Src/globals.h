#ifndef REALOP1_GLOBALS_H_
#define REALOP1_GLOBALS_H_

#include "stm32l476xx.h"

typedef enum {false, true} bool;

typedef enum {false, true} bool;

// Global Variables
extern int core_MHz;
extern int systick_time;

extern bool button0;
extern bool button1;

void nop(long long nop_loops);


#endif // REALOP1_GLOBALS_H_
