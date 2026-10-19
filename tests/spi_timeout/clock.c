#include <stdint.h>

/* The emulator supplies a monotonic millisecond clock at this function. */
uint64_t getSysTime(void) {
    return 0;
}
