#include <print_scan.h>
#include "platform_init.h"
#include "SunSensors/sun_sensors.h"
#include "ADC/adc.h"
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


    printMsg("START\r\n");

    uint16_t test_buffer[1] = {42};

    DMAConfig config;
    config.selection = SELECT_ADC3;
    config.length = 1;
    config.memory_addr = (uint32_t)&test_buffer;
    config.peripheral_addr = (uint32_t) &(ADC3->DR);
    config.peripheral_to_memory = true;
    config.memory_increment = false;
    config.peripheral_increment = false;
    config.mdata_size = 2;
    config.pdata_size = 2;
    config.circular = true;
    config.transfer_interrupt = false;

    configure_channel(config);
    adc_continuous_dma(ADC3, 6);
    dma_enable_channel(SELECT_ADC3);

	while (1) {
		printMsg("REGISTER: %u\r\n", ADC3->DR);

		printMsg("ARRAY: %u\r\n", test_buffer[0]);
	}

#endif

}
