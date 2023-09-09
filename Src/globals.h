#ifndef REALOP1_GLOBALS_H_
#define REALOP1_GLOBALS_H_

#include "stm32l476xx.h"


// Global Variables
extern int core_MHz;
extern int systick_time;
extern int heartbeat_counter;
extern int ag_counter;

void nop(long long nop_loops);



#endif // REALOP1_GLOBALS_H_
