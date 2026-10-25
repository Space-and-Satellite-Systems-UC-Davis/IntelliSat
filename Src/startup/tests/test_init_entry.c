/* Actual init_init() source-body model, NOT a full native/target firmware link.
 *
 * The PDB safe state lives in init_init() now, so this guards the ordering that
 * used to sit in main(): GPIOA/GPIOC clocked with a separate readback before any
 * GPIO access, the pyro commanded OFF before the fallible clock/RTC work, and no
 * ON command anywhere in the entry prefix.
 *
 * The real power_distribution.c is compiled here; only the STM32 registers and
 * the fallible init steps are modeled. This models ordering, not bus timing or
 * electrical cutoff.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "stm32l476xx.h"
#include "GPIO/gpio.h"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "init entry regression: %s (line %d)\n", #condition, __LINE__); \
    exit(1); \
} } while (0)

typedef struct { volatile uint32_t AHB2ENR; } InitRCC;
static InitRCC rcc;
GPIO_TypeDef startup_test_gpio_a, startup_test_gpio_c;
static unsigned rcc_accesses, gpio_accesses, off_commands, completed_steps;
static jmp_buf stop_init;
static int stall_at_step;

#define RCC_AHB2ENR_GPIOAEN (1u << 0)
#define RCC_AHB2ENR_GPIOCEN (1u << 2)
#define PDB_CLOCKS (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN)

static InitRCC *init_rcc(void) {
    ++rcc_accesses;
    if (rcc_accesses > 1) CHECK((rcc.AHB2ENR & PDB_CLOCKS) == PDB_CLOCKS);
    return &rcc;
}
#define RCC init_rcc()

static GPIO_TypeDef *init_gpio(GPIO_TypeDef *port) {
    CHECK((rcc.AHB2ENR & PDB_CLOCKS) == PDB_CLOCKS); /* Never touch GPIO unclocked. */
    CHECK(rcc_accesses >= 2);                        /* Enable plus separate readback. */
    ++gpio_accesses;
    return port;
}
#undef GPIOA
#undef GPIOC
#define GPIOA init_gpio(&startup_test_gpio_a)
#define GPIOC init_gpio(&startup_test_gpio_c)
#include "../../peripherals/PDB/power_distribution.c"

/* Real driver calls these; the pyro output is GPIOC pin 9. */
void gpio_low(GPIO_TypeDef *port, int pin) {
    CHECK(port == &startup_test_gpio_c && pin == 9);
    CHECK((rcc.AHB2ENR & PDB_CLOCKS) == PDB_CLOCKS);
    if (off_commands == 0) {
        /* pdb_init() clears the latch before selecting output mode. */
        CHECK((startup_test_gpio_c.MODER & (3u << 18)) != (1u << 18));
    }
    startup_test_gpio_c.ODR &= ~(1u << pin);
    ++off_commands;
}
void gpio_high(GPIO_TypeDef *port, int pin) {
    (void)port; (void)pin;
    CHECK(false); /* No ON authorized anywhere in this entry prefix. */
}

/* The fallible steps. Each one demands that the pyro is already commanded OFF,
 * then optionally abandons execution to model a stall/timeout reset. */
static void fallible_step(unsigned index) {
    CHECK(off_commands == 2); /* Real pdb_init latch clear + explicit OFF. */
    CHECK(!(startup_test_gpio_c.ODR & (1u << 9)));
    CHECK((startup_test_gpio_c.MODER & (3u << 18)) == (1u << 18));
    CHECK(gpio_accesses > 0);
    if (stall_at_step == (int)index) longjmp(stop_init, (int)index);
    CHECK(completed_steps == index);
    ++completed_steps;
}
void startup_clock_capture_boot_evidence(void) { fallible_step(0); }
void init_coreClocks(void) { fallible_step(1); }
void led_init(void) { fallible_step(2); }
void heartbeat_timer_init(void) { fallible_step(3); }
void startup_clock_init(void) { fallible_step(4); }

#include "init_init_function.inc"

static void check_init(int stall, const char *name) {
    rcc.AHB2ENR = 0;
    startup_test_gpio_a = (GPIO_TypeDef){.MODER = UINT32_MAX, .ODR = UINT32_MAX};
    startup_test_gpio_c = (GPIO_TypeDef){.MODER = UINT32_MAX, .ODR = UINT32_MAX};
    rcc_accesses = gpio_accesses = off_commands = completed_steps = 0;
    stall_at_step = stall;
    if (setjmp(stop_init) == 0) {
        init_init();
        CHECK(stall < 0); /* A modeled stall must have abandoned init_init(). */
    } else {
        CHECK(stall >= 0); /* Only the modeled stall may jump out. */
    }
    /* OFF was commanded before whichever step we stopped at, exactly twice. */
    CHECK(off_commands == 2);
    CHECK(!(startup_test_gpio_c.ODR & (1u << 9)));
    CHECK((startup_test_gpio_a.MODER & (15u << 16)) == (5u << 16));
    CHECK((rcc.AHB2ENR & PDB_CLOCKS) == PDB_CLOCKS);
    CHECK(completed_steps == (unsigned)(stall < 0 ? 5 : stall));
    printf("PASS init-entry source-body model: %s\n", name);
}

int main(void) {
    check_init(-1, "safe outputs, then every fallible step in order");
    check_init(0, "stall at boot evidence leaves the latch off");
    check_init(1, "stall in core clocks leaves the latch off");
    check_init(4, "stall in startup clock leaves the latch off");
    puts("4 init-entry source-body model scenarios passed (not full target link)");
    return 0;
}
