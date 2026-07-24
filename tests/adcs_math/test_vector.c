#include "tests/framework.h"
#include "adcs_math/vector.h"

#define TOL 1e-9

static void test_vec_set_equal(void) {
    vec3 v;
    vec_set(1.0, 2.0, 3.0, &v);
    TEST_ASSERT_DBL(v.x, 1.0, TOL);
    TEST_ASSERT_DBL(v.y, 2.0, TOL);
    TEST_ASSERT_DBL(v.z, 3.0, TOL);

    vec3 a, b;
    vec_set(1.0, 2.0, 3.0, &a);
    vec_set(1.0, 2.0, 3.0, &b);
    TEST_ASSERT(vec_equal(a, b));

    vec_set(1.0, 2.0, 4.0, &b);
    TEST_ASSERT(!vec_equal(a, b));
}

static void test_vec_add(void) {
    vec3 a, b, result;
    vec_set(1.0, 2.0, 3.0, &a);
    vec_set(4.0, 5.0, 6.0, &b);

    vec_add(a, b, &result);
    TEST_ASSERT_DBL(result.x, 5.0, TOL);
    TEST_ASSERT_DBL(result.y, 7.0, TOL);
    TEST_ASSERT_DBL(result.z, 9.0, TOL);

    // commutativity: a+b == b+a
    vec3 result2;
    vec_add(b, a, &result2);
    TEST_ASSERT(vec_equal(result, result2));
}

static void test_vec_sub(void) {
    vec3 a, b, result;
    vec_set(5.0, 7.0, 9.0, &a);
    vec_set(4.0, 5.0, 6.0, &b);

    vec_sub(a, b, &result);
    TEST_ASSERT_DBL(result.x, 1.0, TOL);
    TEST_ASSERT_DBL(result.y, 2.0, TOL);
    TEST_ASSERT_DBL(result.z, 3.0, TOL);

    // v - v = zero vector
    vec_sub(a, a, &result);
    TEST_ASSERT_DBL(result.x, 0.0, TOL);
    TEST_ASSERT_DBL(result.y, 0.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);
}

static void test_vec_scalar(void) {
    vec3 v, result;
    vec_set(1.0, 2.0, 3.0, &v);

    vec_scalar(2.0, v, &result);
    TEST_ASSERT_DBL(result.x, 2.0, TOL);
    TEST_ASSERT_DBL(result.y, 4.0, TOL);
    TEST_ASSERT_DBL(result.z, 6.0, TOL);

    vec_scalar(0.0, v, &result);
    TEST_ASSERT_DBL(result.x, 0.0, TOL);
    TEST_ASSERT_DBL(result.y, 0.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);

    vec_scalar(1.0, v, &result);
    TEST_ASSERT(vec_equal(result, v));
}

static void test_vec_dot(void) {
    vec3 x, y, z;
    vec_set(1.0, 0.0, 0.0, &x);
    vec_set(0.0, 1.0, 0.0, &y);
    vec_set(0.0, 0.0, 1.0, &z);

    // orthogonal basis vectors dot to 0
    TEST_ASSERT_DBL(vec_dot(x, y), 0.0, TOL);
    TEST_ASSERT_DBL(vec_dot(y, z), 0.0, TOL);
    TEST_ASSERT_DBL(vec_dot(x, z), 0.0, TOL);

    // unit vector with itself = 1
    TEST_ASSERT_DBL(vec_dot(x, x), 1.0, TOL);

    // (1,2,3)·(4,5,6) = 4+10+18 = 32
    vec3 a, b;
    vec_set(1.0, 2.0, 3.0, &a);
    vec_set(4.0, 5.0, 6.0, &b);
    TEST_ASSERT_DBL(vec_dot(a, b), 32.0, TOL);
}

