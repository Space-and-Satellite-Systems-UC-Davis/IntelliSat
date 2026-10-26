/*
 * startup_reset.c
 *
 * Production side of the reset boundary declared in startup_reset.h. Startup
 * fails closed by commanding the pyro OFF and then requesting a reset; a
 * permanent fault therefore shows up as a documented reset loop rather than as
 * an inhibited board waiting forever.
 */

#include "startup/startup_reset.h"

#include <stm32l476xx.h>

void startup_request_reset(void) {
    NVIC_SystemReset();
}
