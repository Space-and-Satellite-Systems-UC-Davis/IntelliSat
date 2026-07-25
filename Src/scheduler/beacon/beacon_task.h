#ifndef SCHEDULER_BEACON_BEACON_TASK_H_
#define SCHEDULER_BEACON_BEACON_TASK_H_

#include <stdbool.h>

#include "logging_records.h"

// TODO tune
#define BEACON_PERIOD_MS 60000

/**
 * periodic beaconing task.
 *
 * Each period: reads sensors, updates in-RAM running battery stats,
 * builds a log_record_idle (see beacon_record.h), and transmits it via
 * radio_push().
 */
void beacon_task(void *pvParameters);

/**
 * Runs exactly one beacon cycle (sensor reads, stats update, record
 * build, radio_push with retry). 
 *
 * @param out_record If non-NULL, the built record is copied here so a
 *        caller (e.g. a test) can inspect what was sent.
 * @return Whether radio_push succeeded (after retries).
 */
bool beacon_task_cycle(log_record_idle *out_record);

#endif /* SCHEDULER_BEACON_BEACON_TASK_H_ */
