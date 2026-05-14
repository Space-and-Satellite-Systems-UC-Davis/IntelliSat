#include "QMC5883L.h"
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

void testFunction_MAG_Reads() {

	int16_t mag_X = 0;
	float mag_Y = 0;
	float mag_Z = 0;

	while (1) {
		if (get_buttonStatus_SW1()) {
			printMsg("MAG0\n");
			mag_select(MAG0);
			printMsg("%d\n",  mag_read_X());
			printMsg("%d\n", mag_read_Y());
			printMsg("%d\n", mag_read_Z());

			mag_select(MAG1);
			printMsg("MAG1\n");
			printMsg("%d\n",  mag_read_X());
			printMsg("%d\n", mag_read_Y());
			printMsg("%d\n", mag_read_Z());

		}
	}
}
