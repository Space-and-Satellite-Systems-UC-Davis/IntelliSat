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
    usart_init(USART1, 9600);
    
    while (1) {
        dma_test();
    }
}
