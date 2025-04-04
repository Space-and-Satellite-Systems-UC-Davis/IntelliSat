
#ifndef SCHEDULER_GLOBALS_H
#define SCHEDULER_GLOBALS_H

#ifdef VIRTUAL
#define printMsg printf
#endif

#include "globals.h"
#include "FreeRTOS.h"
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include "event_groups.h"

#include "../integration/integration.h"

#endif
