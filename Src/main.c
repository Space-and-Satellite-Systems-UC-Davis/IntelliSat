/*
 * RTC testing
 */

#include "./system_config/platform_init.h"
#include "./tools/print_scan.h"
#include "./system_config/RTC/rtc.h"


int main() {
	init_platform();
	uint8_t hour, minute, sec;
	while(1) {
		// Read and Display the RTC Time
		rtc_get_time(&hour, &minute, &sec);
		printMsg("Current Time : %d : %d : %d\n", hour, minute, sec);

		nop(1000000);
	}
}
