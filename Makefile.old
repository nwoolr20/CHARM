# CHARM System Makefile - Focused build approach
# Build core functionality first, then extend

# Compiler and flags
CC = gcc
AR = ar
RANLIB = ranlib
CFLAGS = -Wall -Wextra -std=c99 -O2 -mavx2 -march=native -fopenmp -g
LDFLAGS = -pthread -lm -lssl -lcrypto

# Core essential source files for minimal working system
CORE_SOURCES = avx2_detect.c ece_core.c ece_digest.c entropy_bus.c main.c

# All library source files (for full build)
ALL_LIB_SOURCES = avx2_detect.c caeds_anomaly.c caeds_flux.c caeds_notify.c caeds_predict.c \
                  cee_buffer.c cee_mix.c cee_whiten.c charmctl.c ece_core.c ece_digest.c \
                  ece_seed.c ece_stream.c entropy_bus.c entropy_trace.c fallback_entropy.c \
                  neon_backend.c rng_linux.c snapshot_logger.c watchdog_daemon.c

# Object files for core build
CORE_OBJECTS = avx2_detect.o ece_core.o ece_digest.o entropy_bus.o
ALL_LIB_OBJECTS = $(ALL_LIB_SOURCES:.c=.o)

# Output files
LIB_STATIC = libcharm.a
CHARM_BIN = charm
BENCH_BIN = bench_digest
COMPREHENSIVE_BENCH = benchmark_comprehensive

# Default target - build core functionality
all: core

# Build core functionality only
core: $(CORE_OBJECTS) main_simple.o
	@echo "Building CHARM core binary..."
	$(CC) $(CFLAGS) -o $(CHARM_BIN) main_simple.o $(CORE_OBJECTS) $(LDFLAGS)

# Build comprehensive benchmark
bench: $(CORE_OBJECTS) benchmark_comprehensive.o
	@echo "Building comprehensive benchmark..."
	$(CC) $(CFLAGS) -o $(COMPREHENSIVE_BENCH) benchmark_comprehensive.o $(CORE_OBJECTS) $(LDFLAGS)

# Build full system
full: $(LIB_STATIC) $(CHARM_BIN) $(BENCH_BIN)

# Build static library
$(LIB_STATIC): $(ALL_LIB_OBJECTS)
	@echo "Building CHARM library..."
	$(AR) rcs $@ $^
	$(RANLIB) $@

# Build main executable
$(CHARM_BIN): main.o $(LIB_STATIC)
	@echo "Building CHARM binary..."
	$(CC) $(CFLAGS) -o $@ main.o -L. -lcharm $(LDFLAGS)

# Build benchmark executable
$(BENCH_BIN): bench_digest.o $(LIB_STATIC)
	@echo "Building benchmark binary..."
	$(CC) $(CFLAGS) -o $@ bench_digest.o -L. -lcharm $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Run tests
test: $(CHARM_BIN)
	@echo "Running CHARM test suite..."
	@chmod +x test_all.sh
	@./test_all.sh

# Run benchmarks
benchmark: $(BENCH_BIN)
	@echo "Running CHARM benchmarks..."
	@./bench_digest

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -f *.o $(LIB_STATIC) $(CHARM_BIN) $(BENCH_BIN) $(COMPREHENSIVE_BENCH)
	@rm -rf test_output/
	@echo "Clean complete."

# Help
help:
	@echo "CHARM System Makefile"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  core      - Build core ECE functionality (default)"
	@echo "  full      - Build complete system with all components"
	@echo "  clean     - Remove build files"
	@echo "  bench     - Build comprehensive benchmark tool"
	@echo "  test      - Run test suite"
	@echo "  benchmark - Run performance benchmarks"
	@echo "  help      - Show this help message"

.PHONY: all core full clean test benchmark help
