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
			imu_printAllValues();
			set_IMU(IMU1);
			imu_printAllValues();
		}
	}
}
