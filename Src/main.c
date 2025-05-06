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
 // TODO include training_tasks.h
 
 // #include "task.h"
 
 /* Macros */
 #define SYSTICK_DUR_U 10000  // Config. of systick timer in usec (1 ms)
 #define BATTERY_THRESHOLD 20 // TODO: Min. battery voltage value, below which mode -> CHARGING
 #define LED_DELAY 1000
 
 static bool led_state = 0;
 static bool led1_state = 0;
 static bool led2_state = 0;
 static bool led3_state = 0;
 static bool led4_state = 0;
 static bool led5_state = 0;
 static const int led_delay_1 = 1111;
 static const int led_delay_2 = 789;
 
 void toggle_LED(int num)
 {
     switch (num)
     {
     case 1:
         led_d1(!led1_state);
         // Same as led1_state = !led1_state
         led1_state ^= 1;
         break;
     case 2:
         led_d2(!led2_state);
         led2_state ^= 1;
         break;
     case 3:
         led_d3(!led3_state);
         led3_state ^= 1;
         break;
     case 4:
         led_d4(!led4_state);
         led4_state ^= 1;
         break;
     case 5:
         led_d5(!led5_state);
         led5_state ^= 1;
         break;
     default:
         break;
     }
 }
 
 /**
  * 'Blink LED' task.
  */
 static void led_task(void *args)
 {
     int delay_ms = *(int *)args;
     while (1)
     {
         // Toggle the LED.
         int pin = 0;
         led_d1(!led_state);
         led_state = !led_state;
         // Delay for a second-ish.
         vTaskDelay(pdMS_TO_TICKS(delay_ms));
     };
 }
 
 /**
  * @brief The code inside the while loop will get run continuously until the reset button gets hit,
  * at which point the program will restart.
  *
  * Contains initial timer and interrupt handler (TESTING) and
  * the main superloop. Serves as standard behavior when
  * there is no scheduler intervention.
  */
 int branch_main()
 {
     // ! Your training main code goes here
     // TODO
     // To get you familiar with how to use the (one of many) Intellisat Drivers, do the following:
     // In the while loop, blink led(s) of your choice. There must be at least 
     // You must include a delay in between the led blinks
     // You should look into the Src/system_config/LED/led.h header
     // Only functions marked under the OP_REV == 3 macro is usable on the our board.
     while (1)
     {
         toggle_LED(1);
         nop(LED_DELAY);
         toggle_LED(2);
         nop(LED_DELAY);
         toggle_LED(3);
         nop(LED_DELAY);
         toggle_LED(4);
         nop(LED_DELAY);
         toggle_LED(5);
         nop(LED_DELAY);
     }
     return 0;
 }
 
 #define RUN_TEST 0 // 0 = run IntelliSat, 1 = run a very specific test
 #define TEST_ID 1  // ID of the test to run in case RUN_TEST = 1
 
 // #include <TestDefinition.h>
 
 int main()
 {
     init_init();
 
     // TODO: use RTC first_time flag.
     // if (first_time) {
     init_first_time();
     //}
 
     init_platform(!RUN_TEST);
     // ^ don't want to run the Scheduler in case we are running other tests
 
     branch_main();
 }
 