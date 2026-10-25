# Host tests for the Startup FRAM provisioning tools.
#
#   make -C tools/startup_fram/tests -f Makefile test               # python + C suites
#   make -C tools/startup_fram/tests -f Makefile arm-compile        # 13.3.1 cross-compile
#   make -C tools/startup_fram/tests -f Makefile provisioning-image # full ELF/bin link
#
# The C suites compile the real onboard sources in ../onboard against the memory
# FRAM device in this directory. Golden vectors are the literal copies committed in
# contract_fixture.h / contract_fixture.py, so nothing here reads files outside the
# lane (no ../SPEC.md, no ../CONTRACT-VECTOR.json).
#
# PROJECT is this lane's checkout (Src/, Drivers/); BIN_DIR may be relative or
# absolute.
PROJECT := ../../..
CC ?= cc
BIN_DIR ?= bin
RUN_TIMEOUT ?= 120

TEST_FLAGS := -std=c11 -g -O1 -Wall -Wextra -Werror \
              -Istubs-platform -Istubs -I../onboard -I. \
              -I$(PROJECT)/Src/peripherals/FRAM
SANITIZER_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer

WRITER_SOURCES := ../onboard/startup_fram_provision.c fram_device_fake.c
BOOT_SOURCES := ../onboard/startup_fram_provision_boot.c ../onboard/startup_fram_provision.c \
                fram_device_fake.c
HEADERS := stubs/stm32l476xx.h stubs/globals.h stubs-platform/core_config.h \
           stubs-platform/SPI/spi.h stubs-platform/PDB/power_distribution.h \
           stubs-platform/Timers/timers.h stubs-platform/LED/led.h \
           ../onboard/startup_fram_provision.h ../onboard/startup_fram_provision_report.h

# A generated header makes the entry test prove the tool's output is linkable.
GENERATED_HEADER := $(BIN_DIR)/startup_fram_image.h
GENERATED_ARGS := generate --boot-count 0 --adcs-mask 0 \
                  --header-symbol STARTUP_FRAM_PROVISION_IMAGE

TEST_BINS := $(BIN_DIR)/test_provision_writer $(BIN_DIR)/test_provision_boot

$(BIN_DIR):
	@mkdir -p $@

$(GENERATED_HEADER): ../startup_fram.py ../startup_image.py | $(BIN_DIR)
	@echo "GEN $@"
	@rm -f $@  # the tool creates exclusively; the build owns this file
	python3 ../startup_fram.py $(GENERATED_ARGS) --header $@

$(BIN_DIR)/test_provision_writer: test_provision_writer.c $(WRITER_SOURCES) $(HEADERS) framework.h fram_device_fake.h contract_fixture.h | $(BIN_DIR)
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) $(SANITIZER_FLAGS) test_provision_writer.c $(WRITER_SOURCES) -o $@

$(BIN_DIR)/test_provision_boot: test_provision_boot.c $(BOOT_SOURCES) $(HEADERS) framework.h fram_device_fake.h $(GENERATED_HEADER) | $(BIN_DIR)
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) -I$(BIN_DIR) -DSTARTUP_FRAM_PROVISIONING_BUILD $(SANITIZER_FLAGS) \
	    test_provision_boot.c $(BOOT_SOURCES) -o $@

.PHONY: python-test
python-test:
	@echo "--- python unittest ---"
	python3 -m unittest discover -s . -p 'test_*.py' -t .

.PHONY: c-test
c-test: $(TEST_BINS)
	@status=0; \
	for b in $(TEST_BINS); do \
		echo "--- $$b ---"; \
		ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
		    python3 -c 'import subprocess, sys; sys.exit(subprocess.run(sys.argv[2:], timeout=float(sys.argv[1])).returncode)' "$(RUN_TIMEOUT)" "$$b" || status=1; \
	done; \
	exit $$status

# The provisioning entry must refuse to build as part of the normal firmware.
.PHONY: guard-test
guard-test: | $(BIN_DIR)
	@echo "--- provisioning entry is not part of the normal build ---"
	@if $(CC) $(TEST_FLAGS) -c ../onboard/startup_fram_provision_boot.c -o $(BIN_DIR)/guard.o 2>$(BIN_DIR)/guard.log; then \
		echo "FAIL: startup_fram_provision_boot.c compiled without STARTUP_FRAM_PROVISIONING_BUILD"; \
		exit 1; \
	fi
	@grep -q "separate provisioning image" $(BIN_DIR)/guard.log \
		|| { echo "FAIL: unexpected guard error"; cat $(BIN_DIR)/guard.log; exit 1; }
	@echo "ok: #error guard fired"

.PHONY: test
test: python-test c-test guard-test

# Exact GNU Arm compile of the provisioning sources against the real headers.
# The provisioning-image target below links the separate initializer using the
# project's startup/clock configuration; this target checks its translation units.
# Exact GNU Arm 13.3.1 shipped with STM32CubeIDE; override with ARM_CC=...
CUBEIDE_TOOLS ?= /Applications/STM32CubeIDE.app/Contents/Eclipse/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.macos64_1.0.100.202509120712/tools/bin
ARM_CC ?= $(CUBEIDE_TOOLS)/arm-none-eabi-gcc
ARM_FLAGS := -mcpu=cortex-m4 -mthumb -std=c11 -Os -g -Wall -Wextra -Werror \
             -I$(PROJECT)/Drivers/Device -I$(PROJECT)/Src/inc -I$(PROJECT)/Src/peripherals \
             -I$(PROJECT)/Src/system_config -I$(PROJECT)/Src/peripherals/FRAM

.PHONY: arm-compile
arm-compile: $(GENERATED_HEADER)
	@mkdir -p $(BIN_DIR)/arm
	@echo "ARM CC startup_fram_provision.c"
	$(ARM_CC) $(ARM_FLAGS) -c ../onboard/startup_fram_provision.c -o $(BIN_DIR)/arm/startup_fram_provision.o
	@echo "ARM CC startup_fram_provision_boot.c"
	$(ARM_CC) $(ARM_FLAGS) -I$(BIN_DIR) -DSTARTUP_FRAM_PROVISIONING_BUILD \
	    -c ../onboard/startup_fram_provision_boot.c -o $(BIN_DIR)/arm/startup_fram_provision_boot.o
	@$(ARM_CC) --version | head -1

# Full standalone provisioning image (ELF + bin) with the exact 13.3.1 toolchain.
.PHONY: provisioning-image
provisioning-image:
	../onboard/build_provisioning.sh

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
