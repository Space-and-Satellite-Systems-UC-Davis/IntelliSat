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
    uint8_t ack[1];
    bool acked = false;
    int count = 0;
    for(int i = 0; i<10; i++){
        count = usart_receiveBytes(bus, ack, sizeof(ack));
        if (ack[0] == 'A'){
            acked = true;
            break;
        }else if(ack[0] == ';'){
            break; //may have accidentally read through packet
        }
    }
    return acked - (count < 1); // receives nothing -> -1, receives noise -> 0, receives ACK -> 1.
}

void crc_ack(USART_TypeDef *bus) {
    uint8_t ack[1];
    ack[0] = 'A';
    usart_transmitBytes(bus, ack, sizeof(ack));
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
    int ack = 0;
    for (int attempts = 0; attempts < 5; attempts++) {
        usart_transmitBytes(bus, buffer, nbytes+breaks+2);
        ack = crc_wait(bus);
        if (ack != -1) break;
    }
    return ack;
}

int crc_read(USART_TypeDef *bus, uint8_t* buf) {
    uint8_t buffer[MAX_MESSAGE_BYTES];
    memset(buffer, 0, sizeof(buffer));
    uint8_t temp[1];
    int size = 0;
    do{
        int count = usart_receiveBytes(bus, temp, 1);
        if(count == 0) break;
        buffer[size] = temp[0];
        size++;
    }while(buffer[size-1] != ';' && size <= MAX_MESSAGE_BYTES);
    if (size <= 0) return -1;
    if (crc_remainder(buffer, size)) return -1;
    if (buffer[0] == 'A' && buffer[1] == crc_remainder("A", 1) && buffer[2] == ';') return -1;
    crc_ack(bus);
    int breaks = 0;
    for (int index = 0; index + breaks < size && index < MAX_PAYLOAD_BYTES; index++) {
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
    int bytes_copied = 0;
    uint8_t subchunk[MAX_PAYLOAD_BYTES];

    for (int i = 0; i < nchunks; i++) {
        subchunk[0] = i;
        if (bytes_copied + lchunks < nbytes) {
        	// Regular step
            memcpy(&subchunk[1], payload + i*lchunks, lchunks);
            bytes_copied += lchunks;

            if (!crc_transmit(bus, subchunk, lchunks + 1)) return -1;
        } else {
        	// Copy remainder. Avoid sending random junk
            memcpy(&subchunk[1], payload + i*lchunks, nbytes-bytes_copied);
            if (!crc_transmit(bus, subchunk, nbytes-bytes_copied+1)) return -1;
        }
    }
    return cumulative_success;
}

int crc_chunked_read(USART_TypeDef *bus, uint8_t* buf, int lchunks, int nchunks) {
    uint8_t subchunk[MAX_PAYLOAD_BYTES];
    int read = 0;
    for (int i = 0; i < nchunks; i++) {
        int size = crc_read(bus, subchunk);
        // Giving a negative value for the third argument of memcpy is an error
        if (size-1 < 0) continue;
        memcpy(buf + read, &subchunk[1], size-1);
        if (subchunk[0] == i) read += size-1;
        if (subchunk[0] >= nchunks) return -1;
    }
    return read;
}
