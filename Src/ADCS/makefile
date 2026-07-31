# Build the library by default. 
.DEFAULT_GOAL := all

include config.mk
include sources.mk
include test.mk

# Optional host-specific overrides (gitignored). Used for things like pointing
# Homebrew GCC at the macOS SDK; see local.mk.example.
-include local.mk

# Optional host-specific overrides (gitignored). Used for things like pointing
# Homebrew GCC at the macOS SDK; see local.mk.example.
-include local.mk

# Target executable name
TARGET := libADCS.a

all: $(TARGET)

DEPENDENCIES := $(patsubst %.o,%.d,$(OBJECTS))
-include $(DEPENDENCIES)

$(TARGET): $(OBJECTS)
	@echo "AR $@"
	$(Q)ar rcs $@ $^

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@ 

run:
	./$(TARGET)

version:
	$(Q)$(CC) --version

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do ./$$t || exit 1; done

clean:
	@echo "CLEAN"
	$(Q)rm -f $(TARGET) $(OBJECTS) $(DEPENDENCIES)
	$(Q)rm -rf $(TEST_DIR)/bin


.PHONY: all clean run test version


