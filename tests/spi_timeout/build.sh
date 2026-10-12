#!/usr/bin/env bash
set -eu
cd "$(dirname "$0")/../.."
output="${1:-build/spi-timeout}"
mkdir -p "$output"
"${CC:-arm-none-eabi-gcc}" -mcpu=cortex-m4 -mthumb -O1 -g \
  -ffunction-sections -fdata-sections -Wall -Wextra -Wno-unused-parameter \
  -IDrivers/Device -IDrivers/CMSIS -ISrc/inc -ISrc/system_config -ISrc/tools \
  -ISrc/system_config/SPI \
  "${SPI_SOURCE:-Src/system_config/SPI/spi.c}" Src/tools/Utils.c \
  tests/spi_timeout/clock.c -nostdlib -Wl,--gc-sections \
  -Wl,-e,spi_transmitReceive -Wl,-Ttext=0x08000000 \
  -o "$output/spi.elf"
