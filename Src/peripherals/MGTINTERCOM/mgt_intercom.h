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
 #include <UART/crc.h>

#define MGT_USART_BUS USART2
#define PFC2MGT_BAUDRATE 9600

/**
 * Intialize the PFC->MGT Intercom USART devices
 * 
 * @returns None
 */
 void mgt_intercom_init();

 /**
  * Set the PWM for a coil
  * 
  * @param coil_number  The number of the coil whose PWM is being set
  * @param pwm  1 or 0, for pwm 1 or 0
  * @param percent  The PWM percentage (0-100)
  * 
  * @returns Boolean denoting whether the MGT side responded
  */
 bool mgt_intercom_set_coil_percent(int coil_number, int pwm, int percentage);

 /**
  * Get the current through a coil
  * 
  * @param coil_number  the number of the coil
  * 
  * @returns  the current in Amps (A), or -1 if nothing was read
  */
 float mgt_intercom_get_current(int coil_number);

 /**
  * Shut down all PWMs and timers on the MGT side
  * 
  * @returns Boolean denoting whether the MGT side responded
  */
 bool mgt_intercom_shutdown_all();

 /**
  * Shut down a specific timer
  * 
  * @param timer_number  The id of the timer to be turned off
  * 
  * @returns Boolean denoting whether the MGT side responded
  */
 bool mgt_intercom_shutdown_timer(int timer_number);