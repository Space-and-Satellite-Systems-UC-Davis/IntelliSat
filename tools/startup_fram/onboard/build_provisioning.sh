#!/bin/sh
#
# Build the standalone Startup FRAM provisioning image (ELF + raw binary).
#
# It reuses the project's own Startup/ assembly, its linker script (with the
# documented .noinit addition in provisioning_STM32L476ZGTX_FLASH.ld) and the
# real drivers, and adds only the provisioning entry sources. Normal
# Src/main.c, .cproject and the normal firmware are not touched, and this image
# is never linked into the normal firmware.
#
# Toolchain: the exact GNU Arm 13.3.1 that ships with STM32CubeIDE. Override with
# ARM_GCC=/path/to/arm-none-eabi-gcc if it lives elsewhere.
#
# Image content: PROVISION_IMAGE_ARGS overrides the generator arguments, e.g.
#   PROVISION_IMAGE_ARGS="--boot-count 0 --adcs-sensors Sun0,Mag1 --armed" ./build_provisioning.sh
# Boot count and ADCS selection must be supplied explicitly. There is no default
# image. Omitting --armed produces an unarmed image.
#
# No flashing, probing or debug-server launch happens here: the script only
# compiles and links local files.
#
# Usage:
#   ./build_provisioning.sh                      build the image
#   ./build_provisioning.sh --refresh-linker-script   regenerate the derived .ld
#   ./build_provisioning.sh --clean

set -eu

script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
lane_root=$(CDPATH= cd -- "$script_dir/../../.." && pwd)

CUBEIDE_TOOLS="/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.macos64_1.0.100.202509120712/tools/bin"
ARM_GCC=${ARM_GCC:-$CUBEIDE_TOOLS/arm-none-eabi-gcc}
ARM_SIZE=${ARM_SIZE:-$(dirname -- "$ARM_GCC")/arm-none-eabi-size}
ARM_NM=${ARM_NM:-$(dirname -- "$ARM_GCC")/arm-none-eabi-nm}
ARM_OBJCOPY=${ARM_OBJCOPY:-$(dirname -- "$ARM_GCC")/arm-none-eabi-objcopy}

BUILD_DIR=${BUILD_DIR:-$script_dir/build}
IMAGE_HEADER=$BUILD_DIR/startup_fram_image.h
PROJECT_LD=$lane_root/STM32L476ZGTX_FLASH.ld
IMAGE_LD=$script_dir/provisioning_STM32L476ZGTX_FLASH.ld
PROVISION_IMAGE_ARGS=${PROVISION_IMAGE_ARGS:-}
PYTHON=${PYTHON:-python3}

SOURCES="
$lane_root/Startup/startup_stm32l476zgtx.s
$lane_root/Src/system_config/core_config.c
$lane_root/Src/system_config/Timers/heartbeat_timer.c
$lane_root/Src/system_config/SPI/spi.c
$lane_root/Src/system_config/GPIO/gpio.c
$lane_root/Src/system_config/LED/led.c
$lane_root/Src/peripherals/PDB/power_distribution.c
$lane_root/Src/peripherals/FRAM/MB85RS256B.c
$lane_root/Src/tools/Utils.c
$script_dir/startup_fram_provision.c
$script_dir/startup_fram_provision_boot.c
$script_dir/startup_fram_provision_main.c
"

refresh_linker_script() {
    echo "REFRESH $IMAGE_LD"
    "$PYTHON" - "$PROJECT_LD" "$IMAGE_LD" <<'PY'
import pathlib
import sys

project = pathlib.Path(sys.argv[1]).read_text()
marker = ('  /* User_heap_stack section, used to check that there is enough "RAM" Ram'
          '  type memory left */')
if marker not in project:
    sys.exit("anchor not found in the project linker script")
addition = '''  /* PROVISIONING-IMAGE ADDITION: the bounded inspection report is NOLOAD and
   * outside .bss, so it survives the failure path's NVIC_SystemReset() and stays
   * readable by a debugger while the image reset-loops. Nothing else in this
   * script differs from STM32L476ZGTX_FLASH.ld at the repository root. */
  .noinit (NOLOAD) :
  {
    . = ALIGN(4);
    _snoinit = .;
    KEEP(*(.noinit))
    KEEP(*(.noinit*))
    . = ALIGN(4);
    _enoinit = .;
  } >RAM

'''
header = '''/*
 * Derived linker script for the standalone Startup FRAM provisioning image.
 *
 * Copied from /STM32L476ZGTX_FLASH.ld (the project's own script) with exactly one
 * addition: a `.noinit (NOLOAD)` output section holding the debugger-visible
 * `startup_fram_provision_report`. Everything else, including the memory map,
 * stack/heap reservation and the /DISCARD/ rules, is identical to the project
 * script.
 */

'''
pathlib.Path(sys.argv[2]).write_text(header + project.replace(marker, addition + marker, 1))
print("ok")
PY
    diff "$PROJECT_LD" "$IMAGE_LD" >/dev/null && {
        echo "ERROR: derived linker script is identical to the project script" >&2
        exit 1
    }
    echo "OK: derived linker script differs only by the .noinit addition"
}

case "${1:-}" in
--refresh-linker-script)
    refresh_linker_script
    exit 0
    ;;
--clean)
    rm -rf "$BUILD_DIR"
    echo "cleaned $BUILD_DIR"
    exit 0
    ;;
"")
    ;;
*)
    echo "usage: $0 [--refresh-linker-script|--clean]" >&2
    exit 2
    ;;
esac

