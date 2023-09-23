#include "./system_config/platform_init.h"
#include "./tools/print_scan.h"


button = false;
int main() {
	init_platform();

	while(1) {
		if (button == true) {
			//

			button = false;
		}
		nop(core_MHz * 1000);
	}
}
