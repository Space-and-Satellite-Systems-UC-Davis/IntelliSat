#include "rtc.h"
#include <stdio.h>
#include <stdint.h>
#include "print_scan.h"

// Delay so no console spam on the logic analyzer
// Can adjust this value based on clock speed (maybe 1M loops ~ 100ms-ish on some configs?)
void delay_loops(volatile uint32_t count) {
    while (count--) {
        __asm("nop"); // Prevent compiler optimization (wow 154 nop)
    }
}

void testFunction_getCalendar(void) {
    // Init printer (UART/console)
    printer_init();
    printMsg("RTC Calendar Test Start: \r\n");

		// For testing, waiting 5 sec
		delay_loops(50000000);

    // Init RTC using driver, LSE (Low Speed External) as source
    // Unknown if board has no crystal, but if not can change LSE to LSI or so
    rtc_config(LSE, 1);

    // Rollover test, init to Dec 31st, 2024, 23:59:55
    // Should expect the year to change to 25 and month to 01
    printMsg("Setting time to Dec 31, 2024 - 23:59:55...\r\n");
    
    rtc_setCalendar(24, December, 31, Tuesday); 
    rtc_setTime(23, 59, 55);

    // Infinite loop, monitor time
    uint8_t y, m, d, w;
    uint8_t hh, mm, ss;

    for (;;) {
        // Current time and date
        rtc_getCalendar(&y, &m, &d, &w);
        rtc_getTime(&hh, &mm, &ss);

        // Format: 20YY-MM-DD (Day) HH:MM:SS
        printMsg("20%02u-%02u-%02u (Day=%u) %02u:%02u:%02u\r\n", y, m, d, w, hh, mm, ss);

        // Check
        if (m == January && d == 1 && ss == 0) {
            printMsg("!!! HAPPY NEW YEAR (Rollover Successful) !!!\r\n");
        }

        // Delay ~1 second for readable output
        // This value should depend on CPU clock, might have to start with 1,000,000 and adjust
        delay_loops(1000000);
    }
}
