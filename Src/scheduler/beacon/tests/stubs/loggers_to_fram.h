#ifndef SCHEDULER_BEACON_TESTS_STUBS_LOGGERS_TO_FRAM_H_
#define SCHEDULER_BEACON_TESTS_STUBS_LOGGERS_TO_FRAM_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FRAM_PAGE_SIZE 256

bool FRAM_fetch(uint16_t page, size_t data_size, void *data);
bool FRAM_push(uint16_t page, size_t data_size, void *data);

/* Test-only helper: clears all simulated FRAM pages. */
void fram_stub_reset(void);

#endif /* SCHEDULER_BEACON_TESTS_STUBS_LOGGERS_TO_FRAM_H_ */
