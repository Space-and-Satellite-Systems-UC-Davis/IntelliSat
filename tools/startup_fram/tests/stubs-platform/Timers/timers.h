#pragma once

#include <stdbool.h>

/* Host replacement for Src/system_config/Timers/timers.h (TIM7 millisecond base). */
bool heartbeat_timer_init(void);
