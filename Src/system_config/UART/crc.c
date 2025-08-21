/******************************************************************************
* File:             crc.c
*
* Author:           James Knepper  
* Created:          2025-08-20 1900
* Description:      See crc.h
*****************************************************************************/

#include "UART/pcp.h"
#include <stdio.h>

#define POLYAL 0xD8
#define MAX_PAYLOAD_BYTES 255
#define CRC_CHECK_SIZE sizeof(uint8_t)
#define MAX_MESSAGE_BYTES MAX_PAYLOAD_BYTES + CRC_CHECK_SIZE

/**
 * Calculate the CRC remainder to be appended 
 */
uint8_t crc_remainder(uint8_t *payload, int nbytes) {
    uint8_t remainder = 0;

    for (int byte_index = 0; byte_index < nbytes; byte_index++) {
        remainder ^= payload[byte_index];

        for (uint8_t bit = 8; bit > 0; bit--) {
            if (remainder & 0x80) {
                remainder = (remainder << 1) ^ POLYAL;
            }
            else {
                remainder = (remainder << 1);
            }
        }
    }

    return remainder;
}

void crc_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes) {
    uint8_t buffer[MAX_MESSAGE_BYTES];
    memcpy(buffer, payload, nbytes);
    buffer[nbytes] = crc_remainder(buffer, nbytes);
    usart_transmitBytes(bus, buffer, nbytes + CRC_CHECK_SIZE);
}

int crc_read(USART_TypeDef *bus, uint8_t* buf) {
    uint8_t buffer[MAX_MESSAGE_BYTES];
    size_t size = usart_receiveBytes(bus, buffer, MAX_MESSAGE_BYTES);
    if (!crc_remainder(buffer, size)) {
        return -1;
    }
    memcpy(buf, buffer, size - CRC_CHECK_SIZE);
    return size;
}