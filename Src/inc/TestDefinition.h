#pragma once

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           TEST DECLARATIONS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

extern void testFunction_Nothing();
extern void testFunction_IMU_AcelGyro_Reads();
extern void testFunction_MAG_Reads();
extern void testFunction_LogTimer_Callback();
extern void testerFunction_PWMGeneration();
extern void testFunction_IMU_Dupe_Reads();
extern void testFunction_Diode_Reads();
extern void testFunction_INA_TMP_Outputs();
extern void testFunction_watchdog();

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           TEST REGISTRATION
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// Function that returns a pointer to a void function
void (*getTestFunction(int test_id))(void) {
	//! Every case must return a void function
	switch (test_id) {
		case 0:
			return testFunction_Nothing;
		case 1:
			return testFunction_IMU_AcelGyro_Reads;
		case 2:
			return testFunction_MAG_Reads;
		case 3:
			return testFunction_LogTimer_Callback;
		case 4:
			return testerFunction_PWMGeneration;
		case 8:
			return testFunction_IMU_Dupe_Reads;
		case 11:
			return testFunction_INA_TMP_Outputs;
		case 12:
			return testFunction_Diode_Reads;
		case 14: 
			return testFunction_watchdog;
		default:
			return testFunction_Nothing;
	}
	return testFunction_Nothing;
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           	  EXTRAS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_Nothing() {
	while(1);
}
