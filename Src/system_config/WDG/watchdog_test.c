#include "watchdog.h"
#include <LED/led.h>
#include <SunSensors/sun_sensors.h>

void testFunction_watchdog() {
    //int x = sun_sensors_readVoltage(PANEL0,DIODE0);
    //printMsg("voltage: %f\n", x);
    //RCC->CSR |= RCC_CSR_RMVF;
    WWDG->CR |= WWDG_KICK;
    printMsg("start watchdog\n");
    led_d2(true);
    watchdog_config();
    watchdog_kick();
}