if [ -z "$PROVISION_IMAGE_ARGS" ]; then
    echo "ERROR: set PROVISION_IMAGE_ARGS with explicit --boot-count and --adcs-mask or --adcs-sensors" >&2
    exit 2
fi

echo "== toolchain =="
if [ ! -x "$ARM_GCC" ]; then
    echo "ERROR: arm-none-eabi-gcc not found at $ARM_GCC" >&2
    exit 1
fi
compiler_version=$("$ARM_GCC" --version)
printf '%s\n' "$compiler_version" | head -1
printf '%s\n' "$compiler_version" | grep -q '13\.3\.1' || {
    echo "ERROR: this build requires GNU Arm 13.3.1" >&2
    exit 1
}
echo

echo "== image =="
mkdir -p "$BUILD_DIR"
# The build owns these artefacts and replaces them; the generator itself keeps
# exclusive creation so an operator cannot clobber a hand-written image by hand.
rm -f "$IMAGE_HEADER" "$BUILD_DIR/startup_fram_image.bin"
"$PYTHON" "$lane_root/tools/startup_fram/startup_fram.py" generate \
    $PROVISION_IMAGE_ARGS \
    --header-symbol STARTUP_FRAM_PROVISION_IMAGE \
    --header "$IMAGE_HEADER" \
    --binary "$BUILD_DIR/startup_fram_image.bin"
echo

COMMON_FLAGS="-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -std=c11 \
-O2 -g3 -ffunction-sections -fdata-sections -fno-common \
-DSTARTUP_FRAM_PROVISIONING_BUILD \
-I$lane_root/Drivers/Device -I$lane_root/Src/inc -I$lane_root/Src/peripherals -I$lane_root/Src/peripherals/FRAM \
-I$lane_root/Src/system_config -I$lane_root/Src/tools -I$script_dir -I$BUILD_DIR"

echo "== compile =="
objects=""
for source in $SOURCES; do
    object=$BUILD_DIR/$(basename "$source" | sed 's/\.[cs]$//').o
    case "$source" in
    *.s) extra="-x assembler-with-cpp" ;;
    *) extra="" ;;
    esac
    echo "CC $(basename "$source")"
    # -Werror for this lane's own sources; the project sources are compiled as-is.
    case "$source" in
    "$script_dir"/*) warnings="-Wall -Wextra -Werror" ;;
    *) warnings="-Wall -Wextra" ;;
    esac
    # shellcheck disable=SC2086
    "$ARM_GCC" $COMMON_FLAGS $extra $warnings -c "$source" -o "$object"
    objects="$objects $object"
done
echo

echo "== link =="
ELF=$BUILD_DIR/startup_fram_provision.elf
MAP=$BUILD_DIR/startup_fram_provision.map
# shellcheck disable=SC2086
"$ARM_GCC" $COMMON_FLAGS -T"$IMAGE_LD" -Wl,--gc-sections -Wl,-Map="$MAP" \
    -Wl,--print-memory-usage -Wl,--no-warn-rwx-segments \
    --specs=nano.specs --specs=nosys.specs -static \
    $objects -o "$ELF"
echo

echo "== artefacts =="
"$ARM_OBJCOPY" -O binary "$ELF" "$BUILD_DIR/startup_fram_provision.bin"
"$ARM_SIZE" "$ELF"
echo
echo "ELF   $ELF"
echo "BIN   $BUILD_DIR/startup_fram_provision.bin"
echo "MAP   $MAP"
echo "HDR   $IMAGE_HEADER"
echo
echo "== report symbol (for the GDB dump in tools/startup_fram/README.md) =="
"$ARM_NM" -S --defined-only "$ELF" \
    | grep -E "startup_fram_provision_report|_snoinit|_enoinit|_sbss|_ebss|_estack" || true
echo

echo "== checks =="
symbols=$("$ARM_NM" -S --defined-only "$ELF")
report_line=$(printf '%s\n' "$symbols" | grep " startup_fram_provision_report$")
report_addr=$(printf '%s\n' "$report_line" | awk '{print $1}')
report_size=$(printf '%s\n' "$report_line" | awk '{print $2}')
# $NF because nm prints a size column only for symbols that have one.
ebss=$(printf '%s\n' "$symbols" | awk '$NF == "_ebss" {print $1}')
enoinit=$(printf '%s\n' "$symbols" | awk '$NF == "_enoinit" {print $1}')
snoinit=$(printf '%s\n' "$symbols" | awk '$NF == "_snoinit" {print $1}')
if [ -z "$report_addr" ] || [ -z "$ebss" ] || [ -z "$snoinit" ] || [ -z "$enoinit" ]; then
    echo "FAIL: report/ .noinit/ .bss symbols missing from the ELF" >&2
    exit 1
fi
if [ "$((0x$report_addr))" -lt "$((0x$ebss))" ]; then
    echo "FAIL: report at 0x$report_addr is inside .bss (zeroed every boot)" >&2
    exit 1
fi
if [ "$((0x$snoinit))" != "$((0x$report_addr))" ] || \
   [ "$((0x$enoinit))" != "$((0x$report_addr + 0x$report_size))" ]; then
    echo "FAIL: report is not the only .noinit object" >&2
    exit 1
fi
echo "OK: report 0x$report_addr (+0x$report_size) is outside .bss (ends 0x$ebss) and reset-persistent"

if "$ARM_NM" "$ELF" | grep -qE " (init_init|init_platform|vTaskStartScheduler|Startup)$"; then
    echo "FAIL: normal-boot symbols are linked into the provisioning image" >&2
    exit 1
fi
echo "OK: no normal-boot symbols (init_init/init_platform/Startup/FreeRTOS) in the image"
echo
echo "BUILD OK (exit 0)"
