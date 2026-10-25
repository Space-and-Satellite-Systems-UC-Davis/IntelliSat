/* Actual main source-body model, NOT a full native/target firmware link.
 * Compiles the generated verbatim main() body against boundary facades and
 * asserts what main does now: hand straight to the init chain, honour the FRAM
 * boot-counter gate, then enter Startup() and the RTOS branch.
 *
 * Every hardware entry point is poisoned, so re-adding ad-hoc initialization to
 * main() (e.g. the PDB clocks/init that now live in init_init()) fails here.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "entry regression: %s (line %d)\n", #condition, __LINE__); \
    exit(1); \
} } while (0)

static unsigned calls;
static bool platform_run_scheduler_arg;
static bool fram_result;
static unsigned wfi_calls;
static jmp_buf stop_main;

/* Nothing in this list may be reachable from main(): initialization belongs to
 * init_init()/init_platform(), not the entry point. */
static void hardware_setup_reached(void) { CHECK(false); }
#define RCC hardware_setup_reached()
void pdb_init(void) { hardware_setup_reached(); }
void pdb_pyro_off(void) { hardware_setup_reached(); }
void pdb_pyro_on(void) { hardware_setup_reached(); }
void gpio_low(void *port, int pin) { (void)port; (void)pin; hardware_setup_reached(); }
void gpio_high(void *port, int pin) { (void)port; (void)pin; hardware_setup_reached(); }

/* The init chain, in order. main() may only call these, and init_init() first. */
static void init_init(void) {
    CHECK(calls == 0); /* Nothing at all may precede the init chain. */
    ++calls;
}
static bool init_platform(bool run_scheduler) {
    CHECK(calls == 1);
    platform_run_scheduler_arg = run_scheduler;
    ++calls;
    return fram_result;
}

/* Gate: a failed boot counter never reaches Startup(). */
static void printMsg(const char *message) {
    CHECK(calls == 2);
    CHECK(message != NULL);
    ++calls;
}
static void __WFI(void) {
    CHECK(calls == 3);
    if (++wfi_calls == 3) longjmp(stop_main, 1); /* Idles forever, never continues. */
}

/* Normal path. */
static void Startup(void) {
    CHECK(calls == 2);
    ++calls;
}
static int branch_main(void) {
    CHECK(calls == 3);
    ++calls;
    longjmp(stop_main, 2);
    return 0;
}

#define main startup_entry
#include "main_function.inc"
#undef main

static void check_entry(bool ready, int reason, const char *name) {
    fram_result = ready;
    calls = wfi_calls = 0;
    platform_run_scheduler_arg = false; /* main() must pass !RUN_TEST == true. */
    int jumped = setjmp(stop_main);
    if (jumped == 0) {
        startup_entry();
        CHECK(false); /* startup_entry() must not fall through. */
    }
    CHECK(jumped == reason);
    CHECK(platform_run_scheduler_arg);
    if (ready) {
        CHECK(calls == 4); /* init_init, init_platform, Startup, branch_main. */
        CHECK(wfi_calls == 0);
    } else {
        CHECK(calls == 3); /* init_init, init_platform, printMsg, then idling. */
        CHECK(wfi_calls == 3);
    }
    printf("PASS entry source-body model: %s\n", name);
}
int main(void) {
    check_entry(true, 2, "init chain first, then Startup and the RTOS branch");
    check_entry(false, 1, "failed FRAM boot counter idles instead of starting up");
    check_entry(true, 2, "main performs no hardware setup of its own");
    puts("3 entry source-body model scenarios passed (not full target link)");
    return 0;
}
