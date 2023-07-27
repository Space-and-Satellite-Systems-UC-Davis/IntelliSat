/*
 * A function to check wether the value of a uint64_t will fit in n bits
 * Used to check if values can fit in bitfields smaller than uint64_t
 *
 */

#ifndef LOGGERS_FITS_IN_BITS_H_
#define LOGGERS_FITS_IN_BITS_H_

uint8_t fits_in_bits(uint64_t num, uint8_t n);

#endif /* LOGGERS_FITS_IN_BITS_H_ */
