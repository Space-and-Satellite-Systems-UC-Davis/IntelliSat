#include "platform_init.h"
#include "tools/print_scan.h"
#include "system_config/ADC/adc.h"
/*
 * For now this won't be implemented on the main branch
 * Therefore, the main branch cannot be built.
 * Each development / feature branch has it's own implementation
 * which is used for testing specific features.
 */
void branch_main(){
	//test ADC
	adc_init();
	printMsg("Initializing ADC\r\n");

	adc_enable();
	printMsg("Enabling ADC\r\n");

	adc_configGPIO();
	printMsg("Configuring GPIO Pin C0 for ADC\r\n");

	adc_setConstantGPIOValue();

	adc_setChannel();
	printMsg("ADC1 channel 1, GPIO Pin C0 set\r\nBeginning to read values\r\n");

	while(1){
		printMsg("ADC value: %d \r\n", adc_singleConversion());

		nop(1000000);
	}
}

int main() {
	init_platform();
	branch_main();
}
