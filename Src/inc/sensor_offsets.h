#ifndef INTELLISAT_SENSOR_OFFSETS_H
#define INTELLISAT_SENSOR_OFFSETS_H
/* Existing FRAM ADCS bit positions; shared so FRAM/RTC include order is safe. */
typedef enum SensorOffset_enum {
    Sun0=0, Sun1, Sun2, Sun3, Sun4, Sun5, Sun6,
    Sun7, Sun8, Sun9, Sun10, Sun11,
    Coil0, Coil1, Coil2, Imu0, Imu1, Mag0, Mag1, Hdd0, Hdd1
} SensorOffset;
#endif
