# Startup FRAM provisioning tools (`tools/startup_fram`)

Host generator/inspector, a checked onboarding writer, and a reproducible
**standalone** provisioning firmware image (ELF + raw binary) for the 48-byte
Startup persistence record (format tag `SSP1`).

Everything here is offline: the host tool only reads and writes local files, and
the image only exists after you deliberately build and flash it. No command in
this directory opens a probe, serial port or USB device, flashes anything, or
starts a debug server.

Authorities: `SPEC.md` ("Exact minimal wire contract") and the independent vector
`CONTRACT-VECTOR.json`. The test suites do **not** read those files; they use the
committed literal copy in `tests/contract_fixture.{h,py}` (see "Tests").

## 1. What this does and does not do

* Generates the fresh 48-byte image (and a matching C header) with an explicit
  boot counter and an explicit ADCS selection, optionally armed.
* Decodes/validates any 48-byte image or FRAM read-back and explains problems.
* Writes that image to a real device **only** from the separate provisioning
  firmware image, which never runs Startup, the scheduler, or any burn.
* Does **not** change the ADCS format, add a persistence framework, or touch
  normal boot code. The ADCS mask keeps the existing `SensorOffset` layout
  (bits 0..20); bits 21..31 stay reserved zero. No new framework, no second state
  machine, no RTC continuity.

## 2. Wire layout (byte addresses on the FRAM device)

| Address | Bytes | Meaning |
|---|---:|---|
| 0..3 | 4 | boot counter, uint32 LE |
| 4..7 | 4 | ADCS sensor enable mask, uint32 LE (bits 0..20 used, 21..31 reserved zero) |
| 8..27 | 20 | Startup slot A |
| 28..47 | 20 | Startup slot B |

Each slot (offsets relative to the slot start):

| Offset | Bytes | Meaning |
|---|---:|---|
| 0 | 4 | ASCII `SSP1` |
| 4 | 4 | save sequence, uint32 LE; fresh image = 1, `0xFFFFFFFF` is the last legally **publishable/loadable** record (only the next save reports exhaustion; the sequence never wraps) |
| 8 | 1 | resume state, existing `MCUState` numeric value (0,1,2,3,6 persist; 4,5,7 transient) |
| 9 | 1 | attempts used, 0..7 (saturates) |
| 10 | 1 | flags, bit0 = armed, all other bits zero |
| 11 | 4 | remaining wait, uint32 LE |
| 15 | 4 | CRC32 of slot bytes 0..14 (reflected `0xEDB88320`, zlib compatible) |
| 19 | 1 | commit marker `0xA5` (`0x00` = uncommitted/invalid) |

**Absolute addresses of the two commit markers are 27 (slot A) and 47 (slot B).**
Byte 19 above is the slot-relative offset. Addresses 48..32767 are never written
by these tools.

Field rules: sequence 1..`0xFFFFFFFF` (0 is never valid), attempts 0..7, flags
bit0 only, `INITIAL_CHECKS` ⇒ attempts 0 and remaining ≤ 1800000 ms,
`WAIT_POWER_ON` ⇒ remaining 0, `WAITING_FOR_UPLINK` ⇒ attempts 1..7 and remaining
≤ the interval derived from the count (172800000 ms while consumed <7, else
604800000 ms), `SAT_TO_GROUND_1`/`MAIN_OPS` ⇒ remaining 0, every non-initial
state ⇒ armed. Loader rules: at least one committed slot, higher sequence wins,
equal sequences must be byte-identical.

## 3. Host generator / inspector

```bash
# unarmed factory image + matching C header (default: no arming)
python3 tools/startup_fram/startup_fram.py generate \
    --boot-count 0 --adcs-sensors Sun0,Sun1,Mag1 \
    --binary build/startup_image.bin --header build/startup_fram_image.h \
    --header-symbol STARTUP_FRAM_PROVISION_IMAGE

# explicit mask, deliberately armed
python3 tools/startup_fram/startup_fram.py generate \
    --boot-count 0 --adcs-mask 0x00000000 --armed --binary build/startup_armed.bin

# decode a generated image or a FRAM read-back
python3 tools/startup_fram/startup_fram.py inspect build/startup_image.bin [--json]
```

