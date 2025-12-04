#include <print_scan.h>
#include "platform_init.h"
#include "SunSensors/sun_sensors.h"
#include "ADC/adc.h"
#include "DMA/DMA.h"
#include "IMU/ASM330LHH.h"
#include "Buttons/buttons.h"

#define RUN_TEST 0
#define TEST_ID  0

#include <TestDefinition.h>

int main(void) {
    init_init();
    init_platform(!RUN_TEST);
    usart_init(USART2, 9600);

#if (RUN_TEST==1) && (TEST_ID != 0)
    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();
#else

    const char* message = "Hello, DMA USART2 TX!\r\n";

    usart_transmitBytesDMA((uint8_t*)message, SELECT_USART2_TX, true);

    while (1) {
        uint32_t count = DMA1_Channel7->CNDTR;
        printMsg("count: %d\r\n", count);
    }
#endif
}
