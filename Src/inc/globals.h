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
#define DEFAULT_TIMEOUT_MS 1000

void delay_ms(uint64_t ms);
void nop(long long nop_loops);
uint64_t getSysTime();

bool is_time_out(uint64_t start_time, uint64_t timeout_ms);
void while_timeout(void (*do_work)(), bool (*is_done)(), uint64_t timeout_ms);
void empty_while_timeout(bool (*is_done)(), uint64_t timeout_ms);
bool is_GPIOA_not_ready();
bool is_GPIOB_not_ready();
bool is_GPIOB_not_ready();
bool is_GPIOD_not_ready();
bool is_GPIOE_not_ready();
bool is_GPIOF_not_ready();
bool is_GPIOG_not_ready();

#endif // REALOP1_GLOBALS_H_
