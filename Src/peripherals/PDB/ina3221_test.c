/*
 * ina3221_test.c
 *
 *  Created on: Aug 15, 2026
 */
#include "globals.h"
#include "ina3221.h"

void testFunction_INA3221_PDB() {
//	ina3221_init(0, 4, 4, 68, 30);

	nop(1);
	while (true) {
		ina3221_init(0, 4, 4, 68, 30);
		delay_ms(1000);
	}
}
