#include "ASM330LHH.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>
#include <../system_config/LED/led.h>

void testFunction_IMU_Dupe_Reads() {
	while(1){
//	printMsg("IMU0 Selected\r\n");
	if (get_buttonStatus_SW1()) {
			set_IMU(IMU0);
			printMsg("IMU0 Selected\r\n");
			printMsg("Gyro\r\n");
			printMsg("X: %f\r\n", imu_readGyro_X());
			printMsg("Y: %f\r\n", imu_readGyro_Y());
			printMsg("Z: %f\r\n", imu_readGyro_Z());
			printMsg("Accel:\r\n");
			printMsg("X: %f\r\n", imu_readAcel_X());
			printMsg("Y: %f\r\n", imu_readAcel_Y());
			printMsg("Z: %f\r\n", imu_readAcel_Z());
			printMsg("%f\r\n", imu_readTemp());

			set_IMU(IMU1);
			printMsg("IMU1 Selected\r\n");
			printMsg("Gyro\r\n");
			printMsg("%d\r\n", imu_readGyro_X());
			printMsg("%d\r\n", imu_readGyro_Y());
			printMsg("%d\r\n", imu_readGyro_Z());
			printMsg("Accel:\r\n");
			printMsg("%d\r\n", imu_readAcel_X());
			printMsg("%d\r\n", imu_readAcel_Y());
			printMsg("%d\r\n", imu_readAcel_Z());
			printMsg("%f\r\n", imu_readTemp());
		}
	}
}
