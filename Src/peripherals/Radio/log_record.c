/*********************************************************
 * log_record.c 
 * 
 * Driver for creating/filling out status packets for the PFC to send to RADIO
 * 
 * Author: Benjamin Morgan (2026)
 * 
 * Log:
 *  20 May 2026 - Initial
 * 
 ********************************************************/

#include "log_record.h"


#define NOT_IMPLEMENTED 0
/**
 * 
 * Fills the log_record_idle struct for sending to the Radio.
 * takes in a pointer to the previous log
 * 
 */
log_record_idle fill_log_idle() {

    log_record_idle log;
    uint8_t dummyUInt8_t;

    // WIP
    log.boot_number = (uint8_t)(RTC->BKP0R);


    // DONE
    // uint8_t current_year;
    // uint8_t current_month;
    // uint8_t current_date;
    // uint8_t current_hour;
    // uint8_t current_minute;
    // uint8_t current_second;
    rtc_getTime(&log.current_hour, &log.current_minute, &log.current_second);
    rtc_getCalendar(&log.current_year, &log.current_month, &log.current_date, &dummyUInt8_t);


    // WIP
    log.last_action_taken = NOT_IMPLEMENTED;
    log.last_uplink_year = NOT_IMPLEMENTED;
    log.last_uplink_month = NOT_IMPLEMENTED;
    log.last_uplink_date = NOT_IMPLEMENTED;
    log.last_uplink_hour = NOT_IMPLEMENTED;
    log.last_uplink_minute = NOT_IMPLEMENTED;
    log.last_uplink_second = NOT_IMPLEMENTED;
    log.num_new_experiments = NOT_IMPLEMENTED;


    // WIP
    // float batt_level;
    log.batt_level = pwrmon_getBusVoltage(BATMON_GPIO, BATMON_SCL, BATMON_SDA);
    log.batt_7_day_low = NOT_IMPLEMENTED;
    log.batt_7_day_avg = NOT_IMPLEMENTED;
    log.batt_7_day_high = NOT_IMPLEMENTED;



    // float imu_angvel_x;
    log.imu_angvel_x = imu_readGyro_X();
    // float imu_angvel_y;
    log.imu_angvel_y = imu_readGyro_Y();
    // float imu_angvel_z;
    log.imu_angvel_z = imu_readGyro_Z();

    // float mag_x;
    log.mag_x = mag_read_X();
    // float mag_y;
    log.mag_y = mag_read_Y();
    // float mag_z;
    log.mag_z = mag_read_Z();


    // WIP
    float reading = 0;
    reading = sun_sensors_readVoltage(PANEL0, DIODE0);
    log.css_px0 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL0, DIODE1);
    log.css_px1 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL1, DIODE0);
    log.css_py0 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL1, DIODE1);
    log.css_py1 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL2, DIODE0);
    log.css_pz0 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL2, DIODE1);
    log.css_pz1 = reading >= 0 ? (uint8_t)(reading * 255) : 0;


    // WIP
    reading = sun_sensors_readVoltage(PANEL3, DIODE0);
    log.css_nx0 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL3, DIODE1);
    log.css_nx1 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL4, DIODE0);
    log.css_ny0 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL4, DIODE1);
    log.css_ny1 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL5, DIODE0);
    log.css_nz0 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    reading = sun_sensors_readVoltage(PANEL5, DIODE1);
    log.css_nz1 = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    // WIP
    // uint8_t temp_px;
    reading = sun_sensors_readTemp(PANEL0,TEMP0);
    log.temp_px = reading >= 0 ? (uint8_t)(reading * 255) : 0;
    // uint8_t temp_nx;
    reading = sun_sensors_readTemp(PANEL3,TEMP0);
    log.temp_nx = reading >= 0 ? (uint8_t)(reading * 255) : 0;
    // uint8_t temp_py;
    reading = sun_sensors_readTemp(PANEL1,TEMP0);
    log.temp_py = reading >= 0 ? (uint8_t)(reading * 255) : 0;
    // uint8_t temp_ny;
    reading = sun_sensors_readTemp(PANEL4,TEMP0);
    log.temp_ny = reading >= 0 ? (uint8_t)(reading * 255) : 0;

    // WIP
    log.error_msg = NOT_IMPLEMENTED;


    log.attitude = (mat3){0}; // TODO: convert to euler angles


    log.is_in_eclipse = NOT_IMPLEMENTED;
    return log;
}
