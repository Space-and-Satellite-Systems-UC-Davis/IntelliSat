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
#include "./scheduler/schedulerGlobals.h"

/* File Includes */
#include "scheduler/scheduler.h"
#include "scheduler/myTask.h"
#include "scheduler/status.h"

#include "../FreeRTOS/Source/include/FreeRTOS.h"
#include "../FreeRTOS/Source/include/task.h"
//#include "FreeRTOS.h"
//#include "task.h"

/* Macros */
#define SYSTICK_DUR_U 10000          // Config. of systick timer in usec (1 ms)
#define BATTERY_THRESHOLD 20        // TODO: Min. battery voltage value, below which mode -> CHARGING

/* Misc variables */
int reboot_count;
// volatile uint16_t flagBits = 0;     // Declared in status.h
volatile struct OperationBits flagBits = {0,0};

/* Testing Variables */
int max_handler_count;
int systick_handler_count;
int is_unlimited_tick;

// Define signal handler and timer
struct sigaction sysTick;
struct itimerval sysTick_timer;

uint32_t main_stack_frame[32];
volatile uint32_t main_PC;

static bool ledState = 0;

uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
//void SystemClock_Config(void);


/* Prototypes */
void sysTickHandler(int signal);
//jmp_buf to_mode_select;

/**
 * @brief Initial startup mode
 *
 * The one-time 30 minute wait when the Intellisat
 * is first released from the ISS.
 *
 * @note Load initial values from flash
 * @todo Replace sleep time to 30 min
 */
void startup() {
    // TODO: Retrieve base info from flash (flagBits, reboot_count, etc.)
    reboot_count++;
    if (!IS_BIT_SET(flagBits.statusBits, START)) {
        //printMsg("First startup detected\nStarting 5 second wait...\n");
        delay_ms(5000); // TODO: replace with correct wait period (30min)

        SET_BIT(flagBits.statusBits, START); // TODO: Intentionally at the end in case of failure during wait state (REVISIT)

    } else {
        //printMsg("Loading Backups\nPlease wait (5s)...\n");
        delay_ms(5000); // TESTING: remove when backups implemented
        // TODO: Load backups here
    }
}


static const int led_delay_1 = 1111;
static const int led_delay_2 = 789;

void toggleLED(int pin) {
    led_dx(pin, !ledState);
    ledState = !ledState;
}


/**
 * 'Blink LED' task.
 */
static void led_task(void *args) {
  int delay_ms = *(int*)args;
  

  while (1) {
    // Toggle the LED.
	  int pin = 0;
	  led_dx(pin, !ledState);
	  ledState = !ledState;
    // Delay for a second-ish.
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
  };
}

/**
 * @brief Superloop
 *
 * Contains initial timer and interrupt handler (TESTING) and
 * the main superloop. Serves as standard behavior when
 * there is no scheduler intervention.
 */
int branch_main() {

    xTaskCreate(led_task, "LED_blink_1", 128, (void*)&led_delay_1, configMAX_PRIORITIES-1, NULL);
    xTaskCreate(led_task, "LED_blink_2", 128, (void*)&led_delay_2, configMAX_PRIORITIES-1, NULL);

    vTaskStartScheduler();

    while(1) {}

    return 0;

}

#define RUN_TEST	1	// 0 = run IntelliSat, 1 = run a very specific test
#define TEST_ID 	1	// ID of the test to run in case RUN_TEST = 1

#include <TestDefinition.h>

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
