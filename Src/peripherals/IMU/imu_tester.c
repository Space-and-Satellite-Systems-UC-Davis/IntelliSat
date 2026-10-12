#include "ASM330LHH.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_IMU_AcelGyro_Reads() {
	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);

	float acel_X = 0;
	float acel_Y = 0;
	float acel_Z = 0;
	float gyro_X = 0;
	float gyro_Y = 0;
	float gyro_Z = 0;

	while (1) {
		if (get_buttonStatus_1()) {
			acel_X = imu_readAcel_X();
			acel_Y = imu_readAcel_Y();
			acel_Z = imu_readAcel_Z();

			gyro_X = imu_readGyro_X();
			gyro_Y = imu_readGyro_Y();
			gyro_Z = imu_readGyro_Z();
		}
	}
}
