#include "low_pwr.h"
#include "PWR/sleep_primitives.h"
#include "PWR/pwr_lp_mode.h"
#include "RTC/rtc.h"
#include "peripherals/PWRMON/INA226.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

/* Defined in virtual_rtos.c */
extern TaskHandle_t detumbleHandle;
extern TaskHandle_t experimentHandle;

#define LOW_PWR_POLL_MS 30000

/* ------------------------------------------------------------------ */
/*  Tickless idle hook                                                  */
/* ------------------------------------------------------------------ */

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime) {
    uint32_t seconds = xExpectedIdleTime / configTICK_RATE_HZ;
    if (seconds == 0) return;

    bool already_in_lp = g_system_lp_mode_active;
    if (!already_in_lp) PWR_enterLPRunMode();

    PWR_armRTC(seconds, NULL);

    /* Disable interrupts to close the race between armRTC and WFI.
     * If an ISR readies a task here, eTaskConfirmSleepModeStatus returns
     * eAbortSleep. ARM guarantees WFI returns immediately if an interrupt
     * is pending at the moment interrupts are re-enabled, so nothing is lost. */
    portDISABLE_INTERRUPTS();

    if (eTaskConfirmSleepModeStatus() == eAbortSleep) {
        portENABLE_INTERRUPTS();
        PWR_disarmRTC();
        if (!already_in_lp) PWR_exitLPRunMode();
        return;
    }

    uint32_t before = rtc_getUnixTime();

    portENABLE_INTERRUPTS();
    PWR_wfi();

    uint32_t after   = rtc_getUnixTime();
    /* Handle midnight rollover (RTC wraps at 86400 s). */
    uint32_t elapsed = (after >= before) ? (after - before)
                                         : (86400u - before + after);

    PWR_disarmRTC();
    if (!already_in_lp) PWR_exitLPRunMode();

    /* Cap at xExpectedIdleTime to prevent vTaskStepTick assertion. */
    TickType_t ticks = (TickType_t)(elapsed * configTICK_RATE_HZ);
    if (ticks > xExpectedIdleTime) ticks = xExpectedIdleTime;
    if (ticks > 0) vTaskStepTick(ticks);
}

/* ------------------------------------------------------------------ */
/*  Battery voltage monitor task                                        */
/* ------------------------------------------------------------------ */

void lowPwr_task(void *pvParameters) {
    (void)pvParameters;

    for (;;) {
        float voltage = pwrmon_getBusVoltage(BATMON_GPIO, BATMON_SCL, BATMON_SDA);

        if (!g_system_lp_mode_active
                && voltage < LP_MODE_ENTER_VOLTAGE_V) {

            pwr_enterSystemLPMode();
            /* Suspend ADCS tasks after peripheral shutdown — prevents
             * actuator commands using data from powered-down sensors. */
            if (detumbleHandle)   vTaskSuspend(detumbleHandle);
            if (experimentHandle) vTaskSuspend(experimentHandle);

        } else if (g_system_lp_mode_active
                && voltage > LP_MODE_ENTER_VOLTAGE_V + LP_MODE_EXIT_HYSTERESIS_V) {

            pwr_exitSystemLPMode();
            /* Resume only after peripherals are restored so the first
             * sensor reads see valid hardware. */
            if (detumbleHandle)   vTaskResume(detumbleHandle);
            if (experimentHandle) vTaskResume(experimentHandle);
        }

        vTaskDelay(pdMS_TO_TICKS(LOW_PWR_POLL_MS));
    }
}
