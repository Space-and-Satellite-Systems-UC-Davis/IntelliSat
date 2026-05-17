/**@file quaternion.h
 *
 * @brief Interface for quaternion utility functions.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com) 6/3/2023
 */

#ifndef QUATERNION_H
#define QUATERNION_H

#include "vector.h"

typedef struct Quaternion {
    double scalar;
    vec3 vector;
} quat; // Use "quat" as the type.

/**@brief Load the output quaternion with the given scalar and vector.
 *
 * @param scalar The scalar component to load into the output quat.
 * @param vector The vector component to load into the output quat.
 * @param output The quat* that will hold the resulting quaternion.
 *
 * @return Void.
 */
void quat_set(double scalar, vec3 vector, quat *output);

/**@brief Multiply a quaternion by a scalar value.
 *
 * Also consider this a method of scalar division!
 *
 * @param scalar The scalar by which to multiply.
 * @param quaternion The quaternion being multiplied.
 * @param output The quat* that will hold the result.
 *
 * @return Void.
 */
void quat_scalar(double scalar, quat quaternion, quat *output);

/**@brief Multiply two quaternions.
 *
 * Quaternion multiplication is NONCOMMUTATIVE!
 * 	left*right != right*left.
 *
 * Multiplying two quaternions serves to combine the rotations
 * 	they represent into one. This is the reason it's noncommutative:
 * 	applying left's rotation before right's rotation gives a
 * 	different rotation than vice versa.
 *
 * @param left The quaternion on the left of the multiplication.
 * @param right The quaternion on the right of the multiplication.
 * @param output The quat* that will hold the result.
 *
 * @return Void.
 */
void quat_mult(quat left, quat right, quat *output);

/**@brief Normalize a quaternion (stretch length to 1).
 *
 * @param quaternion The quaternion to normalize.
 * @param output The quat* that will hold the normalized quaternion.
 *
 * @return Error: -1 if mag(quaternion) == 0
 *                 0 if success
 */
int quat_norm(quat quaternion, quat *output);

/**@brief Find the conjugate of a quaternion.
 *
 * The conjugate is the same quaternion with a negative vector value.
 *
 * @param quaternion The quaternion for which to find the conjugate.
 * @param output The quat* that will hold the conjugate quaternion.
 *
 * @return Void.
 */
void quat_conj(quat quaternion, quat *output);

/**@brief Find the inverse of a quaternion.
 *
 * The inverse is the normalized conjugate of a quaternion.
 *
 * @param quaternion The quaternion for which to find the inverse.
 * @param output The quat* that will hold the inverse quaternion.
 *
 * @return Void.
 */
void quat_inverse(quat quaternion, quat *output);

/**@brief Create a quaternion from an axis and rotation angle.
 *
 * @param angle The angle of rotation in radians around the axis.
 * @param vector The vector whose direction is the axis of rotation.
 * @param output The quat* that will hold the resulting quaternion.
 *
 * @return Void.
 */
void quat_from(double angle, vec3 vector, quat *output);

/**@brief Apply the rotation represented by a quaternion to a vector.
 *
 * @param vector The vector to rotate.
 * @param quaternion The quaternion representation of the rotation.
 * @output The vec* that will hold the rotated vector.
 *
 * @return Void.
 */
void quat_rotate_vec(vec3 vector, quat quaternion, vec3 *output);

/**@breif Calculate the magnitude (length) of a quaternion.
 *
 * @param quaternion The quaternion in question!
 *
 * @return The magnitude of the quaternion.
 */
double quat_mag(quat quaternion);

#endif // QUATERNION_H
