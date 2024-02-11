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
	printMsg("Initializing ADC\n");

	adc_enable();
	printMsg("Enabling ADC\n");

	adc_configGPIO();
	printMsg("Configuring GPIO Pin A0 for ADC\n");

	adc_setChannel();
	printMsg("ADC1 channel 5, GPIO Pin A0 set\n Beginning to read values\n");

	while(1){
		printMsg("ADC value: %d", adc_singleConversion());
	}
}

int main() {
	init_platform();
	branch_main();
}
