#ifndef SCHEDULER_BEACON_BEACON_STATS_H_
#define SCHEDULER_BEACON_BEACON_STATS_H_

#include <stdint.h>

/**
 * Battery low/avg/high over a rolling ~7-day window.
 *
 * Backed by FRAM on FRAM_BEACON_STATS_PAGE.
 *
 * Layout: one bucket per (day_number % BEACON_STATS_NUM_BUCKETS), where
 * day_number is an absolute, monotonically increasing day count (e.g.
 * unix_time / 86400), plus a magic value for validity checking.
 */

#define FRAM_BEACON_STATS_PAGE 5
#define BEACON_STATS_NUM_BUCKETS 7
#define BEACON_STATS_MAGIC_VALUE 0xB0A710C5u

typedef struct {
    float batt_low;
    float batt_avg;
    float batt_high;
} beacon_battery_window_t;

/**
 * Must be called once before beacon_stats_update()/beacon_stats_get().
 * Detects first-ever use (via the magic value) and zeroes all buckets in
 * that case
 */
void beacon_stats_init(void);

/**
 * Records one battery reading into today's bucket.
 *
 * @param new_batt_reading The battery voltage/level just sampled.
 * @param day_number       Absolute day count (e.g. unix_time / 86400).
 */
void beacon_stats_update(float new_batt_reading, uint32_t day_number);

/**
 * Aggregates all 7 buckets into a rolling-window low/avg/high.
 * Buckets with a zero sample count are excluded from the aggregate.
 */
void beacon_stats_get(beacon_battery_window_t *out);

#endif /* SCHEDULER_BEACON_BEACON_STATS_H_ */
