#include "timers.h"
#include <globals.h>
#include <print_scan.h>
#include <Radio/radio.h>
#include <Timers/timers.h>
#include <LED/led.h>"

void printer() {
	printMsg("1");
}
// void branch_main() {
// 	led_init();
// 	radio_init();
// 	printer_init();

// 	uint64_t initTime = getSysTime();
// 	uint64_t changeInTime = 0;

// 	// infinite loop
// 	uint8_t buffer[255];
// 	for (int i = 0; i < 255; i++) {
// 		buffer[i] = i;
// 	}
// 	while (1) {

// //		char readState = radio_receiveStateRequest();
// //		printMsg("Reading data %c\r\n", readState);

// 		changeInTime = getSysTime() - initTime;
// 		if (changeInTime > 10000) {
// 			radio_sendByteStreamToMemRequest(255, buffer);
// //			printMsg("%d\r\n", radio_readOneByte());
// //			printMsg("%c\r\n", radio_receiveStateRequest());
// 			changeInTime = 0;
// 			initTime = getSysTime();
// 		}


// 	}
// //	logger_initExpTimer();
// //	logger_registerLogFunction(printer);

// //	while (1) {
// //		logger_expTimerOn();
// //		delay_ms(1000);
// //		logger_expTimerOff();
// //		printMsg("\r\n");
// //	}
// }
