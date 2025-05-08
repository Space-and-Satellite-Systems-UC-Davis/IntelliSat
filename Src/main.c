#include <print_scan.h>
#include "platform_init.h"
#include "SunSensors/sun_sensors.h"
#include "DMA/DMA.h"

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

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}
    printMsg("START\r\n");

	uint16_t test_buffer[255] = {99};


    DMAConfig config;
    config.channel = DMA1_Channel3;
    config.length = 1;
    config.memory_addr = (uint32_t*)&test_buffer;
//    config.peripheral_addr = &ADC3->DR;

    configure_channel(config);
    set_continuous_dma(ADC3, 6);

	while (1) {
//		printMsg("THE VALUE: %f\r\n", sun_sensors_readVoltage(PANEL0, DIODE0));
		printMsg("THE REGISTER: %u\r\n", ADC3->DR);
//		printMsg("THE FUNCTION: %u\r\n", adc_readChannel(ADC3, 6));
		printMsg("THE ARRAY: %u\r\n", test_buffer[0]);


	}

#endif

}
