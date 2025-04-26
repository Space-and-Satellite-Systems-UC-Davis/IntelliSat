#include "watchdog.h"
#include <LED/led.h>
#include <SunSensors/sun_sensors.h>

void testFunction_watchdog() {
    whoReset();
    printMsg("start watchdog\n");
    led_d2(true);
    watchdog_iwdg_config(410);
    watchdog_wwdg_config(400);
    watchdog_kick();
}
