/*
 * fits_in_bits.c
 *
 *  Created on: Jul 26, 2023
 *      Author: dilgill
 */
#include <stdint.h>

#include "fits_in_bits.h"

// Check if num fits into n bits by checking if the value past the nth bit is > 0 (i.e. if it has any ones)
uint8_t fits_in_bits(uint64_t num, uint8_t n) {
    uint64_t res = (num >> n);
    return !(res > 0);
}
