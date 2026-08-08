#include <LED/led.h>
#include <print_scan.h>

typedef enum {
    INITIAL_CHECKS,
    WAITING_FOR_UPLINK,
    PERFORM_HANDSHAKE_1,
    PERFORM_HANDSHAKE_2,
    PERFORM_HANDSHAKE_3,

    MAIN_OPS,
} MCUState;

void received_uplink() { return true; }

void testFunction_Startup_Simulation()
{
    MCUState state = INITIAL_CHECKS;
    int t = 0;

    while (true) {
        if (t == 0) {
            printMsg("t = 0\r\n");
        }

        if (t == 30) {
            printMsg("t = 30\r\n");
            printMsg("power on.\r\n");
            printMsg("burn wire on.\r\n");
        }

        switch(state) {
            case INITIAL_CHECKS:
                // RTC
                if (RTC->BKP0R == 0x00000000) {
                    printMsg("RTC has not been run.\r\n");
                } else {
                    printMsg("RTC has been run.\r\n");
                }

                // Check watchdog

                // Check I2C, SPI, UART

                state = WAITING_FOR_UPLINK;
                break;
            case WAITING_FOR_UPLINK:
                break;
            case MAIN_OPS:
                break;
        }
        delay_ms(10);
        t += 1;
    }

}
