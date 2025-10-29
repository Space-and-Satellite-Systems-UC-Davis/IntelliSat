/******************************************************************************
* File:             crc.c
*
* Author:           James Knepper  
* Created:          2025-08-20 1900
* Description:      See crc.h
*****************************************************************************/

#include "UART/crc.h"
#include <string.h>
#include "print_scan.h"

/**
 * Calculate the CRC remainder to be appended 
 */
uint8_t crc_remainder(uint8_t payload[], int nbytes) {
    uint8_t remainder = 0;
    bool unbroken = true;
    for (int byte_index = 0; byte_index < nbytes; byte_index++) {
        if (payload[byte_index] == '\\' && unbroken) {
            unbroken = false;
            continue;
        }
        if (payload[byte_index] == ';' && unbroken) {
            return remainder;
        }
        unbroken = true;
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
    uint8_t buffer[2*MAX_MESSAGE_BYTES];
    uint8_t breaks = 0;
    for (int index = 0; index < nbytes; index++){
        if (payload[index] == ';' || payload[index] == '\\') {
            buffer[nbytes + breaks] = '\\';
            breaks++;
        } 
        buffer[nbytes + breaks] = payload[index];
    }
    uint8_t remainder = crc_remainder(buffer, nbytes + breaks);
    if (remainder == '\\' || remainder == ';') {
        buffer[nbytes + breaks] = '\\';
        breaks++;
    }
    buffer[nbytes + breaks] = remainder;
    bool ack = false;
    while (!ack) {
        usart_transmitBytes(bus, buffer, nbytes + breaks + CRC_CHECK_SIZE);
        usart_transmitBytes(bus, ";", 1);
        ack = crc_wait(bus);
    }
}

int crc_read(USART_TypeDef *bus, uint8_t* buf) {
    uint8_t buffer[2*MAX_MESSAGE_BYTES];
    int size = usart_receiveBytes(bus, buffer, 2*MAX_MESSAGE_BYTES);
    if (size <= 0) return -1;
    if (crc_remainder(buffer, size)) return -1;
    crc_ack(bus);
    int breaks = 0;
    for (int index = 0; index + breaks < size; index++) {
        if (buffer[index + breaks] == '\\') {
            breaks++;
            buf[index] = buffer[index + breaks];
            continue;
        }
        if (buffer[index + breaks] == ';') {
            return size - breaks;
        }
    }
    return -1;
}

int crc_wait(USART_TypeDef *bus) {
    uint8_t ack[1] = "\0";
    usart_receiveBytes(bus, ack, 1);
    if (ack != 'A') return false;
    else return true;
}

void crc_ack(USART_TypeDef *bus) {
    usart_transmitBytes(bus, "A", 1);
}