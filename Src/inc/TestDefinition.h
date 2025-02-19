#pragma once
#include "../testing/unity.h"

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           TEST DECLARATIONS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

extern void testFunction_IMU_AcelGyro_Reads();
extern void testFunction_MAG_Reads();
extern void testFunction_LogTimer_Callback();
extern void testerFunction_PWMGeneration();

extern void testFunction_Nothing();
extern void testFunction_TestUnity();

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
		case 5:
			return testFunction_TestUnity;
		default:
			return testFunction_Nothing;
	}
	return testFunction_Nothing;
}


// Run every test
void runAllTests() {

}

//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
//                           	  EXTRAS
//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-

void testFunction_TestUnity() {
	TEST_ASSERT(1);
}

void testFunction_Nothing() {
	while(1);
}
