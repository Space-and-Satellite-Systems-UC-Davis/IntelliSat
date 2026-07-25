/**@file sun_sensors/sun_sensors.h
 *
 * @brief Functions to determine orientation relative to the sun and eclipse
 * status
 *
 * @author Soren Keck (sorenkeck@gmail.com)
 *
 * @date 10/16/2025
 */

#ifndef SUN_SENSORS_H
#define SUN_SENSORS_H

#define NUM_SUN_SENSORS 6

#include "adcs_math/vector.h"

/**@brief Estimate the Sun direction from coarse photodiode measurements.
 *
 * Selects the illuminated face on each axis (+/- X,Y,Z), forms a signed
 * vector from the three strongest axis readings, and normalizes it.
 *
 * @param sensor_readings Voltage readings from one photodiode on each face
 *                        ordered as {+X, -X, +Y, -Y, +Z, -Z}.
 * @param sun_vec Return-by-reference pointer to normalized Sun vector in the
 *                spacecraft body frame.
 *
 * @return Status code (0 = success, non-zero = insufficient signal / invalid).
 */
int estimate_sun_photodiodes(double photodiode_currVals[NUM_SUN_SENSORS],
                             vec3 *sun_vec);

/**@brief Detect whether the spacecraft is in eclipse using photodiode
 * signals.
 *
 * Computes the total illumination from the selected illuminated face on
 * each axis and compares it to a fixed fraction of the expected maximum
 * photodiode voltage. If the signal is below threshold, the spacecraft is
 * considered to be in eclipse.
 *
 * @param sensor_readings Voltage readings from one photodiode on each face
 *                        ordered as {+X, -X, +Y, -Y, +Z, -Z}.
 *
 * @return 1 if eclipsed, 0 otherwise.
 */
int is_eclipsed_photodiodes(float sensor_readings[NUM_SUN_SENSORS]);

#endif
