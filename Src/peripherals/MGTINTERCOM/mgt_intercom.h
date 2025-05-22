/**
 * mgt_intercomm_intercom.h
 * 
 * Driver for communication with the magnetorquer
 * 
 * Author: James Knepper (2025)
 * 
 * Log:
 * 21 May 2025 - Initial
 */

 #include <globals.h>
 #include <UART/pcp.h>

void mgt_intercomm_init();

 /**
  * Set the PWM for a coil
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param coil_number  The number of the coil whose PWM is being set
  * @param pwm  1 or 0, for pwm 1 or 0
  * @param percent  The PWM percentage (0-100)
  * 
  * @returns  boolean successful
  */
 bool mgt_intercomm_set_coil_percent(PCPDevice * device, int coil_number, int pwm, int percentage);

 /**
  * Get the current through a coil
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param coil_number  the number of the coil
  * 
  * @returns  the current in Amps (A)
  */
 float mgt_intercomm_get_current(PCPDevice * device, int coil_number);

 /**
  * Shut down all PWMs and timers on the MGT side
  * 
  * @param device  A pointer to the PCP device of the MGT
  * 
  * @returns  boolean successful
  */
 bool mgt_intercomm_shutdown_all(PCPDevice * device);

 /**
  * Shut down a specific timer
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param timer_number  The id of the timer to be turned off
  * 
  * @returns boolean successful
  */
 bool mgt_intercomm_shutdown_timer(PCPDevice * device, int timer_number);