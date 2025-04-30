#include "watchdog.h"
#include <LED/led.h>
#include <SunSensors/sun_sensors.h>

void testFunction_watchdog() {
    printMsg("start watchdog\n");
    led_d2(true);
    watchdog_config(420);
    int count = 0;
    while(1){
        delay_ms(5000);
        printMsg("uh oh\n");
        int x = sun_sensors_readVoltage(PANEL0,DIODE0);
        printMsg("voltage: %f\n", x);
    }
}
