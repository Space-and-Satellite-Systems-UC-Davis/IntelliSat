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
#include "UART/crc.h"


void mgt_intercom_init() {
   usart_init(MGT_USART_BUS, PFC2MGT_BAUDRATE);
}

bool mgt_intercom_setCoilPercent(int coil_number, int percentage) {
    uint8_t payload[MAX_MESSAGE_BYTES];
    payload[0] = 'S';
    payload[1] = coil_number + '0';
    payload[2] = ' ';
    payload[3] = percentage;
    return crc_transmit(MGT_USART_BUS, payload, 4);
}

float mgt_intercom_getCurrent(int coil_number) {
	uint8_t payload[MAX_MESSAGE_BYTES];
	payload[0] = 'C';
	payload[1] = coil_number + '0';
	crc_transmit(MGT_USART_BUS, payload, 2);
	float buffer[64];
	if (crc_read(MGT_USART_BUS, buffer) < 0) return -1;
	return buffer[0];
}

bool mgt_intercom_shutdownAll() {
    uint8_t payload[MAX_MESSAGE_BYTES];
    payload[0] = 'D';
    return crc_transmit(MGT_USART_BUS, payload, 1);
}

bool mgt_intercom_shutdownTimer(int timer_number) {
    uint8_t payload[MAX_MESSAGE_BYTES];
    payload[0] = 'T';
    payload[1] = timer_number + '0';
    return crc_transmit(MGT_USART_BUS, payload, 2);
}

bool mgt_intercom_setCoilDirection(int coilNumber, DIRECTION dir) {
   uint8_t payload[MAX_MESSAGE_BYTES];
    payload[0] = 'W';
    payload[1] = coilNumber + '0';
    payload[2] = ' ';
    if (dir == HIGH) {
       payload[3] = 'H';
    } else {
       payload[3] = 'L';
    }
    return crc_transmit(MGT_USART_BUS, payload, 4);
}
