/**
 * @file triad.h
 *
 * @brief Interface to the TRIAD algorithm.
 *
 * TRIAD calculates REALOP's relative attitude as a rotation matrix
 *  that represents the rotation from Earth-fixed coordinates to
 *  satellite-fixed coordinates.
 * It does so by comparing a "triad" of vectors from each frame
 *  and working backwards to figure out their relationship.
 * These triads are constructed from the direction of Earth's
 *  magnetic field, the direction of the sun, and their cross
 *  product in each reference frame.
 * On REALOP, the Earth-frame vectors are calculated/looked up,
 *  while the satellite-frame vectors are measured by sensors.
 * The attitude matrix can later be converted into a quaternion
 *  or Euler angles.
 *
 * @see:
 *   https://en.wikipedia.org/wiki/Triad_method
 * 	 https://arc.aiaa.org/doi/10.2514/3.2555
 * 	 And check the SSS google drive!
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 7/17/2023
 */

#ifndef TRIAD_H
#define TRIAD_H

#include "adcs_math/matrix.h"
#include "adcs_math/vector.h"

typedef enum { TRIAD_SUCCESS, TRIAD_NORM_FAILURE } triad_run_status;

/**@brief Calculate the satellite's attitude given the reference
 *  and sensor sun and mag vectors.
 *
 * All parameter vectors must be [X, Y, Z] but needn't be normalized.
 *
 * @param bod_sun The sun vector in the satellite's reference frame.
 * @param bod_mag The mag vector in the satellite's reference frame.
 * @param ref_sun The sun vector in the Earth's reference frame.
 * @param ref_mag The mag vector in the Earth's reference frame.
 * @param output The mat3* that will hold the satellite's attitude
 *  as a direction cosine matrix.
 *
 * @return triad_run_status Error code.
 */
triad_run_status triad(vec3 bod_sun, vec3 bod_mag, vec3 ref_sun, vec3 ref_mag,
                       mat3 *output);

#endif // TRIAD_H
