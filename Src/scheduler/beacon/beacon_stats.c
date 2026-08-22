#include "beacon_stats.h"

#include <stdbool.h>
#include <string.h>

#include "FRAM/loggers_to_fram.h"

typedef struct __attribute__((packed)) {
    float low;
    float high;
    float sum;
    uint32_t count;
    uint32_t day_number;
} beacon_batt_bucket_t;

typedef struct __attribute__((packed)) {
    uint32_t magic;
    beacon_batt_bucket_t buckets[BEACON_STATS_NUM_BUCKETS];
} beacon_stats_blob_t;

static void load_blob(beacon_stats_blob_t *blob) {
    FRAM_fetch(FRAM_BEACON_STATS_PAGE, sizeof(*blob), blob);
}

static void save_blob(const beacon_stats_blob_t *blob) {
    FRAM_push(FRAM_BEACON_STATS_PAGE, sizeof(*blob), (void *) blob);
}

void beacon_stats_init(void) {
    beacon_stats_blob_t blob;
    load_blob(&blob);

    if (blob.magic == BEACON_STATS_MAGIC_VALUE) {
        return;
    }

    memset(&blob, 0, sizeof(blob));
    blob.magic = BEACON_STATS_MAGIC_VALUE;
    save_blob(&blob);
}

void beacon_stats_update(float new_batt_reading, uint32_t day_number) {
    uint8_t bucket_index = day_number % BEACON_STATS_NUM_BUCKETS;

    beacon_stats_blob_t blob;
    load_blob(&blob);

    beacon_batt_bucket_t *bucket = &blob.buckets[bucket_index];

    if (bucket->day_number != day_number || bucket->count == 0) {
        // Stale (last written on a previous occurrence of this bucket
        // slot) or never-used bucket - start fresh for today.
        bucket->low = new_batt_reading;
        bucket->high = new_batt_reading;
        bucket->sum = new_batt_reading;
        bucket->count = 1;
        bucket->day_number = day_number;
    } else {
        if (new_batt_reading < bucket->low) {
            bucket->low = new_batt_reading;
        }
        if (new_batt_reading > bucket->high) {
            bucket->high = new_batt_reading;
        }
        bucket->sum += new_batt_reading;
        bucket->count++;
    }

    save_blob(&blob);
}

void beacon_stats_get(beacon_battery_window_t *out) {
    beacon_stats_blob_t blob;
    load_blob(&blob);

    float low = 0.0f;
    float high = 0.0f;
    float sum = 0.0f;
    uint32_t count = 0;
    bool seen = false;

    for (uint8_t i = 0; i < BEACON_STATS_NUM_BUCKETS; i++) {
        const beacon_batt_bucket_t *bucket = &blob.buckets[i];

        if (bucket->count == 0) {
            continue;
        }

        if (!seen || bucket->low < low) {
            low = bucket->low;
        }
        if (!seen || bucket->high > high) {
            high = bucket->high;
        }
        sum += bucket->sum;
        count += bucket->count;
        seen = true;
    }

    out->batt_low = low;
    out->batt_high = high;
    out->batt_avg = (count > 0) ? (sum / (float) count) : 0.0f;
}
