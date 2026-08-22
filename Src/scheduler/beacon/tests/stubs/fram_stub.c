#include "FRAM/loggers_to_fram.h"

#include <string.h>

// A handful of simulated 256-byte pages is plenty for host tests -
// beacon currently only uses FRAM_BEACON_STATS_PAGE (5).
#define FRAM_STUB_NUM_PAGES 16

static uint8_t pages[FRAM_STUB_NUM_PAGES][FRAM_PAGE_SIZE] = {0};

bool FRAM_fetch(uint16_t page, size_t data_size, void *data) {
    if (data_size > FRAM_PAGE_SIZE || page >= FRAM_STUB_NUM_PAGES) {
        return false;
    }
    memcpy(data, pages[page], data_size);
    return true;
}

bool FRAM_push(uint16_t page, size_t data_size, void *data) {
    if (data_size > FRAM_PAGE_SIZE || page >= FRAM_STUB_NUM_PAGES) {
        return false;
    }
    memcpy(pages[page], data, data_size);
    return true;
}

void fram_stub_reset(void) {
    memset(pages, 0, sizeof(pages));
}
