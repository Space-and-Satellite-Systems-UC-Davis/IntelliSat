#include <globals.h>

//Many loops just stall until is_done is triggered, so they use this as placeholder
void empty_function() {
    return;
}

bool is_time_out(uint64_t start_time, uint64_t timeout_ms) {
    return (WILL_LOOPS_TIMEOUT == true) && ((start_time + timeout_ms) < getSysTime());
}

void while_timeout(void (*do_work)(), bool (*is_done)(), uint64_t timeout_ms) {
    uint64_t start_time = getSysTime(); //time in ms

    while (is_done() == false && is_time_out(start_time, timeout_ms) == false) {
        do_work();
    }
}

void empty_while_timeout(bool (*is_done)(), uint64_t timeout_ms) {
    while_timeout(empty_function, is_done, timeout_ms);
}

//Multiple files use these functions so declaring here to avoid declaring multiple times
bool is_GPIOA_ready() { return (GPIOA->OTYPER == 0xFFFFFFFF); }
bool is_GPIOB_ready() { return (GPIOB->OTYPER == 0xFFFFFFFF); }
bool is_GPIOC_ready() { return (GPIOC->OTYPER == 0xFFFFFFFF); }
bool is_GPIOD_ready() { return (GPIOD->OTYPER == 0xFFFFFFFF); }
bool is_GPIOE_ready() { return (GPIOE->OTYPER == 0xFFFFFFFF); }
bool is_GPIOF_ready() { return (GPIOF->OTYPER == 0xFFFFFFFF); }
bool is_GPIOG_ready() { return (GPIOG->OTYPER == 0xFFFFFFFF); }