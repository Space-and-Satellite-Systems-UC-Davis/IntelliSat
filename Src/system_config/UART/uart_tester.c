#include "uart.h"
#include "../../globals.h"
#include "../Buttons/buttons.h"
#include "../../tools/print_scan.h"

// NULL & 0 checks
void tester1() {
	printMsg("*** Test 1 ***\r\n");

	char buffer[3] = {' ', ' ', ' '};
	int len = 3;
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

		got += usart_recieveBytes(LPUART1, buffer, len);

	} while (curr_time <= (start_time+10000));
	printMsg("\r\n");

	printMsg("*** Test Results ***\r\n");
	printMsg("\tBytes Received: %d\r\n", got);
	printMsg("\tChars Received: {%c, %c, %c}\r\n", buffer[0], buffer[1], buffer[2]);
	printMsg("********************\r\n");
}

// Tests if 1 character can be received
void tester2() {
	printMsg("*** Test 2 ***\r\n");
}

// Receive but 1 sec delay after press
void tester3() {
	printMsg("*** Test 3 ***\r\n");

}

// Character --> Pause with presses --> Reads
void tester4() {
	printMsg("*** Test 4 ***\r\n");

}

void branch_main() {
	int counter = 0;

	while (1) {
		if (get_buttonStatus_0()) {
			counter++;
		}
		if (get_buttonStatus_1()) {
			switch (counter) {
			case 1:
				tester1();
				break;
			case 2:
				tester2();
				break;
			case 3:
				tester3();
				break;
			case 4:
				tester4();
				break;
			default:
				counter = 0;
				break;
			}
		}
	}
}
