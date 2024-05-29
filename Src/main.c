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
	printMsg("Configuring GPIO Pin A2 for ADC\r\n");

	adc_setConstantGPIOValue();

	//Right now it sets it pin A2, which is photodiode/sunsensor 0 (SOL-0)
	//Plug in TP2 to the bottommost wire on the connector and TP1 to the one above it
	adc_setChannel();
	printMsg("ADC1 channel 7, GPIO Pin A2 set\r\nBeginning to read values\r\n");

	while(1){
		uint16_t adcVal = adc_singleConversion();
		printMsg("ADC value: %d", adcVal);
		printMsg(". MilliVolt value: ");
		adc_printMilliVolt(adc_adcToVolt2(adcVal));
		printMsg("\r\n");
		nop(1000000);
	}
}

int main() {
	init_platform();
	branch_main();
}
