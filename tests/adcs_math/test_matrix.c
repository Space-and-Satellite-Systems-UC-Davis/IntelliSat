#include "tests/framework.h"
#include "adcs_math/matrix.h"

#define TOL 1e-9

// Verify all 9 elements of a matrix match expected values.
static void assert_mat(mat3 m,
    double x1, double x2, double x3,
    double y1, double y2, double y3,
    double z1, double z2, double z3)
{
    TEST_ASSERT_DBL(m.x1, x1, TOL);
    TEST_ASSERT_DBL(m.x2, x2, TOL);
    TEST_ASSERT_DBL(m.x3, x3, TOL);
    TEST_ASSERT_DBL(m.y1, y1, TOL);
    TEST_ASSERT_DBL(m.y2, y2, TOL);
    TEST_ASSERT_DBL(m.y3, y3, TOL);
    TEST_ASSERT_DBL(m.z1, z1, TOL);
    TEST_ASSERT_DBL(m.z2, z2, TOL);
    TEST_ASSERT_DBL(m.z3, z3, TOL);
}

static void test_mat_set(void) {
    mat3 m;
    mat_set(1,2,3, 4,5,6, 7,8,9, &m);
    assert_mat(m, 1,2,3, 4,5,6, 7,8,9);
}

static void test_mat_set_from_vec(void) {
    // columns: first=(1,4,7), second=(2,5,8), third=(3,6,9)
    // → matrix rows: [1,2,3], [4,5,6], [7,8,9]
    vec3 a, b, c;
    vec_set(1.0, 4.0, 7.0, &a);
    vec_set(2.0, 5.0, 8.0, &b);
    vec_set(3.0, 6.0, 9.0, &c);
    mat3 m;
    mat_set_from_vec(a, b, c, &m);
    assert_mat(m, 1,2,3, 4,5,6, 7,8,9);
}

static void test_mat_transpose(void) {
    mat3 m, t, tt;
    mat_set(1,2,3, 4,5,6, 7,8,9, &m);

    mat_transpose(m, &t);
    assert_mat(t, 1,4,7, 2,5,8, 3,6,9);

    // double-transpose recovers original
    mat_transpose(t, &tt);
    assert_mat(tt, 1,2,3, 4,5,6, 7,8,9);
}

static void test_mat_add(void) {
    mat3 identity, zero_mat, result;
    mat_set(1,0,0, 0,1,0, 0,0,1, &identity);
    mat_set(0,0,0, 0,0,0, 0,0,0, &zero_mat);

    mat_add(identity, zero_mat, &result);
    assert_mat(result, 1,0,0, 0,1,0, 0,0,1);

    mat_add(identity, identity, &result);
    assert_mat(result, 2,0,0, 0,2,0, 0,0,2);
}

static void test_mat_sub(void) {
    mat3 m, result;
    mat_set(1,2,3, 4,5,6, 7,8,9, &m);

    mat_sub(m, m, &result);
    assert_mat(result, 0,0,0, 0,0,0, 0,0,0);
}

static void test_mat_scalar(void) {
    mat3 identity, result;
    mat_set(1,0,0, 0,1,0, 0,0,1, &identity);

    mat_scalar(2.0, identity, &result);
    assert_mat(result, 2,0,0, 0,2,0, 0,0,2);

    mat_scalar(0.0, identity, &result);
    assert_mat(result, 0,0,0, 0,0,0, 0,0,0);
}

static void test_mat_mult(void) {
    mat3 identity, m, result;
    mat_set(1,0,0, 0,1,0, 0,0,1, &identity);
    mat_set(1,2,3, 4,5,6, 7,8,9, &m);

    mat_mult(identity, m, &result);
    assert_mat(result, 1,2,3, 4,5,6, 7,8,9);

    mat_mult(m, identity, &result);
    assert_mat(result, 1,2,3, 4,5,6, 7,8,9);
}

static void test_mat_vec_mult(void) {
    mat3 identity;
    mat_set(1,0,0, 0,1,0, 0,0,1, &identity);

    vec3 v, result;
    vec_set(1.0, 2.0, 3.0, &v);
    mat_vec_mult(identity, v, &result);
    TEST_ASSERT_DBL(result.x, 1.0, TOL);
    TEST_ASSERT_DBL(result.y, 2.0, TOL);
    TEST_ASSERT_DBL(result.z, 3.0, TOL);

    // diag(1,2,3) * (1,1,1) = (1,2,3)
    mat3 scale;
    mat_set(1,0,0, 0,2,0, 0,0,3, &scale);
    vec3 ones;
    vec_set(1.0, 1.0, 1.0, &ones);
    mat_vec_mult(scale, ones, &result);
    TEST_ASSERT_DBL(result.x, 1.0, TOL);
    TEST_ASSERT_DBL(result.y, 2.0, TOL);
    TEST_ASSERT_DBL(result.z, 3.0, TOL);
}

static void test_mat_det(void) {
    mat3 m;

    mat_set(1,0,0, 0,1,0, 0,0,1, &m);
    TEST_ASSERT_DBL(mat_det(m), 1.0, TOL);

    mat_set(0,0,0, 0,0,0, 0,0,0, &m);
    TEST_ASSERT_DBL(mat_det(m), 0.0, TOL);

    // det([[1,2,3],[0,1,4],[5,6,0]]) = 1*(0-24) - 2*(0-20) + 3*(0-5) = 1
    mat_set(1,2,3, 0,1,4, 5,6,0, &m);
    TEST_ASSERT_DBL(mat_det(m), 1.0, TOL);
}

static void test_mat_inverse(void) {
    mat3 m, inv, product;

    // inverse of identity is identity
    mat_set(1,0,0, 0,1,0, 0,0,1, &m);
    int ret = mat_inverse(m, &inv);
    TEST_ASSERT(ret == 0);
    assert_mat(inv, 1,0,0, 0,1,0, 0,0,1);

    // M * M^-1 = I  (M = [[2,1,0],[1,3,0],[0,0,1]], det = 5)
    mat_set(2,1,0, 1,3,0, 0,0,1, &m);
    ret = mat_inverse(m, &inv);
    TEST_ASSERT(ret == 0);
    mat_mult(m, inv, &product);
    assert_mat(product, 1,0,0, 0,1,0, 0,0,1);

    // singular matrix (det = 0) returns -1
    mat_set(1,2,3, 4,5,6, 7,8,9, &m);
    ret = mat_inverse(m, &inv);
    TEST_ASSERT(ret == -1);
}

int main(void) {
    TEST_SUITE("matrix");
    RUN_TEST(test_mat_set);
    RUN_TEST(test_mat_set_from_vec);
    RUN_TEST(test_mat_transpose);
    RUN_TEST(test_mat_add);
    RUN_TEST(test_mat_sub);
    RUN_TEST(test_mat_scalar);
    RUN_TEST(test_mat_mult);
    RUN_TEST(test_mat_vec_mult);
    RUN_TEST(test_mat_det);
    RUN_TEST(test_mat_inverse);
    TEST_RESULTS();
}
