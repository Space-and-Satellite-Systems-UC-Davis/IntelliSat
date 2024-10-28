#include "timers.h"
#include <globals.h>
#include <print_scan.h>

void testFunction_LogTimer_Callback() {
	logger_initExpTimer();
//	logger_registerLogFunction(printer);

	while (1) {
		logger_expTimerOn();
		delay_ms(1000);
		logger_expTimerOff();
		printMsg("\r\n");
	}
}
