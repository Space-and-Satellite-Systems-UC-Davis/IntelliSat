#include <LED/led.h>
#include <RTC/rtc.h>
#include <print_scan.h>

#define SAT_FIRST_RUN_REG RTC->BKP0R
#define FLAG_SAT_ALREADY_RAN 0x00000001

#define WAIT_30_REG RTC->BKP1R
#define FLAG_WAIT_DONE 0x00000001

typedef enum {
  INITIAL_CHECKS,     // initiate MCU
  WAIT_POWER_ON,      // wait until t=30 to power on
  WAITING_FOR_UPLINK, //(a) receive uplink
  SAT_TO_GROUND_1,    // perform handshake: sat to ground
  GROUND_TO_SAT_1,    // perform handshake: ground to sat
  SAT_TO_GROUND_2,    // perform handshake: sat to ground again
  MAIN_OPS,           // main ops after handshake
  DIAGNOSTICS_MODE    // receved nothing after 2 weeks
} MCUState;

bool received_uplink() { return true; }
bool received_ground_signal() { return true; }

void wait30_callback() { rtc_writeToBKPNumber(FLAG_WAIT_DONE, 1); }

void testFunction_Startup_Simulation() {
  MCUState state = INITIAL_CHECKS;
  int t = 0;
  int state_start_t = 0;

  bool t0_printed = false;
  bool t30_printed = false;

  while (true) {
    if (!t0_printed && t >= 0) {
      printMsg("t = 0 minutes\r\n");
      t0_printed = true;
    }

    if (!t30_printed && t == 30 * 60) {
      printMsg("t = 30 minutes\r\n");
      t30_printed = true;
    }

    switch (state) {
    case INITIAL_CHECKS:

      // TODO: Check watchdog

      // TODO: Check I2C, SPI, UART

      // TODO: Check RTC

      // Has the satellite run the first time?
      // NO: Save state of satellite
      // YES: Continue
      if (SAT_FIRST_RUN_REG != FLAG_SAT_ALREADY_RAN) {
        printMsg("RTC has not been run.\r\n");

        printMsg("TODO: save sattelite state.\r\n");

        // 1. Remember satellite already ran
        // 2. Reset the 30 minute wait flag
        rtc_writeToBKPNumber(FLAG_SAT_ALREADY_RAN, 0);
        rtc_writeToBKPNumber(0, 1);

      } else {
        printMsg("RTC has been run.\r\n");
      }

      // Is the satellite running for the first time / completed the do nothing
      // for 30 minutes?
      // NO: do nothing for 30 minutes, use interrupt to wake after 30
      // YES: skip to the uplink / handshake process
      if (WAIT_30_REG != FLAG_WAIT_DONE) {
        printMsg("TODO: reset initial state.\r\n");


        // delete previous callbacks and schedule another 30 minute one
        rtc_deleteAllEntries();
        rtc_scheduleCallback(0, 30, 0, false, wait30_callback);

        state = WAIT_POWER_ON;
        state_start_t = t;
      } else {
        state = WAITING_FOR_UPLINK;
        state_start_t = t;
      }
      break;

    case WAIT_POWER_ON:
      // Has the satellite completed the 30 minute wait? 
      // YES: burn wire on
      if (WAIT_30_REG == FLAG_WAIT_DONE) {
        printMsg("TODO: burn wire on.\r\n");

        // go initiate wait for handshake
        state = WAITING_FOR_UPLINK;
        state_start_t = t;
      }
      break;

    case WAITING_FOR_UPLINK:
      printMsg("Waiting for handshake...\r\n");

      // has satellite received the initial uplink?
      // YES: Initiate handshake process.
      // NO: if it's been 2 weeks, go into diagnostics mode and run the burn wire
      if (received_uplink()) {
        printMsg("Uplink received. Peform handshake. \r\n");

        // initiate handshake
        state = SAT_TO_GROUND_1;
        state_start_t = t;
      } else if (t - state_start_t >= 1.21e6) {
        printMsg("No uplink received for 2 weeks.");
        printMsg("Running burn wire again if there's charge.");

        // go into diagnostics mode
        state = DIAGNOSTICS_MODE;
        state_start_t = t;
      }
      break;

    case SAT_TO_GROUND_1:
      printMsg("Handshake step 1: satellite -> ground.\r\n");
      // transmit to ground

      state = GROUND_TO_SAT_1;
      state_start_t = t;
      break;

    case GROUND_TO_SAT_1:
      printMsg("Handshake step 2: waiting for ground -> satellite "
               "confirmation.\r\n");
               
      if (received_ground_signal()) {
        printMsg("Ground confirmation received.\r\n");

        state = SAT_TO_GROUND_2;
        state_start_t = t;
      }
      break;

    case SAT_TO_GROUND_2:
      printMsg(
          "Handshake step 3: satellite confirms it knows it can transmit.\r\n");
      // transmit to ground
      printMsg("Handshake complete. Entering main ops.\r\n");

      state = MAIN_OPS;
      state_start_t = t;
      break;

    case DIAGNOSTICS_MODE:
      printMsg("Diagnostics mode: transmitting diagnostics...\r\n");

      // if received an uplink, go back to performing handshake
      if (received_uplink()) {
        printMsg("uplink received. performing handshake... \r\n");
        state = SAT_TO_GROUND_1;
        state_start_t = t;
      }
      break;

    case MAIN_OPS:
      printMsg("Run main mission ops.. \r\n");
      // check TLE
      // Bad TLE / next upload + handshake (hs)
      // TLE not working accurately -> periodic downlinking (high bat / low bat)
      // listen for uplink -> then immediately downlink
      // Good TLE / next upload + hs works
      // periodic downlinking
      // listen for uplink -> immediate downlink [high bat / (low bat -> we
      // didn't know we are in comms window)] comms open -> immediate uplink
      // wait x min. for uplink
      // if nothing then downlink
      // Note: uplink and downlink can't be done within same period
      // (*) ground station should have ability to can command sat which window
      // to uplink or downlink (TBD)
      break;
    }
    delay_ms(1000);
    t += 1;
  }
}
