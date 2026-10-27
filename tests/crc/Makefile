CC := clang
ROOT := ../..
BUILD := .build
CPPFLAGS := -Istubs -I$(ROOT)/Src/system_config
# Suppress existing crc.c warning categories, not sanitizer findings.
CFLAGS := -std=c11 -Wall -Wextra -Werror -Wno-pointer-sign -Wno-sign-compare -Wno-unused-parameter -g -fsanitize=address,undefined -fno-omit-frame-pointer
.PHONY: test
test: $(BUILD)/test_crc
	$(BUILD)/test_crc

$(BUILD)/test_crc: test_crc.c $(ROOT)/Src/system_config/UART/crc.c $(ROOT)/Src/system_config/UART/crc.h stubs/UART/uart.h stubs/print_scan.h
	mkdir -p $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) test_crc.c $(ROOT)/Src/system_config/UART/crc.c -o $@
