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
extern void testFunction_DMA_ADC();
extern void testFunction_watchdog();
extern void testFunction_getCalendar();
extern void testFunction_LPSleep();
extern void testFunction_RTC_Alarm();
extern void testFunction_mgtIntercom();

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

    case 16:
			return testFunction_mgtIntercom;
    case 18:
			return testFunction_RTC_Alarm;
    case 19:
			return testFunction_getCalendar;
    case 20:
			return testFunction_DMA_ADC;
		case 23:
			return testFunction_LPSleep;
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
