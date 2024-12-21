#ifndef REALOP1_GLOBALS_H_
#define REALOP1_GLOBALS_H_

#include "stm32l476xx.h"

#define OP_REV 3	// the Orbital Platform revision being programmed

#if OP_REV == 1 || OP_REV == 2 || OP_REV == 3
#define UNUSED_GPIO             GPIOG, 1
#define LOCK_UNUSED_GPIO_HIGH() GPIOG->PUPDR |= (1 << GPIO_PUPDR_PUPD1)
#endif

typedef enum {false, true} bool;
#define NULL 0

#define WILL_LOOPS_TIMEOUT true

void delay_ms(uint64_t ms);
void nop(long long nop_loops);
uint64_t getSysTime();
void while_timeout(void (*do_work)(), bool (*is_done)(), uint64_t timeout_ms)

#endif // REALOP1_GLOBALS_H_
