/**@file ECEF_to_geodetic.h
 *
 * @brief Converts vector-based ECEF coordinates into lat, lon, and alt.
 *
 * This is an interface to the software provided in this paper:
 *      Karl Osen. Accurate Conversion of Earth-Fixed Earth-Centered
 *      Coordinates to Geodetic Coordinates. [Research Report]
 *      Norwegian University of Science and Technology. 2017.
 *      hal-01704943v2f
 *      https://hal.science/hal-01704943v2/document
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 9/3/2023
 */

#ifndef ECEF_TO_GEODETIC_H
#define ECEF_TO_GEODETIC_H

/**@brief Converts cartesian ECEF coords into lon, lat, and alt.
 *
 * This conversion uses the WGS84 model of the Earth's ellipsoid.
 *
 * @param x,y,z Cartesian components of the ECEF vector to convert.
 * @param longitude,latitude,altitude Pointers to geodetic output.
 *
 * ECEF units are meters.
 * Geodetic units are degrees.
 *
 * @return Error code: zero means success, anything else failure.
 */
int wgs84EcefToGeo(double x, double y, double z, double *longitude,
                   double *latitude, double *altitude);

#endif // ECEF_TO_GEODETIC_H
