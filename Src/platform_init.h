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
#include <DBG/dbg_config.h>

#include <core_config.h>
#include <Timers/timers.h>
#include <Buttons/buttons.h>
#include <SPI/spi.h>
#include <QSPI/qspi.h>
#include <LED/led.h>
#include <RTC/rtc.h>
#include <FRAM/FRAM.h>
#include <FRAM/MB85RS256B.h>
#include <UART/uart.h>
#include <IMU/ASM330LHH.h>
#include <MAG/QMC5883L.h>
#include <DMA/DMA.h>
#include <print_scan.h>
#include <SunSensors/sun_sensors.h>
#include <WDG/watchdog.h>

#include <PDB/power_distribution.h>
#include "startup/startup_clock.h"

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
 * Establishes the safe output state first, then the fallible clock/RTC work:
 * PDB owns GPIOA/GPIOC, so those ports are clocked and read back here before
 * any GPIO access, and the pyro command is driven OFF before init_coreClocks(),
 * led_init(), heartbeat_timer_init() or startup_clock_init() can stall or reset.
 *
 * @param   None
 * @returns None
 */
void init_init() {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN;
    (void)RCC->AHB2ENR;                              // read back before GPIO access
    pdb_init();
    pdb_pyro_off();                                  // Command OFF before fallible initialization.

    startup_clock_capture_boot_evidence();
    init_coreClocks();
    led_init();
    heartbeat_timer_init();
    startup_clock_init();
}


/* First-start waiting is implemented by Startup() in startup/startup.h on the normal boot path. */

/**
 * Configures the system's various features,
 * such as clocks, protocol hardware, and more.
 *
 * @param run_scheduler Reserved; this branch does not yet start a scheduler.
 * @returns true if the persistent boot counter was read, incremented, and verified
 */
bool init_platform(bool run_scheduler) {
    (void)run_scheduler;
    init_callbacks();

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
    //Activate GPIO G. From errata. Strange bug-fix.
	PWR->CR2 |= PWR_CR2_IOSV;

    // systick_init();
	printer_init();
	buttons_init();
    dma_initializePeripheralConstants();
    spi_config(SPI1);
    watchdog_init(300);

    /* The legacy counter increment has no status; use checked byte I/O here. */
    uint32_t count = 0;
    uint32_t readback = 0;
    if (!FRAM_readData(FRAM_BOOT_COUNTER_ADDR, (uint8_t *)&count, sizeof(count))
        || count == UINT32_MAX) return false;
    ++count;
    if (!FRAM_writeEnable()) return false;
    bool written = FRAM_writeData(FRAM_BOOT_COUNTER_ADDR, (uint8_t *)&count, sizeof(count));
    bool disabled = FRAM_writeDisable();
    return written && disabled
        && FRAM_readData(FRAM_BOOT_COUNTER_ADDR, (uint8_t *)&readback, sizeof(readback))
        && readback == count;
}

#endif // REALOP1_PLATFORM_INIT_H
