#include "watchdog.h"
#include <LED/led.h>
#include <SunSensors/sun_sensors.h>

void testFunction_watchdog() {
    printMsg("start watchdog\n");
    int count = 0;
    while(1){
        delay_ms(1000);
        count ++;
        if(count  == 3){
            watchdog_changeIWDGTimeout(5000);
        }
        if(count < 15){
            continue;
        }
        printMsg("uh oh\n");
        int x = sun_sensors_readVoltage(PANEL0,DIODE0);
        printMsg("voltage: %f\n", x);
    }
}
