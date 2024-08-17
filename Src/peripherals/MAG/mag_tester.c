#include "QMC5883L.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_MAG_Reads() {
	mag_init(MAG_ODR_200_Hz, MAG_FS_2_G, MAG_OVERSAMPLE_512);

	float mag_X = 0;
	float mag_Y = 0;
	float mag_Z = 0;

	while (1) {
		if (get_buttonStatus_1()) {
			mag_X = mag_read_X();
			mag_Y = mag_read_Y();
			mag_Z = mag_read_Z();
		}
	}
}
