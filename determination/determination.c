#include "determination/determination.h"
#include "determination/novasc3.1/novas.h"

#include "determination/TRIAD/triad.h"
#include "determination/mag_lookup/mag_lookup.h"
#include "determination/pos_lookup/pos_lookup.h"
#include "determination/sun_lookup/sun_lookup.h"

#include "adcs_math/matrix.h"
#include "adcs_math/sensors.h"
#include "adcs_math/vector.h"
#include "adcs_math/calibration.h"

#include "ADCS.h"
#include "virtual_intellisat.h"

#include <string.h>

typedef struct Determination_Cache {
    double last_update_time;

    // for pos_lookup
    double longitude;
    double latitude;
    double altitude;
    double geocentric_radius;
    double geocentric_latitude;
    pos_lookup_status pos_status;

    vec3 reference_sun;
    sun_lookup_status sun_status;
    vec3 reference_mag;
} determination_cache;

static determination_cache cache;

// TODO: Call getXXX from virtual_intellisat.h for time and mag
// TODO: Write function to get all 12 sun sensor readings
// TODO: Implement logic to turn CSS readings into a vec3

vi_get_css_status get_measured_sun(int generation, vec3 *measured_sun) {

    //px_choice, nx_choice, py_choice, ny_choice, pz_choice, nz_choice;
    vi_sensor sensors[6]; // PX, NX, PY, NY, PZ, NZ
    double currVals[6];   
    static double prevVals[6];  // Not sure how that could be implemented

    for (int i = 0; i < 6; i++){
        sensors[i].component = CSS;
    }

    for (int i = 0; i < 6; i++) {
        sensors[i].choice = 
            sensor_pair_choice(sensors[i], generation) == 1 ? ONE : TWO; 
    }

    for (int i = 0; i < 6; i++){
        if (getCSS(sensors[i], prevVals[i], &(currVals[i])))
            return VI_GET_CSS_FAILURE;
    }

    // Implement logic to combine readings into vector
    *measured_sun = (vec3){0.0, 0.0, 0.0};

    //Clone currVals into prevVals;
    memcpy(prevVals, currVals, sizeof(currVals));

    return VI_GET_CSS_SUCCESS;
}

determination_status determination(mat3 *attitude) {

    int year, month, day, hour, minute, second;

    vec3 measured_mag;
    vec3 measured_sun;
    vec3 mag_prev = (vec3){0, 0, 0};

    vi_sensor magnotometer;
    magnotometer.component = MAG;

    // Get current generation
    int generation = vi_get_determination_generation();

    // Ger magotometer choice
    magnotometer.choice =
        sensor_pair_choice(magnotometer, generation) == 1 ? ONE : TWO;

    // Get current Time
    if (vi_get_epoch(&year, &month, &day, &hour, &minute, &second) ==
        GET_EPOCH_FAILURE)
        return DET_EPOCH_FAILURE;

    // Get current magnetic field reading
    if (getMag(magnotometer, mag_prev, &measured_mag)) {
        return DET_MAG_FAILURE;
    }

    // Get current Sun sensor readings
    if (get_measured_sun(generation, &measured_sun) == VI_GET_CSS_FAILURE) {
        return DET_CSS_FAILURE;
    }

    double UTC =
        julian_date(year, month, day, hour + minute / 60.0 + second / 3600.0);

    int recent_lookup = 0; // false

    if ((UTC - cache.last_update_time) <= julian_date(0, 0, 0, 5 / 60.0)) {
        recent_lookup = 1; // true
    } else {
        cache.last_update_time = UTC;
    }

    int update_IGRF = 0; // false
    char *tle_line1 = NULL;
    char *tle_line2 = NULL;

    vi_get_TLE_status tle_status = vi_get_TLE(tle_line1, tle_line2);

    switch (tle_status) {
    case GET_TLE_FAILURE:
        return DET_NO_TLE;
    case GET_TLE_SUCCESS_OLD:
        break;
    case GET_TLE_SUCCESS_NEW:
        update_IGRF = 1;
        break;
        // update IGRF when we get a new TLE, including the 1st time
    }

    double longitude;
    double latitude;
    double altitude;
    double geocentric_radius;
    double geocentric_latitude;
    pos_lookup_status pos_status;

    if (recent_lookup) {
        longitude = cache.longitude;
        latitude = cache.latitude;
        altitude = cache.altitude;
        geocentric_radius = cache.geocentric_radius;
        geocentric_latitude = cache.geocentric_latitude;
        pos_status = cache.pos_status;
    } else {
        pos_status =
            pos_lookup(tle_line1, tle_line2, UTC, 0.0, &longitude, &latitude,
                       &altitude, &geocentric_radius, &geocentric_latitude);

        cache.pos_status = pos_status;
        cache.longitude = longitude;
        cache.latitude = latitude;
        cache.altitude = altitude;
        cache.geocentric_radius = geocentric_radius;
        cache.geocentric_latitude = geocentric_latitude;
    }

    switch (pos_status) {
    case SGP4_ERROR:
        return DET_POS_LOOKUP_ERROR;
    case TEME2ITRS_ERROR:
        return DET_POS_LOOKUP_ERROR;
    case ITRS2LLA_ERROR:
        return DET_POS_LOOKUP_ERROR;
    case POS_LOOKUP_SUCCESS:
        break;
    }

    vec3 reference_sun;
    sun_lookup_status sun_status;

    if (recent_lookup) {
        reference_sun = cache.reference_sun;
        sun_status = cache.sun_status;
    } else {
        sun_status = sun_lookup(longitude, latitude, altitude, year, month, day,
                                hour, minute, second, &reference_sun);

        cache.sun_status = sun_status;
        cache.reference_sun = reference_sun;
    }

    switch (sun_status) {
    case SUN_LOOKUP_BAD_DATE:
        return DET_POS_LOOKUP_ERROR;
    case SUN_LOOKUP_BAD_ENVIRONMENT:
        return DET_POS_LOOKUP_ERROR;
    case SUN_LOOKUP_BAD_LLA:
        return DET_POS_LOOKUP_ERROR;
    case SUN_LOOKUP_SUCCESS:
        break;
    }

    vec3 reference_mag;

    // Only update IGRF date before recalculating coeffs
    if (update_IGRF) {
        int igrf_time_status =
            igrf_set_date_time(year, month, day, hour, minute, second);

        switch (igrf_time_status) {
        // TODO: use 'default' approximate time if out of bounds?
        case IGRF_SET_DATE_OUT_OF_BOUNDS:
            return DET_IGRF_TIME_ERROR;
        case IGRF_SET_DATE_SUCCESS:
            break;
        }
    }

    if (recent_lookup) {
        reference_mag = cache.reference_mag;
    } else {
        igrf_update(geocentric_latitude, longitude, geocentric_radius,
                    update_IGRF, // recalculate coefficients only on new TLE
                    &reference_mag);
        cache.reference_mag = reference_mag;
    }

    triad_run_status triad_status = triad(
        measured_sun, measured_mag, reference_sun, reference_mag, attitude);

    switch (triad_status) {
    case TRIAD_NORM_FAILURE:
        return DET_TRIAD_ERROR;
    case TRIAD_SUCCESS:
        break;
    }

    // Increment generation on successful execution
    vi_increment_determination_generation();

    return DET_SUCCESS;
}

