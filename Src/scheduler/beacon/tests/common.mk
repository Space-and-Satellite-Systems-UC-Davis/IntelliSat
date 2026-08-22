# ── Shared host-test build fragment for Src/scheduler/* feature tasks ──────
#
# A feature's own tests/test.mk (e.g. Src/scheduler/beacon/tests/test.mk)
# sets TEST_DIR, INCLUDES, and TEST_BINS (with per-binary prerequisite
# lists) and then `include`s this file for the actual build recipe and
# the test/clean targets, so every feature's host tests build and run
# the same way.
#
# Usage (from a feature dir, e.g. Src/scheduler/beacon/):
#   make -f tests/test.mk test
#   make -f tests/test.mk clean

CC ?= gcc
TEST_FLAGS := -g -Wall -Wextra -Wno-unused-parameter $(INCLUDES) -MMD -MP

$(TEST_BINS): | $(TEST_DIR)/bin
	@echo "CC $@"
	$(CC) $(TEST_FLAGS) $^ -o $@ -lm

$(TEST_DIR)/bin:
	@mkdir -p $@

.PHONY: test
test: $(TEST_BINS)
	@status=0; \
	for bin in $(TEST_BINS); do \
		echo "--- $$bin ---"; \
		./$$bin || status=1; \
	done; \
	exit $$status

.PHONY: clean
clean:
	rm -rf $(TEST_DIR)/bin $(TEST_DIR)/*.d
