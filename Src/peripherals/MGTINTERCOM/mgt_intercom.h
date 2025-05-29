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

#define PFC2MGT_BAUDRATE 9600

/**
 * Intialize the PFC->MGT Intercom PCP device
 * 
 * @param pcp  A pointer to an empty pcp device
 * 
 * @returns None
 */
void mgt_intercom_init(PCPDevice *pcp);

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
 bool mgt_intercom_set_coil_percent(PCPDevice * device, int coil_number, int pwm, int percentage);

 /**
  * Get the current through a coil
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param coil_number  the number of the coil
  * 
  * @returns  the current in Amps (A)
  */
 float mgt_intercom_get_current(PCPDevice * device, int coil_number);

 /**
  * Shut down all PWMs and timers on the MGT side
  * 
  * @param device  A pointer to the PCP device of the MGT
  * 
  * @returns  boolean successful
  */
 bool mgt_intercom_shutdown_all(PCPDevice * device);

 /**
  * Shut down a specific timer
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param timer_number  The id of the timer to be turned off
  * 
  * @returns boolean successful
  */
 bool mgt_intercom_shutdown_timer(PCPDevice * device, int timer_number);