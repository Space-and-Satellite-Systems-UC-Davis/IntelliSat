#include <LED/led.h>
#include <RTC/rtc.h>
#include <print_scan.h>

#define STARTUP_STATE_REG RTC->BKP0R
#define FLAG_NOT_RUN 0x00000000
#define FLAG_WAITING 0x00000001
#define FLAG_WAIT_DONE 0x00000002

void wait30_callback() { rtc_writeToBKPNumber(FLAG_WAIT_DONE, 0); }

void testFunction_Startup_Simulation() {

  // if first time running satellite
  if (STARTUP_STATE_REG == FLAG_NOT_RUN) {
    printMsg("RTC has not been run.\r\n");

    printMsg("TODO: save sattelite state.\r\n");

    rtc_writeToBKPNumber(FLAG_WAITING, 0);
  }

  // rtc interrupt after 30 minutes
  if (STARTUP_STATE_REG == FLAG_WAITING) {
    printMsg("TODO: reset initial state.\r\n");

    rtc_deleteAllEntries();
    rtc_scheduleCallback(0, 30, 0, false, wait30_callback);

    while (STARTUP_STATE_REG != FLAG_WAIT_DONE) {
      __WFI();
    }

    printMsg("TODO: Burn wire on");
  }
}
