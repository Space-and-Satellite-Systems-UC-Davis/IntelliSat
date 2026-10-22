#include "sleep_bare_metal.h"
#include "PWR/sleep_primitives.h"

static volatile OperatingMode mode = RUN;

void PWR_maintainLPSleep() {mode = LPSLEEP;}

bool PWR_enterLPSleepMode(uint16_t seconds) {
    PWR_enterLPRunMode();
    bool ok = PWR_armRTC(seconds, PWR_maintainLPSleep);
    if (!ok) {PWR_exitLPRunMode(); return false;}

    mode = LPSLEEP;
    while (mode == LPSLEEP) {
        mode = LPRUN;
        PWR_wfi();
    }

    PWR_disarmRTC();
    PWR_exitLPRunMode();
    return true;
}

