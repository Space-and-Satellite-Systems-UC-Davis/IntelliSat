#include <print_scan.h>
#include "platform_init.h"

#ifdef BUILD_TEST
	void branch_main();
#endif

int main() {
    init_init();
    
    //TODO: use RTC first_time flag.
    //if (first_time) {
    //  init_first_time()
    //}

	init_platform();

#ifdef BUILD_TEST
	branch_main();
#else
	while (1) {
		continue;
	}
#endif

}
