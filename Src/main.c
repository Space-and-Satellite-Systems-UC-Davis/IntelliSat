#include "./system_config/platform_init.h"
#include "./tools/print_scan.h"
#include "./system_config/QSPI/qspi.h"

#define LEN 3

void qspi_test(uint8_t* data) {
	qspi_set_command(1, 1, 0, 0, 0, 1);
	qspi_send_command(0x9F, 0x00, LEN, data, true);
}

button = false;
int main() {
	init_platform();
	qspi_config(23);

	// uint8_t transmission[] = { 0xA1, 0xA2, 0xA3, 0xA4 };
	uint8_t reception[LEN];
	for (int i = 0; i < LEN; i++) {
		reception[i] = 0xFF;
	}
	while(1) {
		if (button == true) {
			qspi_test(reception);
			for (int i = 0; i < LEN; i++) {
				printMsg("%X ", reception[i]);
			}
			printMsg("\n");
			button = false;
		}
		nop(core_MHz * 1000);
	}
}
