#include <stdint.h>
#include <string.h>
#include <MB85RS256B.h>

#define FRAM_PAGE_SIZE 256

void FRAM_fetchData(uint16_t page, void *data, size_t data_size)
{
    uint8_t MISO[FRAM_PAGE_SIZE];

    if (!flash_readPage(page, MISO))
    {
        memset(data, 0xFF, data_size);
        return;
    }

    memcpy(data, MISO, data_size);
}

uint8_t FRAM_pushData(uint16_t page, void *data, size_t data_size)
{
    uint8_t buffer[FRAM_PAGE_SIZE];

    if (data_size > FRAM_PAGE_SIZE)
    {
        return 1;
    }

    memcpy(buffer, data, data_size);
    FRAM_writePage(page, buffer);

    return 0;
}
