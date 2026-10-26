#pragma once
#include <stdint.h>
#include <stdbool.h>
void delay_ms(uint64_t ms);
uint64_t getSysTime(void);
bool is_time_out(uint64_t start_time, uint64_t timeout_ms);
