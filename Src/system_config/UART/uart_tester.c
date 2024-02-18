// 2/18/2023 (Darsh)

#include "uart.h"
#include "../../globals.h"
#include "../../tools/print_scan.h"

// Tests if n character can be received
void tester1() {
	printMsg("*** Test 1 ***\r\n");

	#define LEN 3

	char buffer[LEN] = {' ', ' ', ' '};
	int got = 0;

	uint64_t start_time = getSysTime();
	uint64_t curr_time, old_time = start_time+1000;
	do {
		curr_time = getSysTime();

		if (old_time == curr_time) { // print the timer on the screen
			uint16_t diff = (uint16_t)(curr_time-start_time) / 1000;
			printMsg("%d ", diff);
			old_time = curr_time + 1000;
		}

		if (got >= LEN) {
			break;
		} else {
			got += usart_recieveBytes(LPUART1, &(buffer[got]), (LEN-got));
		}

	} while (curr_time <= (start_time+10000));
	printMsg("\r\n");

	printMsg("*** Test Results ***\r\n");
	printMsg("\tBytes Received: %d\r\n", got);
	printMsg("\tChars Received: {%c, %c, %c}\r\n", buffer[0], buffer[1], buffer[2]);

	printMsg("********************\r\n");
}

// Receiver Overflow Tests?
void tester2() {
	printMsg("*** Test 2 ***\r\n");
}
