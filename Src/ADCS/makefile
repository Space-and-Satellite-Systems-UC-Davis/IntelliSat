include config.mk
include sources.mk

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

clean:
	@echo "CLEAN"
	$(Q)rm -f $(TARGET) $(OBJECTS) $(DEPENDENCIES)

.PHONY: all clean run

