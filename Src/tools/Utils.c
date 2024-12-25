// #include <globals.h>

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