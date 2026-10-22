/**@file sun_lookup.h
 *
 * @brief Interface to wrapper of NREL's Sun Position Algorithm.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 8/29/2023
 */

#ifndef SUN_LOOKUP_H
#define SUN_LOOKUP_H

#include "adcs_math/vector.h"


typedef enum {
    SUN_LOOKUP_SUCCESS,
    SUN_LOOKUP_BAD_DATE,
    SUN_LOOKUP_BAD_ENVIRONMENT,
    SUN_LOOKUP_BAD_LLA
} sun_lookup_status;

/**@brief Look up the position of the sun as a vector.
 *
 * The vector is defined in cartesian space with <East, North, Zenith>
 * 	as the positive <x, y, z> axes. This is not a standard coordinate
 * 	system but it aligns closely with the output of IGRF mag lookup.
 *
 * Longitude is defined as degrees East of Greenwich (-180, 180).
 * Latitude is defined as degrees North of the equator (-90, 90).
 * Altitude is defined as meters above sea level.
 *
 * SPA unfortunately doesn't accept a Julian Day value as time input,
 * 	but giving the calendar date should work fine.
 *
 * @param longitude,latitude,altitude The position of the satellite.
 * @param year,month,day,hour,minute,second The date and time.
 * @param output The vec3* that will hold the output.
 *
 * @return sun_lookup_status Error code.
 */
sun_lookup_status
sun_lookup(
	double longitude,
	double latitude,
	double altitude,
	int year,
	int month,
	int day,
	int hour,
	int minute,
	double second,
	vec3 *output
);


#endif//SUN_LOOKUP_H





