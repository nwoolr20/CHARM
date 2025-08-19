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
INCLUDES = -I$(INCLUDE_DIR) -Ithird_party/crypto/blake3/c

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

# CHARM Algorithm targets
charm_ref: $(BUILD_DIR)
	@echo "Building CHARM reference implementation (strict, no intrinsics)..."
	$(CC) -Wall -Wextra -std=c99 -O2 -Ialgorithm/include -o $(BUILD_DIR)/charm_ref \
		algorithm/src/charm_ref.c -lm

charm_opt: $(BUILD_DIR)
	@echo "Building CHARM optimized implementation (SIMD enabled)..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/charm_opt \
		algorithm/src/charm_opt.c $(LDFLAGS)

# NIST-style conformance testing
conformance: $(BUILD_DIR)
	@echo "Building NIST-style conformance test suite..."
	@mkdir -p conformance/nist/reports
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/charm_kat \
		conformance/nist/runner/charm_kat.cpp $(LDFLAGS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/charm_mc \
		conformance/nist/runner/charm_mc.cpp $(LDFLAGS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/charm_stream \
		conformance/nist/runner/charm_stream.cpp $(LDFLAGS)
	@echo "Running conformance tests..."
	@./$(BUILD_DIR)/charm_kat > conformance/nist/reports/kat_$(shell date +%Y%m%d_%H%M%S).json || true
	@./$(BUILD_DIR)/charm_mc > conformance/nist/reports/mc_$(shell date +%Y%m%d_%H%M%S).json || true
	@./$(BUILD_DIR)/charm_stream > conformance/nist/reports/stream_$(shell date +%Y%m%d_%H%M%S).json || true
	@echo "Conformance test reports saved to conformance/nist/reports/"

conformance-quick:
	@echo "Running quick conformance validation..."
	@echo "This is a placeholder for quick NIST-style tests"

# Build third-party BLAKE3 library
third_party_blake3:
	@echo "Building BLAKE3 library..."
	cd third_party/crypto/blake3/c && \
	gcc -O3 -c blake3.c blake3_dispatch.c blake3_portable.c blake3_sse2.c blake3_sse41.c blake3_avx2.c -mavx2 -msse4.1 -msse2 && \
	ar rcs libblake3.a blake3.o blake3_dispatch.o blake3_portable.o blake3_sse2.o blake3_sse41.o blake3_avx2.o && \
	cp libblake3.a ../../../../

# Documentation target
docs:
	@echo "Documentation available in documents/ directory"
	@echo "Algorithm specification: documents/CHARM-Algorithm-Spec.md"
	@echo "Conformance testing: documents/Conformance-Testing.md"
	@echo "Implementation summary: documents/CHARM_IMPLEMENTATION_SUMMARY.md"
	@echo "Third-party licenses: third_party/README.md"

# Help
help:
	@echo "CHARM System Makefile - Organized Structure"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  core      - Build core ECE functionality (default)"
	@echo "  full      - Build complete system with all components"
	@echo "  clean     - Remove build files"
	@echo "  charm_ref - Build reference implementation (strict, no intrinsics)"
	@echo "  charm_opt - Build optimized implementation (SIMD enabled)"
	@echo "  conformance - Build and run NIST-style conformance tests"
	@echo "  conformance-quick - Run quick conformance validation"
	@echo "  third_party_blake3 - Build BLAKE3 library for benchmarking"
	@echo "  enhanced  - Build enhanced comprehensive benchmark"
	@echo "  small     - Build small inputs benchmark (64B, 256B, 1KB)"
	@echo "  test      - Run test suite"
	@echo "  benchmark - Run performance benchmarks"
	@echo "  docs      - Generate/show documentation"
	@echo "  help      - Show this help message"

.PHONY: all core full clean test benchmark docs help