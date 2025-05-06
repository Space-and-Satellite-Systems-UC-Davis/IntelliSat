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

static bool led_state = 0;

static const int led_delay_1 = 1111;
static const int led_delay_2 = 789;

void toggle_LED(int pin)
{
  led_d1(!led_state);
  led_state = !led_state;
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

  while (1)
  {

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