void get_earth_direction(vec3 *earth_attitude) {

    mat3 attitude;
    determination(&attitude);

    vec3 down;
    vec_set(0, 0, 1, &down);
    mat_vec_mult(attitude, down, earth_attitude);
}

determination_status get_moon_direction(vec3 *moon_attitude) {
    
    int delta_t = 69;

    int year, month, day, hour, minute, second;
    
    // Get current Time
    //vi_get_epoch_status epoch_status = vi_get_epoch(&year, &month, &day, &hour, &minute, &second);
    if (vi_get_epoch(&year, &month, &day, &hour, &minute, &second) ==
        GET_EPOCH_FAILURE)
        return DET_EPOCH_FAILURE; //TODO: fix error handling w/ return type

    // adding delta_t = 69 seconds to UTC to convert to TT
    double TT = julian_date(year, month, day, hour + minute / 60.0 + (second + delta_t) / 3600.0); 
    double UTC = julian_date(year, month, day, hour + minute / 60.0 + second / 3600.0);

    static object moon;
    cat_entry null_star;
    static short int first_time = 1;

    if (first_time) {
        make_cat_entry ("NULL_STAR","   ",0L,0.0,0.0,0.0,0.0,0.0,0.0, &null_star);
        make_object (0,11,"Moon",&null_star, &moon);
        first_time = 0;
    }
    observer location;
    double longitude;
    double latitude;
    double altitude;
    double geocentric_radius;
    double geocentric_latitude;
    pos_lookup_status pos_status;
    char *tle_line1;
    char *tle_line2;

    vi_get_TLE_status tle_status = vi_get_TLE(tle_line1, tle_line2);
    
    switch (tle_status) {
    case GET_TLE_FAILURE:
        return DET_NO_TLE;
    case GET_TLE_SUCCESS_OLD:
        break;
    case GET_TLE_SUCCESS_NEW:
        break;
    }

    pos_status =
            pos_lookup(tle_line1, tle_line2, UTC, 0.0, &longitude, &latitude,
                       &altitude, &geocentric_radius, &geocentric_latitude);

    switch (pos_status) {
    case SGP4_ERROR:
        return DET_POS_LOOKUP_ERROR;
    case TEME2ITRS_ERROR:
        return DET_POS_LOOKUP_ERROR;
    case ITRS2LLA_ERROR:
        return DET_POS_LOOKUP_ERROR;
    case POS_LOOKUP_SUCCESS:
        break;
    }

    double pos[3] = {longitude, latitude, altitude}; // get current satellite position x, y, z
    double vel[3] = {0, 0, 0}; // get current satellite velocity vx, vy, vz
    make_observer_in_space(pos, vel, &location); // check if we can assume that satellite is in space or it could also be on the surface

    sky_pos output;
    // local gcrs coordinate system
    place(TT, &moon, &location, delta_t, 0, 0, &output); // arg6: check if need full accuracy (0) or reduced accuracy (1) is fine

    // convert from geocentric to topocentric
    double x = output.r_hat[0];
    double y = output.r_hat[1];
    double z = output.r_hat[2];
    double u = -x * sin(longitude) + y * cos(longitude);
    double v = (-x * sin(latitude) * cos(longitude)) - (y * sin(latitude) * sin(longitude)) + (z * cos(latitude)); 
    double w = (x * cos(latitude) * cos(longitude)) + (y * cos(latitude) * sin(longitude)) + (z * sin(latitude)); 
    vec3 topo_moon_vec;
    // reverse w and swap u and v
    vec_set(v, u, -w, &topo_moon_vec);

    mat3 attitude;
    determination(&attitude);
    mat_vec_mult(attitude, topo_moon_vec, moon_attitude);
    //convert output (of type sky_pos) to vec3 type
    //moon_attitude = output.r_hat;
}
