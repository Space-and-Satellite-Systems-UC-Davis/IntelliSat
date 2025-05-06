/**
 * @file main.c
 * @brief Entrypoint of kernel systems
 *
 * Contains initial setup, and beginning of
 * main superloop for responsible for uninterrupted
 * runtime.
 *
 * @authors Nithin Senthil, Parteek Singh, Jacob Tkeo
 * @date 6/../23
 */

 #include "platform_init.h"
 #include <print_scan.h>
 #include "system_config/LED/led.h"
 
 /* Globals */
 #include "scheduler/schedulerGlobals.h"
 
 /* File Includes */
 // #include "scheduler/watchdog.h"
 // #include "scheduler/intelliTask.h"
 // #include "scheduler/status.h"
 
 #include "FreeRTOS.h"
 
 // #include "task.h"
 
 /* Macros */
 #define SYSTICK_DUR_U 10000          // Config. of systick timer in usec (1 ms)
 #define BATTERY_THRESHOLD 20        // TODO: Min. battery voltage value, below which mode -> CHARGING
 
 /**
  * @brief The code inside the while loop will get run continuously until the reset button gets hit,
  * at which point the program will restart.
  *
  * Contains initial timer and interrupt handler (TESTING) and
  * the main superloop. Serves as standard behavior when
  * there is no scheduler intervention.
  */
 int branch_main() {
     // ! Your training main code goes here
     // TODO
     // To get you familiar with how to use the (one of many) Intellisat Drivers, do the following:
     // In the while loop, blink led(s) of your choice.
     // You must include some amount of delay in between the led blinks.
     // You should around the system_config folder and accociated directories for led related functions
     // For delays, use the void nop(long long nop_loops); located in src/inc/globals.h.
     // It delays by nop_loops milliseconds.
     // Only functions marked under the OP_REV == 3 macro is usable on the our board.
     while(1) {}
     return 0;
 
 }
 
 #define RUN_TEST	0	// 0 = run IntelliSat, 1 = run a very specific test
 #define TEST_ID 	1	// ID of the test to run in case RUN_TEST = 1
 
 // #include <TestDefinition.h>
 
 int main() {
     init_init();
     
     //TODO: use RTC first_time flag.
     //if (first_time) {
         init_first_time();
     //}
     
     init_platform(!RUN_TEST);
     // ^ don't want to run the Scheduler in case we are running other tests
     
     branch_main();
 }
 