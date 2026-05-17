/**@file matrix.c
 *
 * @brief Implementation of 3x3 matrix utility functions.
 *
 * These functions were designed with the idea in mind that
 * 	a matrix could be passed as both an input and the output
 * 	without issue, even if some pass-by-value arguments are later
 * 	converted to pass-by-reference.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com) 7/13/2023
 */

#include <math.h>

#include "matrix.h"


void mat_set(
	double x1, double x2, double x3,
	double y1, double y2, double y3,
	double z1, double z2, double z3,
	mat3 *output
) {
	(*output).x1 = x1;
	(*output).x2 = x2;
	(*output).x3 = x3;
	(*output).y1 = y1;
	(*output).y2 = y2;
	(*output).y3 = y3;
	(*output).z1 = z1;
	(*output).z2 = z2;
	(*output).z3 = z3;
}

void mat_set_from_vec(
	vec3 first,
	vec3 second,
	vec3 third,
	mat3 *output
) {
	(*output).x1 = first.x;
	(*output).x2 = second.x;
	(*output).x3 = third.x;
	(*output).y1 = first.y;
	(*output).y2 = second.y;
	(*output).y3 = third.y;
	(*output).z1 = first.z;
	(*output).z2 = second.z;
	(*output).z3 = third.z;
}

void mat_transpose(mat3 mat, mat3 *output) {
	mat_set(
		mat.x1, mat.y1, mat.z1,
		mat.x2, mat.y2, mat.z2,
		mat.x3, mat.y3, mat.z3,
		output
	);
}

void mat_add(mat3 left, mat3 right, mat3 *output) {
    mat_set(
        left.x1+right.x1,
        left.x2+right.x2,
        left.x3+right.x3,
        left.y1+right.y1,
        left.y2+right.y2,
        left.y3+right.y3,
        left.z1+right.z1,
        left.z2+right.z2,
        left.z3+right.z3,
        output
    );
}

void mat_sub(mat3 left, mat3 right, mat3 *output) {
    mat_scalar(-1.0, right, &right);
    mat_add(left, right, output);
}

void mat_scalar(double scalar, mat3 mat, mat3 *output) {
	mat_set(
		scalar * mat.x1,
		scalar * mat.x2,
		scalar * mat.x3,
		scalar * mat.y1,
		scalar * mat.y2,
		scalar * mat.y3,
		scalar * mat.z1,
		scalar * mat.z2,
		scalar * mat.z3,
		output
	);
}

void mat_mult(mat3 left, mat3 right, mat3 *output) {
	mat_set(
		left.x1*right.x1 + left.x2*right.y1 + left.x3*right.z1,
		left.x1*right.x2 + left.x2*right.y2 + left.x3*right.z2,
		left.x1*right.x3 + left.x2*right.y3 + left.x3*right.z3,
		left.y1*right.x1 + left.y2*right.y1 + left.y3*right.z1,
		left.y1*right.x2 + left.y2*right.y2 + left.y3*right.z2,
		left.y1*right.x3 + left.y2*right.y3 + left.y3*right.z3,
		left.z1*right.x1 + left.z2*right.y1 + left.z3*right.z1,
		left.z1*right.x2 + left.z2*right.y2 + left.z3*right.z2,
		left.z1*right.x3 + left.z2*right.y3 + left.z3*right.z3,
		output
	);
}

void mat_vec_mult(mat3 left, vec3 right, vec3 *output) {
    vec_set(
        left.x1*right.x + left.x2*right.y + left.x3*right.z,
        left.y1*right.x + left.y2*right.y + left.y3*right.z,
        left.z1*right.x + left.z2*right.y + left.z3*right.z,
        output
    );
}


double mat_det(mat3 mat) {
	return 
		mat.x1 * (mat.y2*mat.z3 - mat.z2*mat.y3) -
		mat.x2 * (mat.y1*mat.z3 - mat.z1*mat.y3) +
		mat.x3 * (mat.y1*mat.z2 - mat.z1*mat.y2);
}

mat3 mat_adj(mat3 mat) {

	mat3 new_mat = (mat3) {
        mat.y2*mat.z3-mat.y3*mat.z2,
        mat.x3*mat.z2-mat.x2*mat.z3,
        mat.x2*mat.y3-mat.x3*mat.y2,
        mat.y3*mat.z1-mat.y1*mat.z3,
        mat.x1*mat.z3-mat.x3*mat.z1,
        mat.x3*mat.y1-mat.x1*mat.y3,
        mat.y1*mat.z2-mat.y2*mat.z1,
        mat.x2*mat.z1-mat.x1*mat.z2,
        mat.x1*mat.y2-mat.x2*mat.y1
    };

	return new_mat;
}

int mat_inverse(mat3 mat, mat3 *output) {
    double determinant = mat_det(mat);

    if (fabs(determinant) < 1e-6) {
        return -1;
    }

	mat3 adj = mat_adj(mat);

    mat_scalar(1/determinant, adj, output);
    return 0;
}





