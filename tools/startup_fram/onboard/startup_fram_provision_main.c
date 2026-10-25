/*
 * startup_fram_provision_main.c
 *
 * `main` and `SystemInit` for the standalone provisioning image only. It exists
 * so the image can be linked without touching normal `Src/main.c`, `.cproject`
 * or the project's own system/clock configuration: the image reuses the real
 * Startup/ assembly, linker script lineage and drivers, and supplies just the two
 * entry symbols that the library startup expects.
 *
 * Never linked into the normal firmware: it refuses to compile without
 * STARTUP_FRAM_PROVISIONING_BUILD (checked in startup_fram_provision_boot.c too).
 */

#ifndef STARTUP_FRAM_PROVISIONING_BUILD
#error "startup_fram_provision_main.c belongs to the provisioning image only; \
define STARTUP_FRAM_PROVISIONING_BUILD"
#endif

#include <stdint.h>

#include "stm32l476xx.h"

int startup_fram_provision_boot(void);

/**
 * Minimal `SystemInit` for this image.
 *
 * The normal firmware gets its `SystemInit` from the CubeIDE-managed system file,
 * which is not part of this lane. Here the reset-state 4 MHz MSI is deliberate:
 * `startup_fram_provision_boot()` calls `init_coreClocks()` immediately, and that
 * routine programs the 80 MHz PLL. Only the FPU access bit is set early, because
 * the image is built with hardware floating point and the driver code uses it.
 */
void SystemInit(void)
{
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= (3UL << 20) | (3UL << 22); /* CP10/CP11 full access */
#endif
}

int main(void)
{
    /* The entry never returns: it resets on failure and parks on success. */
    (void)startup_fram_provision_boot();
    for (;;) {
        __WFI();
    }
}
