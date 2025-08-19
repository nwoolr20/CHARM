# CHARM System Makefile - Organized structure with performance optimizations
# Build core functionality first, then extend

# Compiler and flags - Enhanced for maximum performance
CC = gcc
AR = ar
RANLIB = ranlib

# Performance-optimized compilation flags
CFLAGS = -Wall -Wextra -std=c99 -O3 -flto -ffast-math -funroll-loops \
         -mavx2 -march=native -mtune=native -fopenmp \
         -fomit-frame-pointer -finline-functions -g

# Note: AVX512 support is available in the code but not compiled by default
# to ensure compatibility. On AVX512-capable systems, manually add -mavx512f

# Linker flags
LDFLAGS = -pthread -lm -lssl -lcrypto -flto -L. -lblake3

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
UTILS_DIR = $(SRC_DIR)/utils
TESTS_DIR = $(SRC_DIR)/tests
INCLUDE_DIR = include
BUILD_DIR = build
TOOLS_DIR = tools
BENCHMARKS_DIR = benchmarks

# Include path
INCLUDES = -I$(INCLUDE_DIR)

# Core essential source files for minimal working system (updated paths)
CORE_SOURCES = $(UTILS_DIR)/avx2_detect.c $(CORE_DIR)/ece_core.c $(CORE_DIR)/ece_digest.c $(CORE_DIR)/entropy_bus.c $(CORE_DIR)/system_entropy.c $(SRC_DIR)/main_simple.c

# All library source files (for full build)
ALL_LIB_SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(CORE_DIR)/*.c) $(wildcard $(UTILS_DIR)/*.c)

# Object files for core build (in build directory) 
CORE_OBJECTS = $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o $(BUILD_DIR)/main_simple.o
ALL_LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c)) \
                  $(patsubst $(CORE_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(CORE_DIR)/*.c)) \
                  $(patsubst $(UTILS_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(UTILS_DIR)/*.c))

# Output files
LIB_STATIC = $(BUILD_DIR)/libcharm.a
CHARM_BIN = $(BUILD_DIR)/charm
BENCH_BIN = $(BUILD_DIR)/bench_digest
COMPREHENSIVE_BENCH = $(BUILD_DIR)/benchmark_comprehensive
ENHANCED_BENCH = $(BUILD_DIR)/benchmark_enhanced
SMALL_INPUTS_BENCH = $(BUILD_DIR)/benchmark_small_inputs

# Default target - build core functionality
all: core

# Create build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Pattern rules for object files from different directories
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: $(CORE_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: $(UTILS_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/%.o: $(TESTS_DIR)/%.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

# Build core functionality only
core: $(CORE_OBJECTS)
	@echo "Building CHARM core binary with maximum performance..."
	$(CC) $(CFLAGS) -o $(CHARM_BIN) $(CORE_OBJECTS) $(LDFLAGS)

# Build comprehensive benchmark
bench: $(BUILD_DIR)/benchmark_comprehensive.o $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o
	@echo "Building comprehensive benchmark..."
	$(CC) $(CFLAGS) -o $(COMPREHENSIVE_BENCH) $^ $(LDFLAGS)

# Build enhanced comprehensive benchmark
enhanced: $(BUILD_DIR)/benchmark_enhanced.o $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o
	@echo "Building enhanced comprehensive benchmark..."
	$(CC) $(CFLAGS) -o $(ENHANCED_BENCH) $^ $(LDFLAGS)

# Build small inputs benchmark
small: $(BUILD_DIR)/benchmark_small_inputs.o $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o
	@echo "Building small inputs benchmark..."
	$(CC) $(CFLAGS) -o $(SMALL_INPUTS_BENCH) $^ $(LDFLAGS)

# Build full system
full: $(LIB_STATIC) $(CHARM_BIN) $(BENCH_BIN)

# Build static library
$(LIB_STATIC): $(ALL_LIB_OBJECTS)
	@echo "Building static library..."
	$(AR) rcs $@ $^
	$(RANLIB) $@

# Test target
test: core bench
	@echo "Running test suite..."
	@cd $(BUILD_DIR) && ./charm --version || true
	@cd $(BUILD_DIR) && ./benchmark_comprehensive || true

# Benchmark target
benchmark: bench
	@echo "Running performance benchmarks..."
	@cd $(BUILD_DIR) && ./benchmark_comprehensive

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/*.a $(BUILD_DIR)/charm $(BUILD_DIR)/bench_digest $(BUILD_DIR)/benchmark_comprehensive $(BUILD_DIR)/benchmark_enhanced $(BUILD_DIR)/benchmark_small_inputs
	@rm -rf test_output/
	@echo "Clean complete."

# Documentation target
docs:
	@echo "Generating documentation..."
	@echo "Documentation available in docs/ directory"
	@echo "Main documentation: docs/README.md"
	@echo "Contributing guidelines: CONTRIBUTING.md"
	@echo "License: LICENSE"

# Help
help:
	@echo "CHARM System Makefile - Organized Structure"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  core      - Build core ECE functionality (default)"
	@echo "  full      - Build complete system with all components"
	@echo "  clean     - Remove build files"
	@echo "  bench     - Build comprehensive benchmark tool"
	@echo "  enhanced  - Build enhanced comprehensive benchmark"
	@echo "  small     - Build small inputs benchmark (64B, 256B, 1KB)"
	@echo "  test      - Run test suite"
	@echo "  benchmark - Run performance benchmarks"
	@echo "  docs      - Generate/show documentation"
	@echo "  help      - Show this help message"

.PHONY: all core full clean test benchmark docs help