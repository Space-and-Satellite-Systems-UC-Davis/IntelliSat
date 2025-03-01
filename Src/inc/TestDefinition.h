#pragma once

#include <print_scan.h>

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           TEST DECLARATIONS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

typedef enum {
    TEST_NOTHING,
    TEST_IMU_ACEL_GYRO_READS,
    TEST_IMU_DUPE_READS,
    TEST_MAG_READS,
    TEST_LOGTIMER_CALLBACK,
    TEST_PWMGENERATION,
    TEST_PCP,
    TEST_UART,
} Test;

extern void testFunction_Nothing();
extern void testFunction_IMU_AcelGyro_Reads();
extern void testFunction_MAG_Reads();
extern void testFunction_LogTimer_Callback();
extern void testerFunction_PWMGeneration();
extern void testFunction_PCP();
extern void testFunction_UART();
extern void testFunction_IMU_Dupe_Reads();

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           TEST REGISTRATION
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

// Function that returns a pointer to a void function
void (*getTestFunction(Test test))(void) {
	//! Every case must return a void function
	switch (test) {
		case TEST_NOTHING:
			return testFunction_Nothing;
		case TEST_IMU_ACEL_GYRO_READS:
			return testFunction_IMU_AcelGyro_Reads;
		case TEST_MAG_READS:
			return testFunction_MAG_Reads;
		case TEST_LOGTIMER_CALLBACK:
			return testFunction_LogTimer_Callback;
		case TEST_PWMGENERATION:
			return testerFunction_PWMGeneration;
		case TEST_PCP:
			return testFunction_PCP;
		case TEST_UART:
			return testFunction_UART;
		case TEST_IMU_DUPE_READS:
			return testFunction_IMU_Dupe_Reads;
		default:
			return testFunction_Nothing;
	}
	return testFunction_Nothing;
}


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           	  EXTRAS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_Nothing() {
    printMsg("Hello, world");
	while(1);
}
