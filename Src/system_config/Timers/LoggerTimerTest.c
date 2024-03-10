#include "timers.h"
#include <globals.h>
#include <print_scan.h>


void delay_ms(uint32_t ms) {
	uint64_t curr_time = getSysTime();
	while (getSysTime() < (curr_time + ms));
}

void branch_main() {
	logger_initExpTimer();

	while (1) {
		logger_expTimerOn();
		delay_ms(1000);
		logger_expTimerOff();
		printMsg("\r\n");
	}
}
