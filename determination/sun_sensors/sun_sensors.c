/**@file sun_sensors/sun_sensors.c
 *
 * @brief Implementation of eclipse detection and sun vector determination
 *
 * @author Soren Keck (sorenkeck@gmail.com)
 * @date 10/16/2025
 */
#define MAX_EXPECTED_PHOTODIODE_VOLTAGE 1.62
#define NUM_SUN_SENSORS 6
#define pX 0
#define nX 1
#define pY 2
#define nY 3
#define pZ 4
#define nZ 5

#include "adcs_math/vector.h"
#include "determination/sun_lookup/sun_lookup.h"
#include <math.h>

int estimate_sun_photodiodes(double photodiode_currVals[NUM_SUN_SENSORS],
                             vec3 *sun_vec)
{
    double x_reading = 0.0, y_reading = 0.0, z_reading = 0.0;
    double sensor_readings[NUM_SUN_SENSORS];
    for (int i = 0; i < NUM_SUN_SENSORS; i++)
        sensor_readings[i] =
            (photodiode_currVals[i] > 0.0) ? photodiode_currVals[i] : 0.0;
    if (sensor_readings[pX] > sensor_readings[nX]) {
        x_reading = sensor_readings[pX];
    } else {
        x_reading = -sensor_readings[nX];
    }
    if (sensor_readings[pY] > sensor_readings[nY]) {
        y_reading = sensor_readings[pY];
    } else {
        y_reading = -sensor_readings[nY];
    }
    if (sensor_readings[pZ] > sensor_readings[nZ]) {
        z_reading = sensor_readings[pZ];
    } else {
        z_reading = -sensor_readings[nZ];
    }
    vec3 sun_vec_raw = (vec3){ x_reading, y_reading, z_reading };
    double mag2 =
        x_reading * x_reading + y_reading * y_reading + z_reading * z_reading;
    if (mag2 < 1e-12) {
        sun_vec->x = sun_vec->y = sun_vec->z = 0.0;
        return -1;
    }
    vec_norm(sun_vec_raw, sun_vec);
    return 0;
}

int is_eclipsed_photodiodes(double sensor_readings[NUM_SUN_SENSORS])
{
    double x_reading = 0.0, y_reading = 0.0, z_reading = 0.0;
    if (sensor_readings[pX] > sensor_readings[nX]) {
        x_reading = sensor_readings[pX];
    } else {
        x_reading = -sensor_readings[nX];
    }
    if (sensor_readings[pY] > sensor_readings[nY]) {
        y_reading = sensor_readings[pY];
    } else {
        y_reading = -sensor_readings[nY];
    }
    if (sensor_readings[pZ] > sensor_readings[nZ]) {
        z_reading = sensor_readings[pZ];
    } else {
        z_reading = -sensor_readings[nZ];
    }
    double total = fabs(x_reading) + fabs(y_reading) + fabs(z_reading);
    if (total < 0.1 * MAX_EXPECTED_PHOTODIODE_VOLTAGE) {
        return 1;
    } else {
        return 0;
    }
}