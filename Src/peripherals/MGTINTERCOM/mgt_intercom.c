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
 * Intialize the PFC->MGT Intercom USART devices
 * 
 * @returns None
 */
 void mgt_intercom_init() {
   usart_init(MGT_USART_BUS, PFC2MGT_BAUDRATE);
}

 /**
  * Set the PWM for a coil
  * 
  * @param coil_number  The number of the coil whose PWM is being set
  * @param pwm  1 or 0, for pwm 1 or 0
  * @param percent  The PWM percentage (0-100)
  * 
  * @returns Boolean denoting whether the MGT side responded
  */
 bool mgt_intercom_set_coil_percent(int coil_number, int pwm, int percentage) {
    uint8_t payload[7];
    payload[0] = 'S';
    payload[1] = coil_number + '0';
    payload[2] = ' ';
    payload[3] = pwm + '0';
    payload[4] = ' ';
    payload[5] = percentage / 10 + '0';
    payload[6] = percentage % 10 + '0';
    crc_transmit(MGT_USART_BUS, payload, 7);
    crc_read(MGT_USART_BUS, payload);
    return payload[0] == 'A';
 }

 /**
  * Get the current through a coil
  * 
  * @param coil_number  the number of the coil
  * 
  * @returns  the current in Amps (A), or -1 if nothing was read
  */
 float mgt_intercom_get_current(int coil_number) {
    uint8_t payload[2];
    payload[0] = 'C';
    payload[1] = coil_number + '0';
    crc_transmit(MGT_USART_BUS, payload, 2);
    float buffer[64];
    if (crc_read(MGT_USART_BUS, buffer) < 0) return -1;
    return buffer[0];
 }

 /**
  * Shut down all PWMs and timers on the MGT side
  * 
  * @returns Boolean denoting whether the MGT side responded
  */
 bool mgt_intercom_shutdown_all() {
    uint8_t payload[1];
    payload[0] = 'D';
    crc_transmit(MGT_USART_BUS, payload, 1);
    crc_read(MGT_USART_BUS, payload);
    return payload[0] == 'A';
 }

 /**
  * Shut down a specific timer
  * 
  * @param timer_number  The id of the timer to be turned off
  * 
  * @returns Boolean denoting whether the MGT side responded
  */
 bool mgt_intercom_shutdown_timer(int timer_number) {
    uint8_t payload[2];
    payload[0] = 'T';
    payload[1] = timer_number + '0';
    crc_transmit(MGT_USART_BUS, payload, 2);
    crc_read(MGT_USART_BUS, payload);
    return payload[0] == 'A';
 }