#ifndef SCHEDULER_BEACON_BEACON_TASK_H_
#define SCHEDULER_BEACON_BEACON_TASK_H_

#include <stdbool.h>
#include <stdint.h>

// TODO tune
#define BEACON_PERIOD_MS 60000

extern volatile uint8_t last_task_id;

void beacon_task_init(void);

#endif /* SCHEDULER_BEACON_BEACON_TASK_H_ */
