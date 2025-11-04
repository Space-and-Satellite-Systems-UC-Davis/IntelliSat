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

#include <globals.h>

#include <core_config.h>
#include <Timers/timers.h>
#include <Buttons/buttons.h>
#include <SPI/spi.h>
#include <QSPI/qspi.h>
#include <LED/led.h>
#include <RTC/rtc.h>
#include <UART/uart.h>
#include <IMU/ASM330LHH.h>
#include <MAG/QMC5883L.h>
#include <print_scan.h>
#include <SunSensors/sun_sensors.h>
#include <WDG/watchdog.h>

#define SCB_CPACR_CP10_POS 20U
#define SCB_CPACR_CP11_POS 22U

enum scb_cpacr_cpn_privileges {
    SCB_CPACR_CPN_ACCESS_DENIED,
    SCB_CPACR_CPN_PRIVILEGED_ONLY,
    SCB_CPACR_CPN__RESERVED,
    SCB_CPACR_CPN_FULL_ACCESS
};

/**
 * Configures only what is necessary to boot.
 *
 * @param   None
 * @returns None
 */
void init_init() {
	init_coreClocks();
	rtc_config(LSI, 0);
    //TODO: retrieve RTC vars
    //   and set scheduler flags?
}


/**
 * Performs first-time-startup behaviors.
 *
 * @param   None
 * @returns None
 */
void init_first_time() {
    timer_waitStartupTime();
    //TODO: open antenna via burnwire.
    //TODO: initialize flash headers.
    //TODO: set first_time flag to on.
}


/**
 * Configures the system's various features,
 * such as clocks, protocol hardware, and more.
 * @returns None
 */
void init_platform() {

    SCB->CPACR |= (SCB_CPACR_CPN_FULL_ACCESS << SCB_CPACR_CP10_POS
    | SCB_CPACR_CPN_FULL_ACCESS << SCB_CPACR_CP11_POS); // Enable the Floating-Point Unit for full access
    debug_init();
    set_IMU(IMU0);
	  imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g, IMU_ODR_3333_Hz, IMU_FS_1000_dps);
    set_IMU(IMU1);
    imu_init(IMU_ODR_3333_Hz, IMU_FS_8_g, IMU_ODR_3333_Hz, IMU_FS_500_dps);


	mag_init(MAG_ODR_200_Hz, MAG_FS_8_G, MAG_OVERSAMPLE_512);
    sun_sensor_init();
    //TODO: hdd_init().
    //TODO: initialize intercom.
    //TODO: fetch flash header.
    //TODO: increment boot counter.
    //Activate GPIO G. From errata. Strange bug-fix.
	PWR->CR2 |= PWR_CR2_IOSV;

    systick_init(true);
	printer_init();
	led_init();
	buttons_init();
    watchdog_init(WWDG_TIMEOUT_TIME);
    heartbeat_timer_init();
}

#endif // REALOP1_PLATFORM_INIT_H
