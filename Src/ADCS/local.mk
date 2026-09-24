# Host-specific build overrides (gitignored).
#
# macOS + Homebrew GCC (e.g. `make CC=gcc-15`): GCC's own stdint.h does
# `#include_next <stdint.h>` expecting the system header, but on macOS the
# system headers live inside the Xcode SDK, which GCC does not search by
# default. Point it at the SDK with -isysroot. (Apple's clang finds it on its
# own, so this is only needed when building with Homebrew GCC.)
# SYSROOT is consumed by both CFLAGS (library build) and TEST_FLAGS (make test).
CC = gcc-15
SYSROOT := -isysroot $(shell xcrun --show-sdk-path)
CFLAGS += $(SYSROOT)
TEST_FLAGS += $(SYSROOT)
