#ifndef SCHEDULER_BEACON_BEACON_STATS_H_
#define SCHEDULER_BEACON_BEACON_STATS_H_

#include <stdint.h>

/**
 * Battery low/avg/high over a rolling ~7-day window.
 *
 * Backed by FRAM on FRAM_BEACON_STATS_PAGE.
 *
 * Layout: one bucket per day-of-week (Monday=1..Sunday=7, matching
 * rtc_getCalendar()'s `day` output), plus a magic value for validity checking.
 *
 * Each cycle, if the persisted date for today's weekday bucket
 * doesn't match today's date, that bucket is stale (last written on a
 * previous occurrence of this weekday) and gets reset before
 * accumulating.
 *
 * KNOWN LIMITATION: if the satellite is powered off across one or more
 * full weeks, a bucket can go stale for longer than 7 days before its
 * weekday recurs, at which point it's reset on the next update - there's
 * no cross-check against "how many days ago" beyond weekday+date-of-month
 * matching.
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
 * @param weekday           1-7 (Monday..Sunday), from rtc_getCalendar().
 * @param date_of_month     1-31, from rtc_getCalendar().
 */
void beacon_stats_update(float new_batt_reading, uint8_t weekday, uint8_t date_of_month);

/**
 * Aggregates all 7 buckets into a rolling-window low/avg/high.
 * Buckets with a zero sample count are excluded from the aggregate.
 */
void beacon_stats_get(beacon_battery_window_t *out);

#endif /* SCHEDULER_BEACON_BEACON_STATS_H_ */
