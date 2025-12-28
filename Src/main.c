#include <print_scan.h>
#include "platform_init.h"

#define RUN_TEST	1	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	10	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>
#include "../ADCS-Software/control/experiment/PID_experiment.h"
#include "../ADCS-Software/ADCS.h"
#include "../hdd/hdd_init.h"
#include "../hdd/hdd_drive.h"

int main() {
    init_init();
    init_platform(!RUN_TEST);

#if (RUN_TEST==1) && (TEST_ID != 0)

    void (*testFunc)();
    testFunc = getTestFunction(TEST_ID);
    testFunc();

    #else

	//TODO: use RTC first_time flag.
	//if (first_time) {
	//  init_first_time()
	//}

    led_d2(1);
    led_d3(0);
    printMsg("Begin HDD setup for PID experiment \r\n");
	hdd_init(PWM0);
	hdd_init(PWM1);
	hdd_arm(PWM0);
	hdd_arm(PWM1);
	delay_ms(3000);
	printMsg("Finished preparation; running PID experiment \r\n");
    PID_status expResult = ADCS_MAIN(ADCS_HDD_EXP_ANGVEL);
    printMsg("Called PID_Experiment with result ");
    printMsg(expResult == PID_EXPERIMENT_SUCCESS ? "SUCCESS" : "FAILURE");
    printMsg("\r\n");
    led_d2(0);
    led_d3(1);

	while (1) {
		continue;
	}

#endif

}
