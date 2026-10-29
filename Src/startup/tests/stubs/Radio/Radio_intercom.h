#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <globals.h>
/* Src/system_config/UART/crc.h; no UART/CRC implementation is replaced. */
#define MAX_PAYLOAD_BYTES 254
#define CHUNK_LENGTH 8
typedef struct {
    size_t size;
    uint8_t datatype;
} RadioPacket;
bool radio_push(uint8_t chunk[], size_t nbytes);
bool radio_downlink(uint8_t chunk[], size_t nchunks);
void radio_init(void);
RadioPacket radio_force_pull(uint8_t chunk[], size_t capacity);
