#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/../.."
output="${1:-build/imu-errors}"
mkdir -p "$output"
"${CC:-cc}" -std=c11 -O1 -g -ffunction-sections -fdata-sections \
  -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable \
  -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
  -IDrivers/Device -IDrivers/CMSIS -ISrc -ISrc/inc \
  -ISrc/system_config -ISrc/tools -ISrc/peripherals -ISrc/peripherals/IMU -ISrc/ADCS \
  "${IMU_SOURCE:-Src/peripherals/IMU/ASM330LHH.c}" \
  "${VI_SOURCE:-Src/ADCS/virtual_intellisat.c}" \
  "${IMU_TEST_SOURCE:-Src/unit_tests/IMU_Tests/IMU_tests.c}" \
  Src/ADCS/adcs_math/sensors.c tests/imu_errors/test_imu.c \
  -Wl,--gc-sections -lm -o "$output/test_imu"
"$output/test_imu"
