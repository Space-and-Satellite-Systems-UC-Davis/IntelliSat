/**@file vector.c
 *
 * @brief Implementation of 3D vector utility functions.
 *
 * These functions were designed with the idea in mind that
 * 	a vector could be passed as both an input and the output
 * 	without issue, even if some pass-by-value arguments are later
 * 	converted to pass-by-reference.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com) 5/29/2023
 */

#include "vector.h"
#include <math.h>

void vec_set(double x, double y, double z, vec3 *output) {
    (*output).x = x;
    (*output).y = y;
    (*output).z = z;
}

void vec_scalar(double scalar, vec3 vector, vec3 *output) {
    vec_set(scalar * (vector.x), scalar * (vector.y), scalar * (vector.z),
            output);
}

void vec_cross(vec3 left, vec3 right, vec3 *output) {
    vec_set((left.y * right.z - left.z * right.y),
            (left.z * right.x - left.x * right.z),
            (left.x * right.y - left.y * right.x), output);
}

void vec_add(vec3 left, vec3 right, vec3 *output) {
    vec_set(left.x + right.x, left.y + right.y, left.z + right.z, output);
}

void vec_sub(vec3 left, vec3 right, vec3 *output) {
    vec_set(left.x - right.x, left.y - right.y, left.z - right.z, output);
}

void vec_bisect(vec3 left, vec3 right, vec3 *output) {
    vec3 sum_vector;
    vec_add(left, right, &sum_vector);

    vec_scalar(0.5, sum_vector, output);
}

int vec_norm(vec3 vector, vec3 *output) {
    double mag = vec_mag(vector);

    if (fabs(mag) < 1e-6) {
        return -1;
    }

    vec_scalar(1 / mag, vector, output);

    return 0;
}

double vec_dot(vec3 first, vec3 second) {
    return (first.x) * (second.x) + (first.y) * (second.y) +
           (first.z) * (second.z);
}

double vec_mag(vec3 vector) { return sqrt(vec_dot(vector, vector)); }
