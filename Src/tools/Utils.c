#include <globals.h>
#include <print_scan.h>

//Checks if the time hath come
//Also used for manually implementing timeout loops
bool is_time_out(uint64_t start_time, uint64_t timeout_ms) {
	uint64_t current_time = getSysTime();
	bool is_out = (WILL_LOOPS_TIMEOUT == true) && ((start_time + timeout_ms) <= current_time);
    return is_out;
}

//Continue repeating the do_work function as long as should_continue returns true. End if times out
void while_timeout(void (*do_work)(), bool (*should_continue)(), uint64_t timeout_ms) {
    uint64_t start_time = getSysTime(); //time in ms

    while (should_continue() == true) {
    	if (is_time_out(start_time, timeout_ms) == true) {
    		break;
    	}
        do_work();
    }
}



//Many loops just stall until should_continue is false, so this is used
void empty_function() {
    return;
}

//Continue waiting as long as given function returns true. End if times out
void wait_with_timeout(bool (*continue_waiting)(), uint64_t timeout_ms) {
    while_timeout(empty_function, continue_waiting, timeout_ms);
}


//Multiple files use these functions so declaring here to avoid declaring multiple times
bool is_GPIOA_not_ready() { return (GPIOA->OTYPER == 0xFFFFFFFF); }
bool is_GPIOB_not_ready() { return (GPIOB->OTYPER == 0xFFFFFFFF); }
bool is_GPIOC_not_ready() { return (GPIOC->OTYPER == 0xFFFFFFFF); }
bool is_GPIOD_not_ready() { return (GPIOD->OTYPER == 0xFFFFFFFF); }
bool is_GPIOE_not_ready() { return (GPIOE->OTYPER == 0xFFFFFFFF); }
bool is_GPIOF_not_ready() { return (GPIOF->OTYPER == 0xFFFFFFFF); }
bool is_GPIOG_not_ready() { return (GPIOG->OTYPER == 0xFFFFFFFF); }


//Assumes there is empty space
void prepend(void* arr, size_t len, void* element) {
	for (int i = 0; i < len-1; i++) {
		arr[i+1] = arr[i];
		if (i == 0) arr[0] = element;
	}
}
