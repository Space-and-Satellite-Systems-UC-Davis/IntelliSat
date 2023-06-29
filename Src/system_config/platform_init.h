/*
 * platform_init.h
 *
 *  - May 11, 2023 (Creation)
 *  	Author : Darsh
 *  	Log    : Created the platform initialization function
 */

#ifndef REALOP1_PLATFORM_INIT_H_
#define REALOP1_PLATFORM_INIT_H_

#include "clock_nvic_config.h"


/*
 * Configures the system's various features, such as clocks, nvic, gpio, protocol hardware, and more.
 *
 * @param None
 *
 * @returns None
 */
void init_platform();

#endif // REALOP1_PLATFORM_INIT_H_
