#include "radio/Radio_intercom.h"

/*
 * Host-test stub for radio_getLastUplink().
 * For testing logic only. For launch actually hardware behavior should be verified
 */

static uint8_t last_year = 0;
static uint8_t last_month = 0;
static uint8_t last_date = 0;
static uint8_t last_hour = 0;
static uint8_t last_minute = 0;
static uint8_t last_second = 0;

void radio_stub_set_last_uplink(uint8_t year, uint8_t month, uint8_t date,
                                uint8_t hour, uint8_t minute, uint8_t second) {
    last_year = year;
    last_month = month;
    last_date = date;
    last_hour = hour;
    last_minute = minute;
    last_second = second;
}

void radio_stub_reset(void) {
    last_year = 0;
    last_month = 0;
    last_date = 0;
    last_hour = 0;
    last_minute = 0;
    last_second = 0;
}

void radio_getLastUplink(uint8_t *year, uint8_t *month, uint8_t *date,
                         uint8_t *hour, uint8_t *minute, uint8_t *second) {
    if (year)   *year   = last_year;
    if (month)  *month  = last_month;
    if (date)   *date   = last_date;
    if (hour)   *hour   = last_hour;
    if (minute) *minute = last_minute;
    if (second) *second = last_second;
}
