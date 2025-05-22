/**
 * mgt_intercomm_intercom.c
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

 /**
  * Set the PWM for a coil
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param coil_number  The number of the coil whose PWM is being set
  * @param pwm  1 or 0, for pwm 1 or 0
  * @param percent  The PWM percentage (0-100)
  * 
  * @returns  Success (1) or failure (0)
  */
 bool mgt_intercomm_set_coil_percent(PCPDevice * device, int coil_number, int pwm, int percentage) {
    uint8_t payload[7];
    payload[0] = 'S';
    payload[1] = coil_number + '0';
    payload[2] = ' ';
    payload[3] = pwm + '0';
    payload[4] = ' ';
    payload[5] = percentage / 10 + '0';
    payload[6] = percentage % 10 + '0';
    if (!pcp_transmit(device, payload, 7)) return 0;
    return 1;
 }

 /**
  * Get the current through a coil
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param coil_number  the number of the coil
  * 
  * @returns  the current in Amps (A)
  */
 float mgt_intercomm_get_current(PCPDevice * device, int coil_number) {
    uint8_t payload[2];
    payload[0] = 'C';
    payload[1] = coil_number + '0';
    pcp_transmit(device, payload, 2);
    uint8_t buffer[256];
    pcp_read(device, buffer);
    return buffer[0];
 }

 /**
  * Shut down all PWMs and timers on the MGT side
  * 
  * @param device  A pointer to the PCP device of the MGT
  * 
  * @returns  success (1) or failure (0)
  */
 bool mgt_intercomm_shutdown_all(PCPDevice * device);

 /**
  * Shut down a specific timer
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param timer_number  The id of the timer to be turned off
  * 
  * @returns Success (1) or failure (0)
  */
 bool mgt_intercomm_shutdown_timer(PCPDevice * device, int timer_number);