* `--boot-count` and exactly one of `--adcs-mask` / `--adcs-sensors` are
  mandatory; no production default is guessed. `--adcs-mask` accepts
  `0x00000000`..`0x001FFFFF`; bits 21..31 are rejected.
* `--adcs-sensors` takes `SensorOffset` names: `Sun0`..`Sun11`, `Coil0`..`Coil2`,
  `Imu0`, `Imu1`, `Mag0`, `Mag1`, `Hdd0`, `Hdd1`.
* `--armed` is opt-in. Without it the record is unarmed, and the normal firmware
  never deploys from an unarmed record.
* Outputs are created exclusively; `--overwrite` replaces them. If any requested
  output already exists nothing is written.
* `inspect` exits 1 for anything that is not a complete valid image, prints
  per-slot defects, the loader verdict (both slots / one usable slot / unusable),
  and warns when the image is armed.
* Exit codes: `0` success, `1` contract/validation error, `2` usage error.

## 4. The onboard writer (real device writes)

`onboard/startup_fram_provision.c` (API in `onboard/startup_fram_provision.h`) is
the only code that writes the device. It uses the existing checked low-level
driver calls (`FRAM_writeEnable`, `FRAM_writeData`, `FRAM_writeDisable`,
`FRAM_readData`), checks every return value and every read-back, and never writes
above address 47.

Order, each step verified before the next:

1. Validate the 48-byte image (tag, CRC, marker, field ranges, equal-sequence
   rule, boot counter, reserved ADCS bits). On any defect return
   `STARTUP_FRAM_PROVISION_IMAGE_REJECTED` **without touching the device**.
2. Invalidate **both commit markers (absolute addresses 27 and 47 → `0x00`)** and
   verify them, before anything can look committed.
3. Write boot counter + ADCS mask + slot A payload (0..26), then slot B payload
   (28..46); read both ranges back and compare.
4. Publish each commit marker (`0xA5` at 27, then at 47) last, verifying each.
5. Read back the whole 0..47 image and compare byte for byte.

Status values: `STARTUP_FRAM_PROVISION_OK`, `_IMAGE_REJECTED`,
`_INVALIDATE_FAILED`, `_WRITE_FAILED`, `_COMMIT_FAILED`, `_VERIFY_FAILED`.

Interrupted-update model: the store recovers from a cut at any byte because a
marker is only published after its payload verified, so every committed slot is
either the previous complete record or the complete new one. A 48-byte *factory
initialization as a whole* is not atomic: if power is lost during ground
provisioning, re-provision with the deployment harness disconnected.

## 5. Standalone provisioning image (build + link)

```bash
# Example only: deliberately select boot 0 and no enabled ADCS bits; unarmed.
PROVISION_IMAGE_ARGS="--boot-count 0 --adcs-mask 0" \
    tools/startup_fram/onboard/build_provisioning.sh         # ELF + bin + map
tools/startup_fram/onboard/build_provisioning.sh --clean
tools/startup_fram/onboard/build_provisioning.sh --refresh-linker-script
PROVISION_IMAGE_ARGS="--boot-count 0 --adcs-sensors Sun0,Mag1 --armed" \
    tools/startup_fram/onboard/build_provisioning.sh
# equivalent from the test Makefile:
PROVISION_IMAGE_ARGS="--boot-count 0 --adcs-mask 0" \
    make -C tools/startup_fram/tests -f Makefile provisioning-image
```

There is no default provisioning image: missing explicit `PROVISION_IMAGE_ARGS`
is an error before output creation. Match the arguments to the separately inspected
image and compare the embedded `onboard/build/startup_fram_image.bin` byte-for-byte
before any operator flashing. The example zero mask above is not a production default.

It requires **exact GNU Arm 13.3.1** from STM32CubeIDE (override with
`ARM_GCC=/path/to/arm-none-eabi-gcc`):

```
/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.macos64_1.0.100.202509120712/tools/bin/arm-none-eabi-gcc
```

