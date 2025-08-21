# CHARM-AEAS Makefile
# Build AEAS library, tests, and benchmarks

# Paths
TOPDIR = ../..
CHARM_INCLUDE = $(TOPDIR)/algorithm/include
CHARM_SRC = $(TOPDIR)/algorithm/src
ECE_INCLUDE = $(TOPDIR)/include
ECE_SRC = $(TOPDIR)/src/core

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3 -flto -ffast-math -funroll-loops -mavx2 -march=native -mtune=native -g
INCLUDES = -Iinclude -I$(CHARM_INCLUDE) -I$(ECE_INCLUDE)
LIBS = -lm

# OpenSSL support (optional)
ifdef OPENSSL
    CFLAGS += -DWITH_OPENSSL
    LIBS += -lcrypto
endif

# AVX2 support (optional)
ifdef AVX2
    CFLAGS += -mavx2 -DAEAS_USE_AVX2
endif

# Source files
AEAS_SOURCES = src/util.c src/hmac_charm.c src/hkdf_charm.c src/poly1305.c src/aeas.c
CHARM_SOURCES = $(CHARM_SRC)/charm_lib.c
ECE_SOURCES = $(ECE_SRC)/ece_core.c $(ECE_SRC)/ece_digest.c $(ECE_SRC)/entropy_bus.c $(ECE_SRC)/system_entropy.c $(TOPDIR)/src/utils/avx2_detect.c

# Object files
AEAS_OBJECTS = $(AEAS_SOURCES:.c=.o)
CHARM_OBJECTS = $(CHARM_SOURCES:.c=.o)
ECE_OBJECTS = $(ECE_SOURCES:.c=.o)
ALL_OBJECTS = $(AEAS_OBJECTS) $(CHARM_OBJECTS) $(ECE_OBJECTS)

# Targets
LIBRARY = libaeas.a
TEST_BINARY = aeas_test
BENCH_BINARY = aeas_bench

.PHONY: all clean test bench lib

all: $(LIBRARY) $(TEST_BINARY)

lib: $(LIBRARY)

# Build static library
$(LIBRARY): $(ALL_OBJECTS)
	ar rcs $@ $^
	ranlib $@

# Object file rules
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Test binary
$(TEST_BINARY): tests/aeas_test.c $(LIBRARY)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -laeas $(LIBS)

# Benchmark binary
$(BENCH_BINARY): bench/bench.c $(LIBRARY)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< -L. -laeas $(LIBS)

# Run tests
test: $(TEST_BINARY)
	./$(TEST_BINARY)
	@echo "Test results saved to results/"

# Run benchmarks
bench: $(BENCH_BINARY)
	./$(BENCH_BINARY)
	@echo "Benchmark results saved to results/"

# Clean
clean:
	rm -f $(ALL_OBJECTS) $(LIBRARY) $(TEST_BINARY) $(BENCH_BINARY)
	rm -f results/*.json results/*.md

# Help
help:
	@echo "CHARM-AEAS Build System"
	@echo "Usage: make [target] [options]"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build library and tests (default)"
	@echo "  lib      - Build static library only"
	@echo "  test     - Build and run tests"
	@echo "  bench    - Build and run benchmarks"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help"
	@echo ""
	@echo "Options:"
	@echo "  OPENSSL=1 - Enable OpenSSL for benchmarks"
	@echo "  AVX2=1    - Enable AVX2 optimizations"
	@echo ""
	@echo "Examples:"
	@echo "  make test"
	@echo "  make bench OPENSSL=1"
	@echo "  make all AVX2=1"