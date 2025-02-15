#include "adc_sun_sensors.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_ADC_Reads() {
    adc_init();
	
	while (1) {
		if (get_buttonStatus_SW1()) {
			uint16_t voltage = adc_readVoltageFromDiode(PANEL0, DIODE0);
            printMsg("%d\r\n", voltage);

		}
	}
}
