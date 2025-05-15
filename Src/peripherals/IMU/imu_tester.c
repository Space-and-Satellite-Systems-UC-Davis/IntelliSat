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
			acel_X = ((int16_t)imu_readAcel_X() * 0.061) / 1000;
			acel_Y = ((int16_t)imu_readAcel_Y() * 0.061) / 1000;
			acel_Z = ((int16_t)imu_readAcel_Z() * 0.061) / 1000;

			gyro_X = ((int16_t)imu_readGyro_X() * 35.0) / 1000;
			gyro_Y = ((int16_t)imu_readGyro_Y() * 35.0) / 1000;
			gyro_Z = ((int16_t)imu_readGyro_Z() * 35.0) / 1000;
			printMsg("Accelerometer X reading: %f\r\n", acel_X);
			printMsg("Accelerometer Y reading: %f\r\n", acel_Y);
			printMsg("Accelerometer Z reading: %f\r\n", acel_Z);
			printMsg("Gyro X reading: %f\r\n", gyro_X);
			printMsg("Gyro Y reading: %f\r\n", gyro_Y);
			printMsg("Gyro Z reading: %f\r\n", gyro_Z);
		}
	}
}
