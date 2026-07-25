#include <stdint.h>
#include <string.h>
#include "loggers_to_fram.h"


bool FRAM_fetch(uint16_t page, size_t data_size, void *data) {
    if (data_size > FRAM_PAGE_SIZE) {
        return false;
    }

    uint8_t page_buffer[FRAM_PAGE_SIZE];

    if (!FRAM_readPage(page, page_buffer)) {
        memset(data, 0xFF, data_size);
        return false;
    }
    memcpy(data, page_buffer, data_size);

    return true;
}

bool FRAM_push(uint16_t page, size_t data_size, void *data) {
    if (data_size > FRAM_PAGE_SIZE) {
        return false;
    }

    uint8_t page_buffer[FRAM_PAGE_SIZE];
    memcpy(page_buffer, data, data_size);

    if (!FRAM_writePage(page, page_buffer)) {
        return false;
    }

    return true;
}

