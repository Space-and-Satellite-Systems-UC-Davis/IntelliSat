#include "qspi.h"
#include "../../globals.h"


#define DATA_LEN 3

void branch_main() {
	qspi_config(23);

	uint8_t buffer[DATA_LEN];

	while(1) {
		if (button1) {
			button1 = false;

			// Reading JPEC ID
			qspi_set_command(
					QSPI_INDIRECT_READ,
					QSPI_1_WIRE,
					QSPI_UNUSED,
					QSPI_UNUSED,
					0x00,
					QSPI_1_WIRE,
					true
			);
			qspi_send_command(
					0x9F,
					0x00,
					DATA_LEN,
					buffer,
					true,
					QSPI_TIMEOUT_PERIOD
			);
		}
	}
}
