/*
 * core_config.h
 *
 *  - September 23, 2023
 *  	Author	: Darsh
 *  	Log		: file name change
 *
 *  - April 29, 2023
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : All necessary clock registers configured
 */

#ifndef REALOP1_CORE_CONFIG_H_
#define REALOP1_CORE_CONFIG_H_

#include "stm32l476xx.h"
#include <globals.h>

/**
 * Initializes the clocks of the micro-controller.
 *
 * This function sets up the various clock sources and
 * frequencies for the micro-controller.
 *
 * @param   None
 * @returns None
 */
void init_coreClocks();

/**
 * Enables writing access to registers powered by the Backup Domain
 * Key registers include RCC's BDRC, and several key RTC registers
 *
 * @param   None
 * @returns None
 */
void backup_domain_controlEnable();

/**
 * Disables writing access to registers powered by the Backup Domain
 * Key registers include RCC's BDRC, and several key RTC registers
 *
 * @param   None
 * @returns None
 */
void backup_domain_controlDisable();

#endif /* REALOP1_CORE_CONFIG_H_ */
