# ── Beacon host-side tests ──────────────────────────────────────────────────
# Run from Src/scheduler/beacon/: `make -f tests/test.mk test`
#
# The build recipe, CC/TEST_FLAGS, and test/clean targets are shared
# across Src/scheduler feature tasks - see Src/scheduler/tests/common.mk.
# This file only defines what's specific to beacon: include paths, test
# binaries, and each binary's source list.

TEST_DIR := tests

# ../tests (Src/scheduler/tests) has the shared framework.h.
# tests/stubs is listed before ../../peripherals so "loggers_to_fram.h"
# resolves to the stub, not the real hardware-backed header.
INCLUDES := -I. -I../../ADCS -I../tests -I$(TEST_DIR)/stubs

FRAM_STUB := $(TEST_DIR)/stubs/fram_stub.c

TEST_BINS := \
	$(TEST_DIR)/bin/test_beacon_record \
	$(TEST_DIR)/bin/test_beacon_stats

$(TEST_DIR)/bin/test_beacon_record: \
	$(TEST_DIR)/test_beacon_record.c \
	beacon_record.c

$(TEST_DIR)/bin/test_beacon_stats: \
	$(TEST_DIR)/test_beacon_stats.c \
	beacon_stats.c \
	$(FRAM_STUB)

include ../tests/common.mk
