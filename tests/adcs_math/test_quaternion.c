#include "tests/framework.h"
#include "adcs_math/quaternion.h"

#define TOL  1e-9
#define PI   3.14159265358979323846

static void assert_quat(quat q, double s, double vx, double vy, double vz) {
    TEST_ASSERT_DBL(q.scalar,   s,  TOL);
    TEST_ASSERT_DBL(q.vector.x, vx, TOL);
    TEST_ASSERT_DBL(q.vector.y, vy, TOL);
    TEST_ASSERT_DBL(q.vector.z, vz, TOL);
}

static void test_quat_set(void) {
    vec3 v;
    vec_set(1.0, 2.0, 3.0, &v);
    quat q;
    quat_set(4.0, v, &q);
    assert_quat(q, 4.0, 1.0, 2.0, 3.0);
}

static void test_quat_mag(void) {
    vec3 v;
    quat q;

    // identity quaternion has magnitude 1
    vec_set(0.0, 0.0, 0.0, &v);
    quat_set(1.0, v, &q);
    TEST_ASSERT_DBL(quat_mag(q), 1.0, TOL);

    // (3, {4,0,0}): sqrt(9+16) = 5
    vec_set(4.0, 0.0, 0.0, &v);
    quat_set(3.0, v, &q);
    TEST_ASSERT_DBL(quat_mag(q), 5.0, TOL);
}

static void test_quat_scalar(void) {
    vec3 v;
    vec_set(1.0, 2.0, 3.0, &v);
    quat q, result;
    quat_set(4.0, v, &q);

    quat_scalar(2.0, q, &result);
    assert_quat(result, 8.0, 2.0, 4.0, 6.0);

    quat_scalar(0.0, q, &result);
    assert_quat(result, 0.0, 0.0, 0.0, 0.0);

    quat_scalar(1.0, q, &result);
    assert_quat(result, 4.0, 1.0, 2.0, 3.0);
}

static void test_quat_norm(void) {
    vec3 v;
    quat q, result;

    // already unit magnitude — normalization is a no-op
    vec_set(0.0, 0.0, 0.0, &v);
    quat_set(1.0, v, &q);
    int ret = quat_norm(q, &result);
    TEST_ASSERT(ret == 0);
    TEST_ASSERT_DBL(quat_mag(result), 1.0, TOL);

    // (3, {4,0,0}): magnitude 5 → normalized scalar = 0.6, vx = 0.8
    vec_set(4.0, 0.0, 0.0, &v);
    quat_set(3.0, v, &q);
    ret = quat_norm(q, &result);
    TEST_ASSERT(ret == 0);
    TEST_ASSERT_DBL(quat_mag(result), 1.0, TOL);
    TEST_ASSERT_DBL(result.scalar,   0.6, TOL);
    TEST_ASSERT_DBL(result.vector.x, 0.8, TOL);

    // zero quaternion returns -1
    vec_set(0.0, 0.0, 0.0, &v);
    quat_set(0.0, v, &q);
    ret = quat_norm(q, &result);
    TEST_ASSERT(ret == -1);
}

static void test_quat_conj(void) {
    vec3 v;
    vec_set(1.0, 2.0, 3.0, &v);
    quat q, result;
    quat_set(4.0, v, &q);

    quat_conj(q, &result);
    // scalar unchanged, vector negated
    TEST_ASSERT_DBL(result.scalar,    4.0, TOL);
    TEST_ASSERT_DBL(result.vector.x, -1.0, TOL);
    TEST_ASSERT_DBL(result.vector.y, -2.0, TOL);
    TEST_ASSERT_DBL(result.vector.z, -3.0, TOL);
}

static void test_quat_mult_identity(void) {
    vec3 zero;
    vec_set(0.0, 0.0, 0.0, &zero);
    quat identity, q, result;
    quat_set(1.0, zero, &identity);

    vec3 v;
    vec_set(1.0, 2.0, 3.0, &v);
    quat_set(4.0, v, &q);

    // identity * q = q
    quat_mult(identity, q, &result);
    assert_quat(result, 4.0, 1.0, 2.0, 3.0);

    // q * identity = q
    quat_mult(q, identity, &result);
    assert_quat(result, 4.0, 1.0, 2.0, 3.0);
}

static void test_quat_inverse(void) {
    // q * q^-1 = identity quaternion (1, {0,0,0})
    vec3 axis;
    vec_set(0.0, 0.0, 1.0, &axis);
    quat q, q_inv, product;
    quat_from(PI / 3.0, axis, &q);
    quat_inverse(q, &q_inv);
    quat_mult(q, q_inv, &product);

    TEST_ASSERT_DBL(product.scalar,   1.0, TOL);
    TEST_ASSERT_DBL(product.vector.x, 0.0, TOL);
    TEST_ASSERT_DBL(product.vector.y, 0.0, TOL);
    TEST_ASSERT_DBL(product.vector.z, 0.0, TOL);
}

static void test_quat_from(void) {
    // 90° rotation around Z: q = (cos 45°, {0, 0, sin 45°})
    vec3 z_axis;
    vec_set(0.0, 0.0, 1.0, &z_axis);
    quat q;
    quat_from(PI / 2.0, z_axis, &q);

    double half = sqrt(2.0) / 2.0;
    TEST_ASSERT_DBL(q.scalar,   half, TOL);
    TEST_ASSERT_DBL(q.vector.x, 0.0,  TOL);
    TEST_ASSERT_DBL(q.vector.y, 0.0,  TOL);
    TEST_ASSERT_DBL(q.vector.z, half, TOL);
}

static void test_quat_rotate_vec(void) {
    vec3 z_axis, x_axis, result;
    vec_set(0.0, 0.0, 1.0, &z_axis);
    vec_set(1.0, 0.0, 0.0, &x_axis);
    quat q;

    // rotate X 90° around Z → should give Y
    quat_from(PI / 2.0, z_axis, &q);
    quat_rotate_vec(x_axis, q, &result);
    TEST_ASSERT_DBL(result.x, 0.0, TOL);
    TEST_ASSERT_DBL(result.y, 1.0, TOL);
    TEST_ASSERT_DBL(result.z, 0.0, TOL);

    // identity quaternion leaves vector unchanged
    vec3 zero;
    vec_set(0.0, 0.0, 0.0, &zero);
    quat identity;
    quat_set(1.0, zero, &identity);
    vec3 v;
    vec_set(3.0, 1.0, 4.0, &v);
    quat_rotate_vec(v, identity, &result);
    TEST_ASSERT_DBL(result.x, 3.0, TOL);
    TEST_ASSERT_DBL(result.y, 1.0, TOL);
    TEST_ASSERT_DBL(result.z, 4.0, TOL);
}

int main(void) {
    TEST_SUITE("quaternion");
    RUN_TEST(test_quat_set);
    RUN_TEST(test_quat_mag);
    RUN_TEST(test_quat_scalar);
    RUN_TEST(test_quat_norm);
    RUN_TEST(test_quat_conj);
    RUN_TEST(test_quat_mult_identity);
    RUN_TEST(test_quat_inverse);
    RUN_TEST(test_quat_from);
    RUN_TEST(test_quat_rotate_vec);
    TEST_RESULTS();
}
