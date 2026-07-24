# ── Tests ─────────────────────────────────────────────────────────────────────

TEST_DIR   := tests
TEST_FLAGS := -g -Wall -Wextra -Wno-unused-variable -Wno-unused-value \
              -Wno-unused-but-set-variable -I.
TEST_FLAGS += -MMD -MP

VI_STUBS   := $(TEST_DIR)/stubs/vi_stubs.c

TEST_BINS := \
	$(TEST_DIR)/bin/test_vector \
	$(TEST_DIR)/bin/test_matrix \
	$(TEST_DIR)/bin/test_quaternion \
	$(TEST_DIR)/bin/test_calibration \

# Per-test sources: test driver + unit(s) under test
$(TEST_DIR)/bin/test_vector: \
	$(TEST_DIR)/adcs_math/test_vector.c \
	adcs_math/vector.c

$(TEST_DIR)/bin/test_matrix: \
	$(TEST_DIR)/adcs_math/test_matrix.c \
	adcs_math/matrix.c adcs_math/vector.c

$(TEST_DIR)/bin/test_quaternion: \
	$(TEST_DIR)/adcs_math/test_quaternion.c \
	adcs_math/quaternion.c adcs_math/vector.c

$(TEST_DIR)/bin/test_calibration: \
	$(TEST_DIR)/adcs_math/test_calibration.c \
	adcs_math/calibration.c adcs_math/vector.c $(VI_STUBS)

# Shared recipe for every test binary
$(TEST_BINS): | $(TEST_DIR)/bin
	@echo "CC $@"
	$(Q)$(CC) $(TEST_FLAGS) $^ -o $@ -lm

$(TEST_DIR)/bin:
	@mkdir -p $@