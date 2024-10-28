/**@file vector.h
 *
 * @brief Interface for 3D vector utility functions.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com) 5/29/2023
 */

#ifndef VECTOR_H
#define VECTOR_H


typedef struct Vector3 {
		double x;
		double y;
		double z;
} vec3; //Use "vec3" as the type.


/**@brief Load the output vector with the given values.
 *
 * @param x,y,z The values to load into the output vector.
 * @param output The vec3* that will hold the resulting vector.
 *
 * @return Void.
 */
void vec_set(double x, double y, double z, vec3 *output);


/**@brief Multiply a vector by a scalar value.
 *
 * Also consider this a method of scalar division!
 *
 * @param scalar The scalar by which to multiply.
 * @param vector The vector being multiplied.
 * @param output The vec3* that will hold the result.
 *
 * @return Void.
 */
void vec_scalar(double scalar, vec3 vector, vec3 *output);


/**@brief Perform a cross product on two vectors.
 *
 * The cross product is NONCOMMUTATIVE!
 * 	left x right != right x left.
 *
 * @param left  The vector being multiplied on the left of the cross.
 * @param right The vector being multiplied on the right of the cross.
 * @param output The vec3* that will hold the result of the cross product.
 *
 * @return Void.
 */
void vec_cross(vec3 left, vec3 right, vec3 *output); 


/**@brief Add two vectors element-by-element.
 *
 * @param first,second The vectors to add.
 * @param output The vec3* that will hold the result of the addition.
 *
 * @return Void.
 */
void vec_add(vec3 first, vec3 second, vec3 *output);


/**@brief Find the bisector of two vectors.
 *
 * In this implementation, the bisector's length is the average of the
 * 	input vectors' length. If you only want information about the direction
 * 	of the bisector, make sure to normalize it separately!
 *
 * @param first,second The vectors to bisect.
 * @param output The vec3* that will hold the resulting bisecting vector.
 *
 * @return Void.
 */
void vec_bisect(vec3 first, vec3 second, vec3 *output);


/**@brief Normalize a vector (stretch length to 1).
 *
 * It is likely that the length of a vector normalized with this
 *  function will not be exactly 1.0. There is imprecision involved!
 *
 * @param vector The vector to normalize.
 * @param output The vec3* that will hold the normalized vector.
 *
 * @return Error code: -1 if mag(vector) == 0
 *                      0 if success.
 */
int vec_norm(vec3 vector, vec3 *output);


/**@brief Calculate the dot product of two vectors.
 *
 * @param first,second The vectors to dot.
 *
 * @return The result of the dot product.
 */
double vec_dot(vec3 first, vec3 second);


/**@brief Calculate the magnitude (length) of a vector.
 *
 * The magnitude of a vector is strictly non-negative.
 *
 * @param vector The vector in question!
 *
 * @return The magnitude of the vector.
 */
double vec_mag(vec3 vector);


#endif//VECTOR_H
