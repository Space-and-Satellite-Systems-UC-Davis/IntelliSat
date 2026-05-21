/**
 * @file triad.c
 *
 * @brief Implementation of the TRIAD algorithm.
 *
 * This file was transcribed from the python version on github.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 7/17/2023
 */

#include "triad.h"
#include "adcs_math/matrix.h"
#include "adcs_math/vector.h"

triad_run_status triad(vec3 bod_sun, vec3 bod_mag, vec3 ref_sun, vec3 ref_mag,
                       mat3 *output) {
    vec3 bod_basis1 = bod_mag;

    if (vec_norm(bod_basis1, &bod_basis1) < 0) {
        return TRIAD_NORM_FAILURE;
    }

    vec3 bod_basis2;
    vec_cross(bod_mag, bod_sun, &bod_basis2);
    if (vec_norm(bod_basis2, &bod_basis2) < 0) {
        return TRIAD_NORM_FAILURE;
    }

    vec3 bod_basis3;
    vec_cross(bod_basis1, bod_basis2, &bod_basis3);
    if (vec_norm(bod_basis3, &bod_basis3) < 0) {
        return TRIAD_NORM_FAILURE;
    }

    mat3 bod_triad;
    mat_set_from_vec(bod_basis1, bod_basis2, bod_basis3, &bod_triad);

    vec3 ref_basis1 = ref_mag;
    if (vec_norm(ref_basis1, &ref_basis1) < 0) {
        return TRIAD_NORM_FAILURE;
    }

    vec3 ref_basis2;
    vec_cross(ref_mag, ref_sun, &ref_basis2);
    if (vec_norm(ref_basis2, &ref_basis2) < 0) {
        return TRIAD_NORM_FAILURE;
    }

    vec3 ref_basis3;
    vec_cross(ref_basis1, ref_basis2, &ref_basis3);
    if (vec_norm(ref_basis3, &ref_basis3) < 0) {
        return TRIAD_NORM_FAILURE;
    }

    mat3 ref_triad;
    mat_set_from_vec(ref_basis1, ref_basis2, ref_basis3, &ref_triad);

    mat_transpose(ref_triad, &ref_triad);

    mat_mult(bod_triad, ref_triad,
             output); // TODO verify the order of multiplication.

    return TRIAD_SUCCESS;
}
