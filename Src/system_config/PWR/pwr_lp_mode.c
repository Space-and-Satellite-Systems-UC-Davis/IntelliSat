#include "pwr_lp_mode.h"
#include "sleep_primitives.h"
#include "peripherals/IMU/ASM330LHH.h"
#include "peripherals/MAG/QMC5883L.h"
#include "peripherals/PDB/power_distribution.h"
#include "FreeRTOS.h"
#include "task.h"

volatile bool g_system_lp_mode_active = false;

void pwr_enterSystemLPMode(void) {
    taskENTER_CRITICAL();

    pdb_hdd_off();
    pdb_mgt_off();
    imu_powerDown();
    mag_powerDown();
    PWR_enterLPRunMode();
    g_system_lp_mode_active = true;

    taskEXIT_CRITICAL();
}

void pwr_exitSystemLPMode(void) {
    taskENTER_CRITICAL();

    /* Exit LP Run first — restores 80 MHz so TIM7 and wait_with_timeout
     * are correct before any peripheral re-initialisation. */
    PWR_exitLPRunMode();

    /* OP_REV 3 defaults matching platform_init.h */
    set_IMU(IMU0);
    imu_init(IMU_ODR_3333_Hz, IMU_FS_2_g,  IMU_ODR_3333_Hz, IMU_FS_1000_dps);
    set_IMU(IMU1);
    imu_init(IMU_ODR_3333_Hz, IMU_FS_8_g,  IMU_ODR_3333_Hz, IMU_FS_500_dps);
    mag_init(MAG_ODR_200_Hz,  MAG_FS_8_G,  MAG_OVERSAMPLE_512);

    pdb_hdd_on();
    pdb_mgt_on();
    g_system_lp_mode_active = false;

    taskEXIT_CRITICAL();
}
