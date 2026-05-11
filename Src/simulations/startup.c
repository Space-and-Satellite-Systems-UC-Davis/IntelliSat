#include <LED/led.h>
#include <print_scan.h>

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

void testFunction_Startup_Simulation() {
  MCUState state = INITIAL_CHECKS;
  int t = 0;
  int state_start_t = 0;

  bool t0_printed = false;
  bool t30_printed = false;

  while (true) {
    if (!t0_printed && t >= 0) {
      printMsg("t = 0\r\n");
      t0_printed = true;
    }

    if (!t30_printed && t == 30) {
      printMsg("t = 30\r\n");
      t30_printed = true;
    }

    switch (state) {
    case INITIAL_CHECKS:
      // RTC
      if (RTC->BKP0R == 0x00000000) {
        printMsg("RTC has not been run.\r\n");
      } else {
        printMsg("RTC has been run.\r\n");
      }

      // Check watchdog

      // Check I2C, SPI, UART

      state = WAIT_POWER_ON;
      state_start_t = t;
      break;
    case WAIT_POWER_ON:
      if (t - state_start_t >= 30) {
        printMsg("power on.\r\n");
        printMsg("burn wire on.\r\n");

        // go initiate wait for handshake
        state = WAITING_FOR_UPLINK;
        state_start_t = t;
      }
      break;
    case WAITING_FOR_UPLINK:
      printMsg("Waiting for handshake...\r\n");
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
