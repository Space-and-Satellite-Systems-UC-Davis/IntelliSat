#include "./system_config/platform_init.h"
#include "./tools/print_scan.h"
#include "./system_config/SPI/spi.h"


int main() {
	init_platform();
	while(1) {

		nop(1000000);
	}
}
