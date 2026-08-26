#ifndef SCHEDULER_BEACON_TESTS_STUBS_RADIO_INTERCOM_H_
#define SCHEDULER_BEACON_TESTS_STUBS_RADIO_INTERCOM_H_

#include <stdint.h>

/*
 * Host-test stub for the radio driver. Only the surface
 * build_beacon_record() touches is declared here; the full production
 * header (Src/peripherals/Radio/Radio_intercom.h) pulls in USART/CRC
 * hardware headers that don't exist on the host, so this minimal stub
 * shadows it when -Itests/stubs is on the include path.
 */

void radio_getLastUplink(uint8_t *year, uint8_t *month, uint8_t *date,
                         uint8_t *hour, uint8_t *minute, uint8_t *second);

/* Test-only helper: set what radio_getLastUplink() will report next. */
void radio_stub_set_last_uplink(uint8_t year, uint8_t month, uint8_t date,
                                uint8_t hour, uint8_t minute, uint8_t second);

/* Test-only helper: reset the stub's reported uplink to all-zero. */
void radio_stub_reset(void);

#endif /* SCHEDULER_BEACON_TESTS_STUBS_RADIO_INTERCOM_H_ */
