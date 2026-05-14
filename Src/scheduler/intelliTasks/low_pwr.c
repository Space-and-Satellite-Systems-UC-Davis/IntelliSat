#include "low_pwr.h"
#include "PWR/sleep_primitives.h"
#include "RTC/rtc.h"
#include <stdint.h>

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
    uint32_t seconds = (xExpectedIdleTime / configTICK_RATE_HZ);
    if (seconds == 0) return;

    PWR_enterLPRunMode();
    PWR_armRTC(seconds, NULL);

    if(eTaskConfirmSleepModeStatus() == eAbortSleep) {
        PWR_disarmRTC();
        PWR_exitLPRunMode();
        return;
    }

    uint32_t before = rtc_getUnixTime();
    PWR_wfi();
    uint32_t after = rtc_getUnixTime();
    uint32_t elapsed =  (after >= before) ? (after - before) : (86400 - before + after);
    TickType_t ticks = elapsed * configTICK_RATE_HZ;
    if (ticks > xExpectedIdleTime) ticks = xExpectedIdleTime;

    PWR_disarmRTC();
    PWR_exitLPRunMode();

    vTaskStepTick(ticks);
}
