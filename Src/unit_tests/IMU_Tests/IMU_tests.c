#include "IMU_tests.h"
#include <math.h>
#include <float.h>

void test_initial_state()
{
    printMsg("\r\ntest intiail state\r\n");

    set_IMU(IMU0);
    imu_init(IMU_ODR_3333_Hz, IMU_FS_16_g, IMU_ODR_3333_Hz, IMU_FS_125_dps);
    printMsg("Stable while flat (IMU0): \r\n Gyro: %d\r\n", test_stable_while_flat(2.5f, 1, 5));

    set_IMU(IMU1);
    imu_init(IMU_ODR_3333_Hz, IMU_FS_16_g, IMU_ODR_3333_Hz, IMU_FS_125_dps);
    printMsg("Stable while flat (IMU1): \r\n Gyro: %d\r\n", test_stable_while_flat(2.5f, 1, 5));

    set_IMU(IMU0);
    imu_init(IMU_ODR_3333_Hz, IMU_FS_16_g, IMU_ODR_3333_Hz, IMU_FS_125_dps);
    while (1)
    {
        printMsg("Stable while flat (IMU0): \r\n Gyro: %d\r\n", test_stable_while_flat(2.5f, 1, 5));
    }
}

bool test_stable_while_flat(float max_gyro_fluctuation, int rate, int seconds)
{
    float max_gyro_x = FLT_MIN;
    float max_gyro_y = FLT_MIN;
    float max_gyro_z = FLT_MIN;

    float min_gyro_x = FLT_MAX;
    float min_gyro_y = FLT_MAX;
    float min_gyro_z = FLT_MAX;

    float max_accel_x = FLT_MIN;
    float max_accel_y = FLT_MIN;
    float max_accel_z = FLT_MIN;

    float min_accel_x = FLT_MAX;
    float min_accel_y = FLT_MAX;
    float min_accel_z = FLT_MAX;

    int min_gyro = 0;
    for (int i = 0; i < rate * seconds; i++)
    {
        delay_ms(1000 / rate);
        float cur_gyro_x = imu_readGyro_X();
        float cur_gyro_y = imu_readGyro_Y();
        float cur_gyro_z = imu_readGyro_Z();

        float cur_accel_x = imu_readAcel_X();

        max_gyro_x = fmax(max_gyro_x, cur_gyro_x);
        max_gyro_y = fmax(max_gyro_y, cur_gyro_y);
        max_gyro_z = fmax(max_gyro_z, cur_gyro_z);

        min_gyro_x = fmin(min_gyro_x, cur_gyro_x);
        min_gyro_y = fmin(min_gyro_y, cur_gyro_y);
        min_gyro_z = fmin(min_gyro_z, cur_gyro_z);
    }

    printMsg("\r\n--------------------------\r\n");
    printMsg("max X: %f\r\n", max_gyro_x);
    printMsg("max Y: %f\r\n", max_gyro_y);
    printMsg("max Z: %f\r\n", max_gyro_z);

    printMsg("min X: %f\r\n", min_gyro_x);
    printMsg("min Y: %f\r\n", min_gyro_y);
    printMsg("min Z: %f\r\n", min_gyro_z);

    

    return min_gyro_x >= -max_gyro_fluctuation &&
           min_gyro_y >= -max_gyro_fluctuation &&
           min_gyro_z >= -max_gyro_fluctuation &&
           max_gyro_x <= max_gyro_fluctuation &&
           max_gyro_y <= max_gyro_fluctuation &&
           max_gyro_z <= max_gyro_fluctuation;
}
