/*
 * platform_init.h

 * 	- May 22, 2023
 * 		Author			: Darsh
 * 		Contributors	: Parteek
 * 		Log				: Backup Power, RTC Init
 *
 * 	- May 11, 2023
 * 		Author	: Darsh
 * 		Log		: Created the platform_init() function
 */

#ifndef REALOP1_PLATFORM_INIT_H
#define REALOP1_PLATFORM_INIT_H

#include "globals.h"

#include "system_config/core_config.h"
#include "system_config/Timers/timers.h"
#include "system_config/Buttons/buttons.h"
#include "system_config/SPI/spi.h"
#include "system_config/QSPI/qspi.h"
#include "system_config/LED/led.h"
#include "system_config/RTC/rtc.h"
#include "system_config/UART/uart.h"
#include "peripherals/IMU/ASM330LHH.h"
#include "peripherals/MAG/QMC5883L.h"
#include "tools/print_scan.h"

/**
 * Configures the system's various features,
 * such as clocks, protocol hardware, and more.
 *
 * @param   None
 * @returns None
 */
void init_platform() {
	init_coreClocks();
	PWR->CR2 |= PWR_CR2_IOSV;

	led_init();
	buttons_init();

	rtc_config(LSI, 0);
	mag_init(MAG_ODR_200_Hz, MAG_FS_8_G, MAG_OVERSAMPLE_512);
	imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);

	printer_init();
	systick_init();
}

#endif // REALOP1_PLATFORM_INIT_H
