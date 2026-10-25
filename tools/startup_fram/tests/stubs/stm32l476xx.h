#pragma once
#include <stdbool.h>
#include <stdint.h>

/*
 * Minimal register model for the host builds in this lane: just enough of the
 * CMSIS surface for Src/peripherals/FRAM/MB85RS256B.h (through SPI/spi.h) and for
 * startup_fram_provision_boot.c's clock-enable read-back and reset request.
 * Register layouts are not simulated; the device behaviour under test lives in
 * tests/fram_device_fake.c.
 */

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFR[2];
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SR;
    volatile uint32_t DR;
} SPI_TypeDef;

typedef struct {
    volatile uint32_t AHB2ENR;
} RCC_TypeDef;

extern GPIO_TypeDef fake_gpio_a;
extern GPIO_TypeDef fake_gpio_b;
extern GPIO_TypeDef fake_gpio_c;
extern SPI_TypeDef fake_spi1;
extern RCC_TypeDef fake_rcc;

#define GPIOA (&fake_gpio_a)
#define GPIOB (&fake_gpio_b)
#define GPIOC (&fake_gpio_c)
#define SPI1 (&fake_spi1)
#define RCC (&fake_rcc)

#define RCC_AHB2ENR_GPIOAEN (0x1UL << 0)
#define RCC_AHB2ENR_GPIOCEN (0x1UL << 2)
#define SPI_CR1_SPE (0x1UL << 6)

/* Requesting a reset from the provisioning image must never continue in tests. */
void fake_request_reset(void);
#define NVIC_SystemReset() fake_request_reset()

/* The success path parks the core; the host build lets the test leave that park. */
void fake_park(void);
#define __WFI() fake_park()