static void test_vec_cross(void) {
    vec3 x, y, z, result;
    vec_set(1.0, 0.0, 0.0, &x);
    vec_set(0.0, 1.0, 0.0, &y);
    vec_set(0.0, 0.0, 1.0, &z);

    // X×Y = Z
    vec_cross(x, y, &result);
    TEST_ASSERT_DBL(result.x, 0.0, TOL);
    TEST_ASSERT_DBL(result.y, 0.0, TOL);
    TEST_ASSERT_DBL(result.z, 1.0, TOL);

    // Y×Z = X
    vec_cross(y, z, &result);
    TEST_ASSERT_DBL(result.x, 1.0, TOL);
    TEST_ASSERT_DBL(result.y, 0.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);

    // Z×X = Y
    vec_cross(z, x, &result);
    TEST_ASSERT_DBL(result.x, 0.0, TOL);
    TEST_ASSERT_DBL(result.y, 1.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);

    // anti-commutativity: A×B = -(B×A)
    vec3 xy, yx;
    vec_cross(x, y, &xy);
    vec_cross(y, x, &yx);
    TEST_ASSERT_DBL(xy.x, -yx.x, TOL);
    TEST_ASSERT_DBL(xy.y, -yx.y, TOL);
    TEST_ASSERT_DBL(xy.z, -yx.z, TOL);
}

static void test_vec_mag(void) {
    vec3 v;

    vec_set(1.0, 0.0, 0.0, &v);
    TEST_ASSERT_DBL(vec_mag(v), 1.0, TOL);

    vec_set(0.0, 0.0, 0.0, &v);
    TEST_ASSERT_DBL(vec_mag(v), 0.0, TOL);

    // 3-4-5 right triangle
    vec_set(3.0, 4.0, 0.0, &v);
    TEST_ASSERT_DBL(vec_mag(v), 5.0, TOL);

    vec_set(1.0, 1.0, 1.0, &v);
    TEST_ASSERT_DBL(vec_mag(v), sqrt(3.0), TOL);
}

static void test_vec_norm(void) {
    vec3 v, result;

    // norm of axis-aligned vector gives unit vector along that axis
    vec_set(5.0, 0.0, 0.0, &v);
    int ret = vec_norm(v, &result);
    TEST_ASSERT(ret == 0);
    TEST_ASSERT_DBL(result.x, 1.0, TOL);
    TEST_ASSERT_DBL(result.y, 0.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);

    // result magnitude is 1 for any non-zero input
    vec_set(3.0, 4.0, 0.0, &v);
    ret = vec_norm(v, &result);
    TEST_ASSERT(ret == 0);
    TEST_ASSERT_DBL(vec_mag(result), 1.0, TOL);

    // zero vector returns error code -1
    vec_set(0.0, 0.0, 0.0, &v);
    ret = vec_norm(v, &result);
    TEST_ASSERT(ret == -1);
}

static void test_vec_bisect(void) {
    vec3 a, b, result;

    // bisect of equal vectors → same direction, same magnitude
    vec_set(2.0, 0.0, 0.0, &a);
    vec_bisect(a, a, &result);
    TEST_ASSERT_DBL(result.x, 2.0, TOL);
    TEST_ASSERT_DBL(result.y, 0.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);

    // bisect((1,0,0),(0,1,0)) = 0.5*(1+0, 0+1, 0) = (0.5, 0.5, 0)
    vec_set(1.0, 0.0, 0.0, &a);
    vec_set(0.0, 1.0, 0.0, &b);
    vec_bisect(a, b, &result);
    TEST_ASSERT_DBL(result.x, 0.5, TOL);
    TEST_ASSERT_DBL(result.y, 0.5, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);
}

int main(void) {
    TEST_SUITE("vector");
    RUN_TEST(test_vec_set_equal);
    RUN_TEST(test_vec_add);
    RUN_TEST(test_vec_sub);
    RUN_TEST(test_vec_scalar);
    RUN_TEST(test_vec_dot);
    RUN_TEST(test_vec_cross);
    RUN_TEST(test_vec_mag);
    RUN_TEST(test_vec_norm);
    RUN_TEST(test_vec_bisect);
    TEST_RESULTS();
}
