/*
 * A function to check wether the value of a uint64_t will fit in n bits
 * Used to check if values can fit in bitfields smaller than uint64_t
 *
 */

#ifndef LOGGERS_FITS_IN_BITS_H_
#define LOGGERS_FITS_IN_BITS_H_

#include <stdint.h>

enum {
  MASK_4_BIT = 0xF,
  MASK_12_BIT = 0xFFF,
  MASK_16_BIT = 0xFFFF,
  MASK_20_BIT = 0xFFFFF
};

uint8_t fitsInBits(uint64_t num, uint8_t n);

#endif /* LOGGERS_FITS_IN_BITS_H_ */
