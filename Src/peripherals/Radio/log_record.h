/*
 * log_record.h
 *
 *  Created on: Jul 18, 2026
 */

#ifndef PERIPHERALS_RADIO_LOG_RECORD_H_
#define PERIPHERALS_RADIO_LOG_RECORD_H_

#include "ADCS/logging_records.h"
#include "RTC/rtc.h"
#include "MAG/QMC5883L.h"
#include "IMU/ASM330LHH.h"
#include "SunSensors/sun_sensors.h"

log_record_idle fill_log_idle();

#endif /* PERIPHERALS_RADIO_LOG_RECORD_H_ */
