/**@file sun_lookup.c
 *
 * @brief Implementation of wrapper for NREL's SPA.
 *
 * TODO: make sure the calendar dates we get from Intellisat allign
 * 	with SPA's requirements!
 * 	Also check delta_t and atmospheric refraction.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 8/29/2023
 */

#include "sun_lookup.h"
#include "determination/sun_lookup/spa.h"
#include <math.h>

sun_lookup_status sun_lookup(double longitude, double latitude, double altitude,
                             int year, int month, int day, int hour, int minute,
                             double second, vec3 *output) {
    spa_data spa;

    spa.year = year;
    spa.month = month;
    spa.day = day;
    spa.hour = hour;
    spa.minute = minute;
    spa.second = second;
    spa.delta_ut1 = 0.0; // accounts for fractional second errors, not necessary
    spa.delta_t = 70.0; // from: maia.usno.navy.mil/products/deltaT TODO: update
                        // with better approximation
    spa.timezone = 0;
    spa.longitude = longitude;
    spa.latitude = latitude;
    spa.elevation = altitude;
    spa.pressure = 0.0;
    spa.temperature = 0.0;
    spa.slope = 0.0;
    spa.azm_rotation = 0.0;
    spa.atmos_refract =
        0.5667; // TODO: an inaccurate value can cause error of 5 degrees
    spa.function = SPA_ZA;

    int status = spa_calculate(&spa);

    switch (status) {
    case 1: // year < -2000 or year > 6000
    case 2: // month < 1 or month > 12
    case 3: // day < 1 or day > 31
    case 4: // hour < 0 or hour > 24
    case 5: // minute < 0 or minute > 59
            // hour = 24 and minute > 0
    case 6: // second < 0 or second >= 60
            // hour = 24 and second > 0
    case 7:  // fabs(delta_t) > 8000
    case 8:  // fabs(timezone) > 18
    case 17: // delta_ut1 <= -1 or delta_ut1  >= 1

        return SUN_LOOKUP_BAD_DATE;

    case 12: // pressure < 0 or pressure > 5000
    case 13: // temp <= -273 or temp > 6000
    case 14: // fabs(slope) > 360
    case 15: // fabs(azm_rotation) > 360
    case 16: // fabs(atmos_refract) > 5

        return SUN_LOOKUP_BAD_ENVIRONMENT;

    case 9:  // fabs(longitude) > 180
    case 10: // fabs(latitude) > 90
    case 11: // elevation < -6500000

        return SUN_LOOKUP_BAD_LLA;

    case 0:
        break; // Success!
    }

    // Azimuth and Zenith to radians, Zenith to Elevation.
    double az = (spa.azimuth * M_PI / 180.0);
    double el = (-spa.zenith * M_PI / 180.0) + M_PI / 2.0;

    // Azimuth and Elevation to East, North, and Azimuth
    // cartesian coordinates.
    vec_set((float)sin(az) * cos(el), (float)cos(az) * cos(el), (float)sin(el),
            output);

    return SUN_LOOKUP_SUCCESS;
}
