#include "INA3221.h"

void testFunction_INA3221() {
//	ina3221_init(0, 4, 4, 68, 30);

	while (true) {
		ina3221_init(0, 4, 4, 68, 30);
		nop(5000000);
	}
}
