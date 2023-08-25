#ifndef MOCKUP_FLASH_FLASH_H_
#define MOCKUP_FLASH_FLASH_H_

#include <stdint.h>
#include "../event_logger.h"
#include "../experiment_logger.h"

// EVENT LOGS
// A function to copy event logs in bulk from local storage to mock flash
// "empties" local buffer by setting insertion index to beginning of buffer
uint8_t push_event_logs_to_flash(uint64_t local_buff[]);

// Copie exp logs in bulk from local storage to mock flash
// "empties" local buffer by setting insertion index to beginning of buffer
uint8_t push_local_exp_to_flash(struct LocalExpLogs * local_exp_logs);








#endif