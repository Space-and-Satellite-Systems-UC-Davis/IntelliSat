#include <stdint.h>

#include "ADCS.h"
#include "adcs_math/matrix.h"

/**
 *  Tagged structs for all data DOWNLINKED from REALOP
 */
typedef enum {
    IDLE_LOG_RECORD = 0,
    START_EXPERIMENT_LOG_RECORD = 0x0000FFFF,
    CONTINUE_EXPERIMENT_LOG_RECORD = 0xFFFFFFFF
} log_record_tag;

typedef struct __attribute__((packed)) {
    uint8_t boot_number;
    uint8_t current_year;
    uint8_t current_month;
    uint8_t current_date;
    uint8_t current_hour;
    uint8_t current_minute;
    uint8_t current_second;
    uint8_t last_action_taken;
    uint8_t last_uplink_year;
    uint8_t last_uplink_month;
    uint8_t last_uplink_date;
    uint8_t last_uplink_hour;
    uint8_t last_uplink_minute;
    uint8_t last_uplink_second;
    uint8_t num_new_experiments;
    float batt_level;
    float batt_7_day_low;
    float batt_7_day_avg;
    float batt_7_day_high;
    float imu_angvel_x;
    float imu_angvel_y;
    float imu_angvel_z;
    float mag_x;
    float mag_y;
    float mag_z;
    uint8_t css_px0;
    uint8_t css_px1;
    uint8_t css_py0;
    uint8_t css_py1;
    uint8_t css_pz0;
    uint8_t css_pz1;
    uint8_t css_nx0;
    uint8_t css_nx1;
    uint8_t css_ny0;
    uint8_t css_ny1;
    uint8_t css_nz0;
    uint8_t css_nz1;
    uint8_t temp_px;
    uint8_t temp_nx;
    uint8_t temp_py;
    uint8_t temp_ny;
    uint8_t error_msg;
    mat3 attitude; // TODO: convert to euler angles
    unsigned int is_in_eclipse : 1;
} log_record_idle;

typedef struct __attribute__((packed)) {
    adcs_mode experiment_type;
    uint8_t alternation_generation;
    uint8_t current_year;
    uint8_t current_month;
    uint8_t current_date;
    uint8_t current_hour;
    uint8_t current_minute;
    uint8_t current_second;
    float battery_level;
    float latitude;
    float longitude;
    float altitude;
    mat3 attitude; // TODO: convert to euler angles
} log_record_experiment_start;

// Sensors chosen based on generation
typedef struct __attribute__((packed)) {
    uint8_t current_second;
    float imu_angvel_x;
    float imu_angvel_y;
    float imu_angvel_z;
    float mag_x;
    float mag_y;
    float mag_z;
    uint8_t css_px;
    uint8_t css_py;
    uint8_t css_pz;
    uint8_t css_nx;
    uint8_t css_ny;
    uint8_t css_nz;
    float hdd_output_0;
    float hdd_output_1;
} log_record_experiment_continue;

/**
 *  Tagged structs for all data UPLINKED to REALOP
 */
typedef enum {
    WHEREAMI_UPLINK_RECORD = 0,
    CSS_CALIBRATION_UPLINK_RECORD = 1,
    MAG_CALIBRATION_UPLINK_RECORD = 2,
    IMU_CALIBRATION_UPLINK_RECORD = 3,
    PID_CALIBRATION_UPLINK_RECORD = 4,
    ENABLE_SENSOR_UPLINK_RECORD = 5
} uplink_record_tag;

typedef struct __attribute__((packed)) {
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    char tle_line_1[70];
    char tle_line_2[70];
} uplink_record_whereami;

typedef struct __attribute__((packed)) {
    float css_offset_px0;
    float css_scalar_px0;
    float css_offset_px1;
    float css_scalar_px1;
    float css_offset_py0;
    float css_scalar_py0;
    float css_offset_py1;
    float css_scalar_py1;
    float css_offset_pz0;
    float css_scalar_pz0;
    float css_offset_pz1;
    float css_scalar_pz1;
    float css_offset_nx0;
    float css_scalar_nx0;
    float css_offset_nx1;
    float css_scalar_nx1;
    float css_offset_ny0;
    float css_scalar_ny0;
    float css_offset_ny1;
    float css_scalar_ny1;
    float css_offset_nz0;
    float css_scalar_nz0;
    float css_offset_nz1;
    float css_scalar_nz1;
} uplink_record_calibration_css;

typedef struct __attribute__((packed)) {
    float mag_offset_x0;
    float mag_scalar_x0;
    float mag_offset_y0;
    float mag_scalar_y0;
    float mag_offset_z0;
    float mag_scalar_z0;
    float mag_offset_x1;
    float mag_scalar_x1;
    float mag_offset_y1;
    float mag_scalar_y1;
    float mag_offset_z1;
    float mag_scalar_z1;
} uplink_record_calibration_mag;

typedef struct __attribute__((packed)) {
    float imu_offset_x0;
    float imu_scalar_x0;
    float imu_offset_y0;
    float imu_scalar_y0;
    float imu_offset_z0;
    float imu_scalar_z0;
    float imu_offset_x1;
    float imu_scalar_x1;
    float imu_offset_y1;
    float imu_scalar_y1;
    float imu_offset_z1;
    float imu_scalar_z1;
} uplink_record_calibration_imu;

typedef struct __attribute__((packed)) {
    float hdd_0_kp;
    float hdd_0_ki;
    float hdd_0_kd;
    float hdd_1_kp;
    float hdd_1_ki;
    float hdd_1_kd;
} uplink_record_calibration_pid;

typedef struct __attribute__((packed)) {
    unsigned int css_px0 : 1;
    unsigned int css_px1 : 1;
    unsigned int css_py0 : 1;
    unsigned int css_py1 : 1;
    unsigned int css_pz0 : 1;
    unsigned int css_pz1 : 1;
    unsigned int css_nx0 : 1;
    unsigned int css_nx1 : 1;
    unsigned int css_ny0 : 1;
    unsigned int css_ny1 : 1;
    unsigned int css_nz0 : 1;
    unsigned int css_nz1 : 1;
    unsigned int imu_0 : 1;
    unsigned int imu_1 : 1;
    unsigned int mag_0 : 1;
    unsigned int mag_1 : 1;
    unsigned int hdd_0 : 1;
    unsigned int hdd_1 : 1;
    unsigned int coil_x : 1;
    unsigned int coil_y : 1;
    unsigned int coil_z : 1;
} uplink_record_sensor_enable;
