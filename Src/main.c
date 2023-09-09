#include "./system_config/platform_init.h"
#include "./tools/print_scan.h"
#include "./system_config/SPI/spi.h"

#define LEN 4

int main() {
	init_platform();

	uint8_t transmission[] = { 0xA1, 0xA2, 0xA3, 0xA4 };
	uint8_t reception[LEN];

	while(1) {
		spi_start_communication(SPI2_CS);
		spi2_transmit_recieve(transmission, reception, LEN);
		spi_stop_communication(SPI2_CS);
		for (int i = 0; i < LEN; i++) {
			printMsg("%h", reception[i]);
		}
		nop(core_MHz * 1000);
	}
}
