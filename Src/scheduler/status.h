#ifndef STATUS_H
#define STATUS_H
#pragma once

#include "schedulerGlobals.h"

/* Bitfield manipulation */
#define SET_BIT(BF, N) BF |= ((uint16_t) 0x01 << N)
#define CLR_BIT(BF, N) BF &= ~((uint16_t) 0x01 << N)
#define IS_BIT_SET(BF, N) ((BF >> N) & 0x1)

/* Variable to store mode bits */
// extern volatile uint16_t flagBits;

// enum bits{
//     CHARGING, DETUMBLE, COMMS, ECC, HDD, MRW,
//     START, ANTENNA, COILS, HDD_STATUS, MRW_STATUS
// };

struct operation_bits_t {
	uint8_t mode_bits;
	uint8_t status_bits;
};

extern volatile struct operation_bits_t flag_bits;

enum modes_t {
	LOW_PWR, DETUMBLE, COMMS, ECC, EXPERIMENT, IDLE
};

enum status_t {
	START, ANTENNA, COILS, HDD_STATUS, MRW_STATUS
};

/* 
	uint16_t flagBits:
Mode:
	Charging     0		Does the satellite need to charge?
	Detumble     1		Does the satellite need to detumble?
	Comms        2		Is there a downlink request?
	ECC          3		Is it time to perform ECC?
	HDD          4		Is an HDD test ready to run?
	MRW          5		Is an MRW test ready to run?

Status:
	Start        6		Have we completed the 30 minute timer?
	Antenna      7		Has antenna been deployed?
	Coils        8		Is the coils circuit communicating?
	HDD_status   9		Is the HDD ESC communicating?
	MRW_status   10		Is the MRW ESC communicating?
*/

/* 
	struct OperationBits flagBits:
uint_8_t modeBits:
	LowPower     0		Does the satellite need to enter low power?
	Detumble     1		Does the satellite need to detumble?
	Comms        2		Is there a downlink request?
	ECC          3		Is it time to perform ECC?
	Experiment	 4		Is an experiment ready to run?
	IDLE		 5		Idle when no other mode is running

uint_8_t statusBits:
	Start        0		Have we completed the 30 minute timer?
	Antenna      1		Has antenna been deployed?
	Coils        2		Is the coils circuit communicating?
	HDD_status   3		Is the HDD ESC communicating?
	MRW_status   4		Is the MRW ESC communicating?
*/

void status_check();

#endif//STATUS_H
