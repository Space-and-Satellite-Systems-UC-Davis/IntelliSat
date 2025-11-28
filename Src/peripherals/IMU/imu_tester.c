#include "ASM330LHH.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_IMU_AcelGyro_Reads() {
	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);

	while (1) {
		if (get_buttonStatus_1()) {
			float acel_X = ((int16_t)imu_readAcel_X() * 0.061) / 1000;
			float acel_Y = ((int16_t)imu_readAcel_Y() * 0.061) / 1000;
			float acel_Z = ((int16_t)imu_readAcel_Z() * 0.061) / 1000;

			float gyro_X = ((int16_t)imu_readGyro_X() * 35.0) / 1000;
			float gyro_Y = ((int16_t)imu_readGyro_Y() * 35.0) / 1000;
			float gyro_Z = ((int16_t)imu_readGyro_Z() * 35.0) / 1000;
			printMsg("Accelerometer X: %f", acel_X);
			printMsg("Accelerometer Y: %f", acel_Y);
			printMsg("Accelerometer Z: %f", acel_Z);
			printMsg("IMU X: %f", gyro_X);
			printMsg("IMU Y: %f", gyro_Y);
			printMsg("IMU Z: %f", gyro_Z);
		}
	}
}
