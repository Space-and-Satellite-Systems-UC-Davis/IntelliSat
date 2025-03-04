
#ifndef SCHEDULER_GLOBALS_H
#define SCHEDULER_GLOBALS_H

#ifdef VIRTUAL
#define printMsg printf
#endif

#include "../inc/globals.h"

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "./../integration/integration.h"

#endif
