# Native host tests: real Startup() and real startup_decisions.c.
# Run from the repository root: make -C Src/startup/tests -f test.mk test
CC ?= cc
BIN_DIR ?= bin
TEST_FLAGS := -std=c11 -g -Wall -Wextra -Werror -Istubs -I. -I.. -I../.. -I../../peripherals
SANITIZER_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
TEST_BINS := $(BIN_DIR)/test_startup_decisions $(BIN_DIR)/test_startup $(BIN_DIR)/test_startup_default_battery $(BIN_DIR)/test_persistence $(BIN_DIR)/test_main_entry $(BIN_DIR)/test_init_entry $(BIN_DIR)/test_startup_clock $(BIN_DIR)/test_startup_clock_integration
STUB_HEADERS := $(wildcard stubs/*.h stubs/*/*.h stubs/*/*/*.h)
# Sources included by the clock tests themselves, listed only as rebuild dependencies.
CLOCK_SOURCES := ../startup_clock.c ../startup_clock.h ../startup_persistence.h clock_fixture.h

$(BIN_DIR)/test_startup_decisions: test_startup_decisions.c ../startup_decisions.c ../startup_decisions.h framework.h
$(BIN_DIR)/test_startup: test_startup.c ../startup_decisions.c ../startup_persistence.c ../../peripherals/PDB/power_distribution.c ../startup.h ../startup_decisions.h ../startup_persistence.h ../startup_clock.h startup_fixtures.h framework.h $(STUB_HEADERS)
$(BIN_DIR)/test_startup_default_battery: test_startup.c ../startup_decisions.c ../startup_persistence.c ../../peripherals/PDB/power_distribution.c ../startup.h ../startup_decisions.h ../startup_persistence.h ../startup_clock.h startup_fixtures.h framework.h $(STUB_HEADERS)
$(BIN_DIR)/test_startup_default_battery: TEST_FLAGS += -DSTARTUP_TEST_DEFAULT_BATTERY
# Extract the real private clock-boundary declarations and the real RTC primitives it
# calls, so a permissive stub cannot hide interface drift.
$(BIN_DIR)/startup_clock_contract.h: ../startup_clock.h ../../system_config/RTC/rtc.h | $(BIN_DIR)
	@printf '#include <stdbool.h>\n#include <stdint.h>\n#include "startup_persistence.h"\n' > $@
	@sed -n '/^void startup_clock_/p;/^StClock startup_clock_/p;/^bool startup_clock_/p' $< >> $@
	@grep -q '^void startup_clock_capture_boot_evidence(void);' $@
	@grep -q '^void startup_clock_init(void);' $@
	@grep -q '^StClock startup_clock_sample(void);' $@
	@grep -q '^StClock startup_clock_anchor(void);' $@
	@grep -q '^bool startup_clock_retained(void);' $@
	@grep -q '^bool startup_clock_confirm_wait(uint32_t source, uint32_t revision);' $@
	@sed -n '/^void rtc_config(char clock_source, int forced_config);/p;/^void rtc_setCalendar(/p;/^void rtc_setTime(/p;/^void rtc_getTime(/p;/^void rtc_getCalendar(/p' ../../system_config/RTC/rtc.h >> $@
	@grep -q '^void rtc_config(char clock_source, int forced_config);' $@
	@grep -q '^void rtc_setCalendar(uint8_t year, uint8_t month, uint8_t date, uint8_t day);' $@
	@grep -q '^void rtc_setTime(uint8_t hour, uint8_t minute, uint8_t second);' $@
	@grep -q '^void rtc_getTime(uint8_t \*hour, uint8_t \*minute, uint8_t \*second);' $@
	@grep -q '^void rtc_getCalendar(uint8_t \*year, uint8_t \*month, uint8_t \*date, uint8_t \*day);' $@
$(BIN_DIR)/test_startup $(BIN_DIR)/test_startup_default_battery $(BIN_DIR)/test_startup_clock $(BIN_DIR)/test_startup_clock_integration: $(BIN_DIR)/startup_clock_contract.h
$(BIN_DIR)/test_startup $(BIN_DIR)/test_startup_default_battery $(BIN_DIR)/test_startup_clock $(BIN_DIR)/test_startup_clock_integration: TEST_FLAGS += -D_POSIX_C_SOURCE=200809L -include $(BIN_DIR)/startup_clock_contract.h
$(BIN_DIR)/test_persistence: test_persistence.c ../startup_persistence.c ../startup_persistence.h framework.h $(STUB_HEADERS)

# Exact entry bodies only: avoid unrelated platform/RTOS dependencies in native builds.
# main() is now just the init chain, so the PDB safe-output ordering is modeled
# from the actual init_init() body in platform_init.h.
$(BIN_DIR)/main_function.inc: ../../main.c extract_main.py | $(BIN_DIR)
	python3 extract_main.py $< $@
$(BIN_DIR)/init_init_function.inc: ../../platform_init.h extract_main.py | $(BIN_DIR)
	python3 extract_main.py $< $@ "void init_init\\(\\)"
$(BIN_DIR)/test_main_entry: test_main_entry.c $(BIN_DIR)/main_function.inc $(STUB_HEADERS)
$(BIN_DIR)/test_main_entry: TEST_FLAGS += -I$(BIN_DIR)
$(BIN_DIR)/test_init_entry: test_init_entry.c $(BIN_DIR)/init_init_function.inc ../../platform_init.h ../../peripherals/PDB/power_distribution.c ../../peripherals/PDB/power_distribution.h $(STUB_HEADERS)
$(BIN_DIR)/test_init_entry: TEST_FLAGS += -I$(BIN_DIR)

# Binaries that link only their own translation unit; the clock tests include their sources.
LINKED_BINS := $(filter-out $(BIN_DIR)/test_startup_clock $(BIN_DIR)/test_startup_clock_integration,$(TEST_BINS))
$(LINKED_BINS): | $(BIN_DIR)
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) $(SANITIZER_FLAGS) $(filter-out ../../peripherals/PDB/power_distribution.c,$(filter %.c,$^)) -o $@ -lm

# The clock tests include the real module (and persistence/decisions) as sources, so
# only the test translation unit is compiled.
$(BIN_DIR)/test_startup_clock: test_startup_clock.c $(CLOCK_SOURCES) framework.h $(STUB_HEADERS)
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) $(SANITIZER_FLAGS) test_startup_clock.c -o $@ -lm
$(BIN_DIR)/test_startup_clock_integration: test_startup_clock_integration.c $(CLOCK_SOURCES) ../startup_persistence.c ../startup.h ../startup_decisions.c startup_fixtures.h framework.h $(STUB_HEADERS)
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) $(SANITIZER_FLAGS) test_startup_clock_integration.c -o $@ -lm

$(BIN_DIR):
	@mkdir -p $@

.PHONY: test
test: $(TEST_BINS)
	@status=0; \
	for b in $(TEST_BINS); do \
		echo "--- $$b ---"; \
		ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 ./$$b || status=1; \
	done; \
	exit $$status

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
