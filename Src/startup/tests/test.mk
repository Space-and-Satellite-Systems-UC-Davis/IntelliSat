# Native host tests for the actual Startup() and the real startup_store module.
# Run from the repository root: make -C Src/startup/tests -f test.mk test
CC ?= cc
BIN_DIR ?= bin
TEST_FLAGS := -std=c11 -g -Wall -Wextra -Werror -Istubs -I. -I.. -I../.. -I../../peripherals
SANITIZER_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
TEST_BINS := $(BIN_DIR)/test_startup_store $(BIN_DIR)/test_startup $(BIN_DIR)/test_startup_default_battery $(BIN_DIR)/test_main_entry $(BIN_DIR)/test_init_entry
STUB_HEADERS := $(wildcard stubs/*.h stubs/*/*.h)

STORE_SOURCES := ../startup_store.c ../startup_store.h ../startup_state.h
STARTUP_SOURCES := ../startup.h ../startup_reset.h $(STORE_SOURCES) startup_fixtures.h

$(BIN_DIR)/test_startup_store: test_startup_store.c $(STORE_SOURCES) framework.h startup_fixtures.h $(STUB_HEADERS)
$(BIN_DIR)/test_startup: test_startup.c $(STARTUP_SOURCES) ../../peripherals/PDB/power_distribution.c framework.h $(STUB_HEADERS)
$(BIN_DIR)/test_startup_default_battery: test_startup.c $(STARTUP_SOURCES) ../../peripherals/PDB/power_distribution.c framework.h $(STUB_HEADERS)
$(BIN_DIR)/test_startup_default_battery: TEST_FLAGS += -DSTARTUP_TEST_DEFAULT_BATTERY

# Exact entry bodies only: avoid unrelated platform/RTOS dependencies in native builds.
$(BIN_DIR)/main_function.inc: ../../main.c extract_main.py | $(BIN_DIR)
	python3 extract_main.py $< $@
$(BIN_DIR)/init_init_function.inc: ../../platform_init.h extract_main.py | $(BIN_DIR)
	python3 extract_main.py $< $@ "void init_init\\(\\)"
$(BIN_DIR)/test_main_entry: test_main_entry.c $(BIN_DIR)/main_function.inc $(STUB_HEADERS)
$(BIN_DIR)/test_main_entry: TEST_FLAGS += -I$(BIN_DIR)
$(BIN_DIR)/test_init_entry: test_init_entry.c $(BIN_DIR)/init_init_function.inc ../../platform_init.h ../../peripherals/PDB/power_distribution.c ../../peripherals/PDB/power_distribution.h $(STUB_HEADERS)
$(BIN_DIR)/test_init_entry: TEST_FLAGS += -I$(BIN_DIR)

# Binaries that link only their own translation units. The real PDB source is
# included by the startup/init entry tests with renamed symbols, so it is not
# linked a second time.
LINKED_BINS := $(TEST_BINS)
$(LINKED_BINS): | $(BIN_DIR)
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) $(SANITIZER_FLAGS) $(filter-out ../../peripherals/PDB/power_distribution.c,$(filter %.c,$^)) -o $@ -lm

$(BIN_DIR):
	@mkdir -p $@

.PHONY: test
test: $(TEST_BINS)
	@status=0; \
	for b in $(TEST_BINS); do \
		echo "--- $$b ---"; \
		ASAN_OPTIONS=halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 $$b || status=1; \
	done; \
	exit $$status

.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
