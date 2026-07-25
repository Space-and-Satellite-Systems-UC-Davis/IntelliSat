#include "IMU_tests.h"
#include "../Assertions/Ass.h"
#include <math.h>
#include <float.h>

void test_initial_state()
{
    float max_gyro_fluctuation = 0.0f;
    int rate = 5;    // number of checks per second
    int seconds = 1; // how long to gather data (secs)

    float max_gyro_x = FLT_MIN;
    float max_gyro_y = FLT_MIN;
    float max_gyro_z = FLT_MIN;

    float min_gyro_x = FLT_MAX;
    float min_gyro_y = FLT_MAX;
    float min_gyro_z = FLT_MAX;

    int min_gyro = 0;
    for (int i = 0; i < rate * seconds; i++)
    {
        delay_ms(1000 / rate);
        float cur_gyro_x = imu_readGyro_X();
        float cur_gyro_y = imu_readGyro_Y();
        float cur_gyro_z = imu_readGyro_Z();

        max_gyro_x = fmax(max_gyro_x, cur_gyro_x);
        max_gyro_y = fmax(max_gyro_y, cur_gyro_y);
        max_gyro_z = fmax(max_gyro_z, cur_gyro_z);

        min_gyro_x = fmin(min_gyro_x, cur_gyro_x);
        min_gyro_y = fmin(min_gyro_y, cur_gyro_y);
        min_gyro_z = fmin(min_gyro_z, cur_gyro_z);
    }

    assert_greater_than_equal_float(min_gyro_x, -max_gyro_fluctuation, 0.0f);
    assert_greater_than_equal_float(min_gyro_y, -max_gyro_fluctuation, 0.0f);
    assert_greater_than_equal_float(min_gyro_z, -max_gyro_fluctuation, 0.0f);
    assert_less_than_equal_float(max_gyro_x, max_gyro_fluctuation, 0.001f);
    assert_less_than_equal_float(max_gyro_y, max_gyro_fluctuation, 0.001f);
    assert_less_than_equal_float(max_gyro_z, max_gyro_fluctuation, 0.001f);
}
