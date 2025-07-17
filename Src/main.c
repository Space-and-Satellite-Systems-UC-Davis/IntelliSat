#include <print_scan.h>
#include "platform_init.h"
#include "SunSensors/sun_sensors.h"
#include "DMA/DMA.h"
#include "IMU/ASM330LHH.h"
#include "Buttons/buttons.h"

#define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	0	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>

int main() {
    init_init();
    init_platform(!RUN_TEST);

#if (RUN_TEST==1) && (TEST_ID != 0)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else

    DMA_initializePeripheralConstants();

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}
    printMsg("START\r\n");

    //IRQ

    uint16_t test_buffer[7] = {};

    DMAConfig config;
    config.selection = SELECT_SPI3_RX;
    config.length = 1;
    config.memory_addr = (uint32_t)&test_buffer;
    config.peripheral_addr = (uint32_t) &(SPI3->DR);
    config.circular = true;

    configure_channel(config);


	while (1) {
		if (get_buttonStatus_SW1()) {
//			imu_printAllValues();
//			printMsg("REGISTER: %u\r\n", SPI3->DR);
			printMsg("ARRAY: %u\r\n", test_buffer[0]);

		}
	}

#endif

}
