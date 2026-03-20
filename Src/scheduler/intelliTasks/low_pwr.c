#include "low_pwr.h"
#include "PWR/sleep_primitive.h"
#include "PWR/sleep_primitives.h"
#include "RTC/rtc.h"
#include <stdint.h>
#include <sys/errno.h>

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
    uint32_t seconds = (xExpectedIdleTime / configTICK_RATE_HZ);
    if (seconds == 0) return;

    PWR_enterLPRunMode();
    PWR_armRTC(seconds);

    if(eTaskConfirmSleepModeStatus() == eAbortSleep) {
        PWR_disarmRTC();
        PWR_exitLPSleepMode();
        return;
    }

    uint32_t before = rtc_getUnixTime();
    PWR_wfi();
    uint32_t elapsed = rtc_getUnixTime() - before;

    PWR_disarmRTC();
    PWR_exitLPRunMode();

    vTaskStepTick(elapsed * configTICK_RATE_HZ);
}