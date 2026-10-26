#include "INA3221.h"
#include "print_scan.h"

void testFunction_INA3221() {
	ina3221_init(AVERAGE_16, CONVERT_1100, CONVERT_1100, BUS_SINGLE_SHOT);

	while (true) {
		// Get voltage of the battery
		float volt = ina3221_getBusVoltage(
            INA3221_ADDR_BATTERY, BATTERY_BUS_REG,
			INA3221_SCL_PORT, INA3221_SCL_PIN, 
            INA3221_SDA_PORT, INA3221_SDA_PIN);

        printMsg("Battery Bus Voltage: %f\r\n", volt);

		delay_ms(2000);
	}
}
