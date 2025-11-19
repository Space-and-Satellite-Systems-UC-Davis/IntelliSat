/******************************************************************************
* File:             crc.c
*
* Author:           James Knepper  
* Created:          2025-08-20 1900
* Description:      See crc.h
******************************************************************************/

#include "UART/crc.h"
#include <string.h>
#include "print_scan.h"

int crc_wait(USART_TypeDef *bus) {
    uint8_t ack = '\0';
    int count = usart_receiveBytes(bus, &ack, 1);
    return (ack == 'A') - (count != 1); // receives nothing -> -1, receives noise -> 0, receives ACK -> 1.
}

void crc_ack(USART_TypeDef *bus) {
    usart_transmitBytes(bus, "A", 1);
}

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

bool crc_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes) {
    printMsg("Transmitting: '%s'\r\n", payload);
    uint8_t buffer[MAX_INTERNAL_BYTES];
    uint8_t breaks = 0;
    memset(buffer, 0, MAX_INTERNAL_BYTES);
    for (int index = 0; index < nbytes; index++){
        if (payload[index] == ';' || payload[index] == '\\') {
            buffer[index + breaks] = '\\';
            breaks++;
        } 
        buffer[index + breaks] = payload[index];
    }
    uint8_t remainder = crc_remainder(buffer, nbytes + breaks);
    if (remainder == '\\' || remainder == ';') {
        buffer[nbytes + breaks] = '\\';
        breaks++;
    }
    buffer[nbytes + breaks] = remainder;
    buffer[nbytes + breaks + 1] = ';';
    bool ack = false;
    for (int attempts = 0; attempts < 5; attempts++) {
        usart_transmitBytes(bus, buffer, nbytes + breaks + 1 + 1);
        ack = crc_wait(bus);
        if (ack == 0) attempts--;
        if (ack == 1) break;
    }
    return ack;
}

int crc_read(USART_TypeDef *bus, uint8_t* buf) {
    uint8_t buffer[MAX_MESSAGE_BYTES];
    int size = usart_receiveBytes(bus, buffer, MAX_MESSAGE_BYTES);
    printMsg("Received: '%s'\r\n", buffer);
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
            return index - 1;
        }
        buf[index] = buffer[index + breaks];
    }
    return size - breaks - 1;
}

bool crc_chunked_transmit(USART_TypeDef *bus, uint8_t *payload, int nbytes, int lchunks) {
    int nchunks = ((nbytes - 1) / lchunks) + 1;
    bool cumulative_success = true;
    uint8_t subchunk[MAX_PAYLOAD_BYTES];
    for (int i = 0; i < nchunks; i++) {
        subchunk[0] = i;
        memcpy(&subchunk[0] + 1, payload + i*lchunks, lchunks);
        cumulative_success &= crc_transmit(bus, subchunk, lchunks + 1);
    }
    return cumulative_success;
}

int crc_chunked_read(USART_TypeDef *bus, uint8_t* buf, int lchunks, int nchunks) {
    uint8_t subchunk[MAX_PAYLOAD_BYTES];
    int read = 0;
    for (int i = 0; i < nchunks; i++) {
        crc_read(bus, subchunk);
        if (subchunk[0] == i) read += lchunks;
        if (subchunk[0] > nchunks) return -1;
        memcpy(buf + subchunk[0]*lchunks, &subchunk[1], lchunks);
    }
    return read;
}