Sources linked (the project's own files, no copies):

| File | Why |
|---|---|
| `Startup/startup_stm32l476zgtx.s` | reset handler / vector table of the project |
| `Src/system_config/core_config.c` | `init_coreClocks()` (80 MHz PLL) |
| `Src/system_config/Timers/heartbeat_timer.c` | TIM7 1 ms tick behind `getSysTime()` |
| `Src/system_config/LED/led.c` | `led_init()`; the real TIM7 ISR calls `blinky()` |
| `Src/system_config/SPI/spi.c`, `GPIO/gpio.c` | SPI1 + pin control used by the FRAM driver |
| `Src/peripherals/PDB/power_distribution.c` | `pdb_init()` / `pdb_pyro_off()` |
| `Src/peripherals/FRAM/MB85RS256B.c` | the existing checked FRAM driver |
| `Src/tools/Utils.c` | the bounded `wait_with_timeout()` the SPI driver uses |
| `onboard/startup_fram_provision.c` | validation + staged write |
| `onboard/startup_fram_provision_boot.c` | bring-up, OFF, report, reset/park |
| `onboard/startup_fram_provision_main.c` | `SystemInit()` + `main()` for this image only |

`main.c`, `.cproject` and the normal firmware are untouched. The image supplies
its own `SystemInit()` because the CubeIDE-managed system file is not part of this
lane; `startup_fram_provision_boot()` then calls the real `init_coreClocks()`.

Linker script: `onboard/provisioning_STM32L476ZGTX_FLASH.ld` is a copy of the
project's `STM32L476ZGTX_FLASH.ld` with exactly one addition, a
`.noinit (NOLOAD)` section for the inspection report. Regenerate and re-diff it
with `--refresh-linker-script`; `diff STM32L476ZGTX_FLASH.ld
tools/startup_fram/onboard/provisioning_STM32L476ZGTX_FLASH.ld` must show only
that addition.

Expected result of a clean build (13.3.1, `-O2 -g3 --specs=nano.specs
--specs=nosys.specs -static -Wl,--gc-sections`), from the script's own log:

```
arm-none-eabi-gcc (GNU Tools for STM32 13.3.rel1.20250523-0900) 13.3.1 20240614
RAM:        1736 B        96 KB      1.77%
FLASH:      4612 B         1 MB      0.44%
OK: report 0x20000034 (+0x00000090) is outside .bss (ends 0x20000034) and reset-persistent
OK: no normal-boot symbols (init_init/init_platform/Startup/FreeRTOS) in the image
BUILD OK (exit 0)
```

Artefacts: `onboard/build/startup_fram_provision.{elf,bin,map}`,
`onboard/build/startup_fram_image.{h,bin}`.

`startup_fram_provision_boot.c` and `startup_fram_provision_main.c` `#error`
unless `STARTUP_FRAM_PROVISIONING_BUILD` is defined, so the writer cannot be
linked into the normal firmware; the build script also fails if
`init_init`/`init_platform`/`Startup`/FreeRTOS symbols appear in the image.

### Runtime behaviour of the image

1. GPIOA/GPIOC clocks, `pdb_init()`, `pdb_pyro_off()` — outputs OFF before any
   fallible step.
2. `init_coreClocks()`, `led_init()`, TIM7 tick, `spi_config(SPI1)`.
3. Fill the report, run the writer, then **always** read addresses 0..47 back
   through `FRAM_readData()` and compare with the intended image.
4. `pdb_pyro_off()` again.
5. Failure (writer status ≠ OK **or** read-back mismatch): no logging, no delay,
   `NVIC_SystemReset()`. A permanent fault reset-loops by design.
6. Success: park in `WFI` with outputs OFF and no further FRAM access.

## 6. Verifying a run by hand (GDB)

The image leaves a bounded, reset-persistent report at
`startup_fram_provision_report` (144 bytes: 12 status words + intended[48] +
readback[48]) in `.noinit`, outside `.bss`, so it survives the failure path's
reset. `readback[]` is filled by the target's own `FRAM_readData()` call after the
write sequence — it is the device's answer, not the intended image echoed back.

```bash
ELF=tools/startup_fram/onboard/build/startup_fram_provision.elf
DBG=/Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.macos64_1.0.100.202509120712/tools/bin/arm-none-eabi-gdb
$DBG "$ELF"
(gdb) target extended-remote :3333        # your own ST-LINK gdbserver; not started here
(gdb) monitor reset halt
(gdb) load                                # flash the provisioning image
(gdb) monitor reset halt                  # reset, but do not run yet
(gdb) tbreak pdb_pyro_off                 # temporary breakpoint on OFF
(gdb) ignore $bpnum 1                     # skip the initial OFF; stop at final OFF
(gdb) continue                            # final OFF follows the fresh FRAM readback
(gdb) finish                              # execute OFF; halt before reset/park
(gdb) p startup_fram_provision_report
(gdb) dump binary memory report.bin &startup_fram_provision_report \
      &startup_fram_provision_report + sizeof(startup_fram_provision_report)
(gdb) dump binary memory readback.bin startup_fram_provision_report.readback \
      startup_fram_provision_report.readback + 48
(gdb) x/48xb startup_fram_provision_report.readback
```

This breakpoint sequence relies on the initializer's two explicit OFF calls: it
skips the early safe-state call and stops at the final call **after** fresh SPI
readback. Do not inspect the report at function entry; it may contain an older
run's values. The `.noinit` buffer is volatile and is overwritten on the next run;
it is not power-loss storage. These hardware commands have not been executed as
part of software verification.

Read the fields as: `status == 0` (`STARTUP_FRAM_PROVISION_OK`),
`readback_matches == 1`, `mismatch_count == 0`,
`first_mismatch_addr == 0xFFFFFFFF`, `readback_slot_a_marker == 0xA5`,
`readback_slot_b_marker == 0xA5`, `readback_boot_counter`/`readback_adcs_mask`
equal to what you generated. `readback.bin` can be compared with the generated
`startup_fram_image.bin` (`cmp -l` or `xxd`).

Limits, stated plainly: this comparison proves what the board reported over its
own SPI bus in that run. It is **not** proof that a later normal boot will behave
correctly, and it is not a substitute for reading the FRAM with an external
programmer when the programme requires independent verification. Because the
failure path resets, halt the core (or set a breakpoint) before the next cycle
overwrites the report with a new run's data.

## 7. Ground procedure (test and flight)

1. Deployment harness physically disconnected; pyro outputs verified OFF. The
   image keeps them OFF and never commands ON.
2. Select the ADCS sensors for **this** unit explicitly and generate the image;
   use `--armed` only if this unit may deploy on the first normal boot.
3. Build the image with `build_provisioning.sh`, flash it deliberately, let it run
   once, read the report (§6), then restore the normal firmware.
4. Power loss during provisioning is a re-provisioning case: the markers may be
   left invalid and the normal firmware then treats the device as a storage error
   (OFF + reset). Re-provision with the harness disconnected.

Provisioning is an intentional reset of boot counter, ADCS mask and both Startup
slots — not a migration. It discards existing progress by design.

### What gates an actual deployment (unchanged by these tools)

* The normal firmware keeps its existing battery-eligibility gate for every ON
  command; **that gate defaults to false**, so no ON is issued unless the running
  flight software says the battery is eligible. These tools invent no voltage or
  energy threshold and provide no hardware cut-off guarantee.
* An armed record is necessary but not sufficient: a first boot with
  `INITIAL_CHECKS` still honours the 1800000 ms inhibit and then the battery gate.
* The provisioning image itself never runs Startup and never issues an ON.

## 8. Tests

```bash
make -C tools/startup_fram/tests -f Makefile test              # python + host C suites + guard
make -C tools/startup_fram/tests -f Makefile arm-compile       # 13.3.1 compile of onboard sources
make -C tools/startup_fram/tests -f Makefile provisioning-image # full ELF/bin link
```

* Suites are self-contained: the golden and sequence-max vectors are literal
  copies in `tests/contract_fixture.{h,py}`; nothing reads `SPEC.md` or
  `CONTRACT-VECTOR.json` at build or test time.
* The host C suites compile the real `onboard/*.c` against a memory FRAM device
  (`tests/fram_device_fake.c`) that injects hard errors, short writes, bit
  corruption, lost responses, read failures and write-disable failures at every
  stage and every byte offset of a block.
* The boot-entry suite asserts the report contents: status, read-back equality,
  mismatch count/address, decoded read-back fields, and the difference between
  "device matches but the writer could not confirm" and "device differs".
* `BIN_DIR` may be relative or absolute.

## 9. Hazards

* Legacy destructive FRAM tests (`Src/peripherals/FRAM/FRAM_testing.c` and the old
  RTC-to-FRAM test definitions) write into this same reserved region. Never run
  them against a mission-state device.
* The legacy `FRAM_*` convenience helpers do not check every return value; this
  module uses the checked byte APIs instead of "fixing" the whole repository.
* Nothing here flashes, programs, installs or exercises hardware.
