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
 #include <MGTINTERCOM/mgt_intercom.h>

/**
 * Intialize the PFC->MGT Intercom USART and PCP devices
 * 
 * @param pcp  A pointer to an empty pcp device
 * 
 * @returns None
 */
void mgt_intercom_init(PCPDevice *pcp) {
   usart_init(USART2, PFC2MGT_BAUDRATE);
   make_pcpdev(pcp, USART2);
}

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
 bool mgt_intercom_set_coil_percent(PCPDevice * device, int coil_number, int pwm, int percentage) {
    uint8_t payload[7];
    payload[0] = 'S';
    payload[1] = coil_number + '0';
    payload[2] = ' ';
    payload[3] = pwm + '0';
    payload[4] = ' ';
    payload[5] = percentage / 10 + '0';
    payload[6] = percentage % 10 + '0';
    if (pcp_transmit(device, payload, 7)) return false;
    return true;
 }

 /**
  * Get the current through a coil
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param coil_number  the number of the coil
  * 
  * @returns  the current in Amps (A)
  */
 float mgt_intercom_get_current(PCPDevice * device, int coil_number) {
    uint8_t payload[2];
    payload[0] = 'C';
    payload[1] = coil_number + '0';
    pcp_transmit(device, payload, 2);
    float buffer[64];
    pcp_read(device, buffer);
    return buffer[0];
 }

 /**
  * Shut down all PWMs and timers on the MGT side
  * 
  * @param device  A pointer to the PCP device of the MGT
  * 
  * @returns  boolean successful
  */
 bool mgt_intercom_shutdown_all(PCPDevice * device) {
    uint8_t payload[1];
    payload[0] = 'D';
    if (pcp_transmit(device, payload, 1)) return false;
    return true;
 }

 /**
  * Shut down a specific timer
  * 
  * @param device  A pointer to the PCP device of the MGT
  * @param timer_number  The id of the timer to be turned off
  * 
  * @returns  boolean successful
  */
 bool mgt_intercom_shutdown_timer(PCPDevice * device, int timer_number) {
    uint8_t payload[2];
    payload[0] = 'T';
    payload[1] = timer_number + '0';
    if (pcp_transmit(device, payload, 2)) return false;
    return true;
 }