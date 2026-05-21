/**@file pos_lookup.c
 *
 * @brief Implementation of interface to SGP4 orbit propagator.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 9/4/2023
 */

#include "pos_lookup.h"
#include "determination/novasc3.1/novas.h"
#include "determination/pos_lookup/ECEF_to_geodetic.h"
#include "determination/pos_lookup/sgp4/src/c/TLE.h"

pos_lookup_status pos_lookup(char *tle_line1, char *tle_line2, double UTC1,
                             double UTC2, double *longitude, double *latitude,
                             double *altitude, double *geocentric_radius,
                             double *geocentric_latitude) {
    TLE tle;

    double realop_position_TEME[3];
    double realop_velocity_TEME[3];
    double realop_position_ITRS[3];

    // Get TEME position vector (kilometers) from TLE.

    parseLines(&tle, tle_line1, tle_line2);
    getRVForDate(&tle,
                 (UTC1 + UTC2 - 2440587.5) *
                     86400000.0, // check sgp4/src/c/TLE.h
                 realop_position_TEME, realop_velocity_TEME);

    if (tle.sgp4Error != 0)
        return SGP4_ERROR;

    // Convert from kilometers to meters.

    realop_position_TEME[0] *= 1000.0;
    realop_position_TEME[1] *= 1000.0;
    realop_position_TEME[2] *= 1000.0;

    (*geocentric_radius) =
        sqrt(realop_position_TEME[0] * realop_position_TEME[0] +
             realop_position_TEME[1] * realop_position_TEME[1] +
             realop_position_TEME[2] * realop_position_TEME[2]);

    // Convert TEME vector to ITRS vector (both in meters).

    double MJD = (UTC1 + UTC2) - 2400000.5; // for x_p,y_p approximation

    short int status_cel2ter =
        cel2ter(UTC1, UTC2,
                70, // TODO: update, probably pass as argument from ADCS main
                1,  // Equinox-based so we can ask for equinox of date
                1,  // Reduced accuracy
                1,  // Asking for equinox of date (TEME instead of GCRS)
                0.000005 * MJD - 0.1183, // x_p (see footnote)
                0.00001 * MJD - 0.0177,  // y_p (    ....    )
                realop_position_TEME,    // Transform from TEME
                realop_position_ITRS     // to ITRS
        );

    if (status_cel2ter != 0)
        return TEME2ITRS_ERROR;

    (*geocentric_latitude) =
        asin(realop_position_ITRS[2] / (*geocentric_radius));

    // Convert ITRS vector (meters) to geodetic coordinates (lon,lat,alt).

    int status_etg =
        wgs84EcefToGeo(realop_position_ITRS[0], realop_position_ITRS[1],
                       realop_position_ITRS[2], longitude, latitude, altitude);

    if (status_etg != 0) {
        return ITRS2LLA_ERROR;
    }

    return POS_LOOKUP_SUCCESS;
}

/*x_p and y_p approximations are from
https://www.researchgate.net/publication/262933324
_Earth_Orientation_Parameter_and_Space_Weather_Data
_for_Flight_Operations/
*/
