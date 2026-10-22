/*
    ~ Implementation of 13th Generation IGRF Model ~

References:
    [1] Davis - Mathematical Modeling of Earth's Magnetic Field (2004)
    [2] Yaguang - Spacecraft Modeling Attitude Determination and Control:
            Quaternion Based Approach (2019)
Note:
    Variable names used in this code follows reference [2].

Author:
    Rishav (2021/12/26)
*/

#ifndef _IGRF_H_
#define _IGRF_H_

#include "adcs_math/vector.h"
#include <inttypes.h>
#include <math.h>

#include "adcs_math/vector.h"

#define PI 3.14159265358979323846
#define PI_2 1.57079632679489661923

#define IGRF_START_YEAR 2020
#define IGRF_END_YEAR 2030
#define IGRF_GEN 13

typedef enum {
    IGRF_SET_DATE_SUCCESS,
    IGRF_SET_DATE_OUT_OF_BOUNDS
} igrf_set_date_status;

/**
 * @brief Update IGRF's date/time state variable.
 *
 * The time only needs to be updated before running
 * igrf_update() with interpolate_flag set to true.
 *
 * @param year,month,day,hour,min,sec The current date/time.
 *
 * @return Status code.
 */
igrf_set_date_status igrf_set_date_time(uint16_t year, uint8_t month,
                                        uint8_t day, uint8_t hour, uint8_t min,
                                        uint8_t sec);

/*
   ~ Compute magnetic field strength in local coordinates, nT ~

Inputs:
    latitude    = Latitude measured in degrees positive from equator, radians
    longitude   = Longitude measured in degrees positive east from Greenwich,
radians radius      = Geocentric radius, km interpolate = Run
compute_gauss_coeffs(), which updates the model based on the date, if true. This
is expensive.

Outputs:
        *B_ned = return-by-reference pointer to an adcs_math vec3 struct.
             After igrf_update runs, B_ned will hold the the magnetic
             field vector's North, East and Up components in its X, Y,
             and Z components, respectively (nT).
*/
void igrf_update(float latitude, float longitude, float radius,
                 int interpolate_flag, vec3 *B_ned);

float igrf_get_horizontal_intensity();
float igrf_get_inclination();
float igrf_get_declination();
float igrf_get_norm();

#endif //_IGRF_H_
