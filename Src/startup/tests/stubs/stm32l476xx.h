#pragma once
#include <stdint.h>
/* PDB register-order model only, not an STM32 peripheral simulation. */
typedef struct { uint32_t MODER, ODR; } GPIO_TypeDef;
extern GPIO_TypeDef startup_test_gpio_a, startup_test_gpio_c;
#define GPIOA (&startup_test_gpio_a)
#define GPIOC (&startup_test_gpio_c)

/*
 * RTC/RCC bit definitions used by the Startup interval clock. Values mirror
 * Drivers/Device/stm32l476xx.h; the register words themselves are modeled in
 * stubs/RTC/rtc.h.
 */
#define RTC_TR_PM_Msk (0x1UL << 22)
#define RTC_TR_HT_Pos 20U
#define RTC_TR_HT_Msk (0x3UL << RTC_TR_HT_Pos)
#define RTC_TR_HU_Pos 16U
#define RTC_TR_HU_Msk (0xFUL << RTC_TR_HU_Pos)
#define RTC_TR_MNT_Pos 12U
#define RTC_TR_MNT_Msk (0x7UL << RTC_TR_MNT_Pos)
#define RTC_TR_MNU_Pos 8U
#define RTC_TR_MNU_Msk (0xFUL << RTC_TR_MNU_Pos)
#define RTC_TR_ST_Pos 4U
#define RTC_TR_ST_Msk (0x7UL << RTC_TR_ST_Pos)
#define RTC_TR_SU_Pos 0U
#define RTC_TR_SU_Msk (0xFUL << RTC_TR_SU_Pos)

#define RTC_DR_YT_Pos 20U
#define RTC_DR_YT_Msk (0xFUL << RTC_DR_YT_Pos)
#define RTC_DR_YU_Pos 16U
#define RTC_DR_YU_Msk (0xFUL << RTC_DR_YU_Pos)
#define RTC_DR_WDU_Pos 13U
#define RTC_DR_WDU_Msk (0x7UL << RTC_DR_WDU_Pos)
#define RTC_DR_MT_Pos 12U
#define RTC_DR_MT_Msk (0x1UL << RTC_DR_MT_Pos)
#define RTC_DR_MU_Pos 8U
#define RTC_DR_MU_Msk (0xFUL << RTC_DR_MU_Pos)
#define RTC_DR_DT_Pos 4U
#define RTC_DR_DT_Msk (0x3UL << RTC_DR_DT_Pos)
#define RTC_DR_DU_Pos 0U
#define RTC_DR_DU_Msk (0xFUL << RTC_DR_DU_Pos)

#define RTC_CR_REFCKON (0x1UL << 4)
#define RTC_CR_FMT (0x1UL << 6)
#define RTC_CR_BYPSHAD (0x1UL << 5)

#define RTC_CALR_CALP_Msk (0x1UL << 15)
#define RTC_CALR_CALW8_Msk (0x1UL << 14)
#define RTC_CALR_CALW16_Msk (0x1UL << 13)
#define RTC_CALR_CALM_Msk (0x1FFUL << 0)

#define RTC_ISR_RECALPF (0x1UL << 16)
#define RTC_ISR_INIT (0x1UL << 7)
#define RTC_ISR_INITF (0x1UL << 6)
#define RTC_ISR_RSF (0x1UL << 5)
#define RTC_ISR_INITS (0x1UL << 4)
#define RTC_ISR_SHPF (0x1UL << 3)

#define RTC_PRER_PREDIV_A_Pos 16U
#define RTC_PRER_PREDIV_A_Msk (0x7FUL << RTC_PRER_PREDIV_A_Pos)
#define RTC_PRER_PREDIV_S_Pos 0U
#define RTC_PRER_PREDIV_S_Msk (0x7FFFUL << RTC_PRER_PREDIV_S_Pos)

#define RTC_SSR_SS_Msk 0xFFFFUL

#define RCC_BDCR_LSEON (0x1UL << 0)
#define RCC_BDCR_LSERDY (0x1UL << 1)
#define RCC_BDCR_LSECSSD (0x1UL << 6)
#define RCC_BDCR_RTCSEL_Pos 8U
#define RCC_BDCR_RTCSEL_Msk (0x3UL << RCC_BDCR_RTCSEL_Pos)
#define RCC_BDCR_RTCSEL_0 (0x1UL << RCC_BDCR_RTCSEL_Pos)
#define RCC_BDCR_RTCSEL_1 (0x2UL << RCC_BDCR_RTCSEL_Pos)
#define RCC_BDCR_RTCEN (0x1UL << 15)
#define RCC_BDCR_BDRST (0x1UL << 16)

#define RCC_CSR_RMVF (0x1UL << 23)
#define RCC_CSR_FWRSTF (0x1UL << 24)
#define RCC_CSR_OBLRSTF (0x1UL << 25)
#define RCC_CSR_PINRSTF (0x1UL << 26)
#define RCC_CSR_BORRSTF (0x1UL << 27)
#define RCC_CSR_SFTRSTF (0x1UL << 28)
#define RCC_CSR_IWDGRSTF (0x1UL << 29)
#define RCC_CSR_WWDGRSTF (0x1UL << 30)
#define RCC_CSR_LPWRRSTF (0x1UL << 31)
