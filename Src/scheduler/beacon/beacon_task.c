#include "beacon_task.h"

#include <stdbool.h>
#include <stddef.h>

#include "FreeRTOS.h"
#include "task.h"

#include "beacon_record.h"
#include "beacon_stats.h"
#include "logging_records.h"

#include "PWRMON/INA226.h"
#include "IMU/ASM330LHH.h"
#include "MAG/QMC5883L.h"
#include "SunSensors/sun_sensors.h"
#include "RTC/rtc.h"
#include "ADCS.h"
#include "Radio/Radio_intercom.h"

// TODO: tune retry/backoff values
#define RADIO_PUSH_RETRY_COUNT 3
#define RADIO_PUSH_RETRY_DELAY_MS 500

bool beacon_task_cycle(log_record_idle *out_record) {
    beacon_sensor_snapshot_t snap = {0};
    snap.batt_voltage = pwrmon_getBusVoltage(BATMON_GPIO, BATMON_SCL, BATMON_SDA);

    snap.imu_gx = imu_readGyro_X();
    snap.imu_gy = imu_readGyro_Y();
    snap.imu_gz = imu_readGyro_Z();

    // TODO: Fallback for MAG1(?)
    mag_select(MAG0);
    snap.mag_x = mag_read_X();
    snap.mag_y = mag_read_Y();
    snap.mag_z = mag_read_Z();

    // Panel-to-face mapping per Src/peripherals/SunSensors/sun_sensors.h
    // (PANEL0..5 = +X/-X/+Y/-Y/+Z/-Z). sun_sensors_readVoltage/readTemp
    // return float; log_record_idle's css_*/temp_* fields are uint8_t,
    // so these are direct truncating casts - no scaling/quantization
    // scheme has been defined yet.
    snap.css_px0 = (uint8_t) sun_sensors_readVoltage(PANEL0, DIODE0);
    snap.css_px1 = (uint8_t) sun_sensors_readVoltage(PANEL0, DIODE1);
    snap.css_nx0 = (uint8_t) sun_sensors_readVoltage(PANEL1, DIODE0);
    snap.css_nx1 = (uint8_t) sun_sensors_readVoltage(PANEL1, DIODE1);
    snap.css_py0 = (uint8_t) sun_sensors_readVoltage(PANEL2, DIODE0);
    snap.css_py1 = (uint8_t) sun_sensors_readVoltage(PANEL2, DIODE1);
    snap.css_ny0 = (uint8_t) sun_sensors_readVoltage(PANEL3, DIODE0);
    snap.css_ny1 = (uint8_t) sun_sensors_readVoltage(PANEL3, DIODE1);
    snap.css_pz0 = (uint8_t) sun_sensors_readVoltage(PANEL4, DIODE0);
    snap.css_pz1 = (uint8_t) sun_sensors_readVoltage(PANEL4, DIODE1);
    snap.css_nz0 = (uint8_t) sun_sensors_readVoltage(PANEL5, DIODE0);
    snap.css_nz1 = (uint8_t) sun_sensors_readVoltage(PANEL5, DIODE1);

    // TODO: Fallback if TEMP0 read fails?
    snap.temp_px = (uint8_t) sun_sensors_readTemp(PANEL0, TEMP0);
    snap.temp_nx = (uint8_t) sun_sensors_readTemp(PANEL1, TEMP0);
    snap.temp_py = (uint8_t) sun_sensors_readTemp(PANEL2, TEMP0);
    snap.temp_ny = (uint8_t) sun_sensors_readTemp(PANEL3, TEMP0);

    rtc_getTime(&snap.hour, &snap.minute, &snap.second);
    rtc_getCalendar(&snap.year, &snap.month, &snap.date, NULL);

    snap.det_status = ADCS_get_attitude(&snap.attitude);
    snap.is_in_eclipse = ADCS_is_in_eclipse();

    uint32_t day_number = rtc_getUnixTime() / 86400;
    beacon_stats_update(snap.batt_voltage, day_number);

    beacon_battery_window_t batt_window;
    beacon_stats_get(&batt_window);

    log_record_idle record;
    build_beacon_record(&record, &snap, &batt_window);

    bool sent = false;
    for (int attempt = 0; attempt < RADIO_PUSH_RETRY_COUNT && !sent; attempt++) {
        sent = radio_push((uint8_t *) &record, sizeof(record));
        if (!sent) {
            vTaskDelay(pdMS_TO_TICKS(RADIO_PUSH_RETRY_DELAY_MS));
        }
    }
    // TODO: What if all retries fail?

    if (out_record != NULL) {
        *out_record = record;
    }

    return sent;
}

void beacon_task(void *pvParameters) {
    (void) pvParameters;

    beacon_stats_init();

    for (;;) {
        beacon_task_cycle(NULL);
        vTaskDelay(pdMS_TO_TICKS(BEACON_PERIOD_MS));
    }
}
