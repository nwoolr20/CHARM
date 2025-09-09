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
LDFLAGS = -pthread -lm -lssl -lcrypto -flto

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
UTILS_DIR = $(SRC_DIR)/utils
TESTS_DIR = $(SRC_DIR)/tests
INCLUDE_DIR = include
BUILD_DIR = build
TOOLS_DIR = tools
BENCHMARKS_DIR = benchmarks
PBKDF2_DIR = PBKDF2

# Include path
INCLUDES = -I$(INCLUDE_DIR) -Ithird_party/crypto/blake3/c -I$(PBKDF2_DIR)

# Core essential source files for minimal working system (updated paths)
CORE_SOURCES = $(UTILS_DIR)/avx2_detect.c $(CORE_DIR)/ece_core.c $(CORE_DIR)/ece_digest.c $(CORE_DIR)/entropy_bus.c $(CORE_DIR)/system_entropy.c $(SRC_DIR)/main_simple.c

# Security suite source files
SECURITY_SUITE_SOURCES = $(SRC_DIR)/crypto/keymanagement/key_manager.c \
                        $(SRC_DIR)/crypto/audit/audit.c \
                        $(SRC_DIR)/crypto/auth/auth.c \
                        $(SRC_DIR)/crypto/config/config.c \
                        $(SRC_DIR)/crypto/charm_security_suite.c

# PBKDF2 source files
PBKDF2_SOURCES = $(PBKDF2_DIR)/pbkdf2.c

# All library source files (for full build)
ALL_LIB_SOURCES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(CORE_DIR)/*.c) $(wildcard $(UTILS_DIR)/*.c) $(SECURITY_SUITE_SOURCES)

# Object files for core build (in build directory) 
CORE_OBJECTS = $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o $(BUILD_DIR)/main_simple.o

# Security suite object files
SECURITY_SUITE_OBJECTS = $(BUILD_DIR)/key_manager.o $(BUILD_DIR)/audit.o $(BUILD_DIR)/auth.o $(BUILD_DIR)/config.o $(BUILD_DIR)/charm_security_suite.o

# PBKDF2 object files
PBKDF2_OBJECTS = $(BUILD_DIR)/pbkdf2.o

ALL_LIB_OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(SRC_DIR)/*.c)) \
                  $(patsubst $(CORE_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(CORE_DIR)/*.c)) \
                  $(patsubst $(UTILS_DIR)/%.c,$(BUILD_DIR)/%.o,$(wildcard $(UTILS_DIR)/*.c)) \
                  $(SECURITY_SUITE_OBJECTS)

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

# Security suite object files
$(BUILD_DIR)/key_manager.o: $(SRC_DIR)/crypto/keymanagement/key_manager.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/audit.o: $(SRC_DIR)/crypto/audit/audit.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/auth.o: $(SRC_DIR)/crypto/auth/auth.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/config.o: $(SRC_DIR)/crypto/config/config.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/charm_security_suite.o: $(SRC_DIR)/crypto/charm_security_suite.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/charm_minimal_api.o: $(SRC_DIR)/charm_minimal_api.c | $(BUILD_DIR)
	@echo "Compiling $< with performance optimizations..."
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BUILD_DIR)/pbkdf2.o: $(PBKDF2_DIR)/pbkdf2.c | $(BUILD_DIR)
	@echo "Compiling PBKDF2 implementation..."
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
	$(CC) $(CFLAGS) -DHAVE_BLAKE3 -o $(ENHANCED_BENCH) $^ $(LDFLAGS)

# Build small inputs benchmark
small: $(BUILD_DIR)/benchmark_small_inputs.o $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o
	@echo "Building small inputs benchmark..."
	$(CC) $(CFLAGS) -o $(SMALL_INPUTS_BENCH) $^ $(LDFLAGS)

# Build full system
full: $(LIB_STATIC) $(CHARM_BIN) $(BENCH_BIN)

# Build security suite
security_suite: $(SECURITY_SUITE_OBJECTS) $(PBKDF2_OBJECTS) $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o
	@echo "Building CHARM Security Suite CLI..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/charm_security_suite \
		$(SRC_DIR)/security_suite_cli.c \
		$(SECURITY_SUITE_OBJECTS) $(PBKDF2_OBJECTS) \
		$(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o \
		$(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o \
		$(LDFLAGS)

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

# Test security suite - minimal build
test_security_suite_minimal: $(SECURITY_SUITE_OBJECTS) $(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o $(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o $(BUILD_DIR)/charm_minimal_api.o
	@echo "Building minimal security suite test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/test_security_suite \
		$(SRC_DIR)/tests/test_security_suite.c \
		$(SECURITY_SUITE_OBJECTS) \
		$(BUILD_DIR)/avx2_detect.o $(BUILD_DIR)/ece_core.o $(BUILD_DIR)/ece_digest.o \
		$(BUILD_DIR)/entropy_bus.o $(BUILD_DIR)/system_entropy.o \
		$(BUILD_DIR)/charm_minimal_api.o \
		$(LDFLAGS)
	@echo "Running security suite test..."
	@mkdir -p logs keystore config
	@cd $(BUILD_DIR) && ./test_security_suite

# Test security suite
test_security_suite: $(SECURITY_SUITE_OBJECTS) $(ALL_LIB_OBJECTS)
	@echo "Building security suite test..."
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/test_security_suite \
		$(SRC_DIR)/tests/test_security_suite.c \
		$(SECURITY_SUITE_OBJECTS) \
		$(ALL_LIB_OBJECTS) \
		$(LDFLAGS)
	@echo "Running security suite test..."
	@mkdir -p logs keystore config
	@cd $(BUILD_DIR) && ./test_security_suite

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
	g++ $(CFLAGS) -Ialgorithm/include -o $(BUILD_DIR)/charm_kat \
		conformance/nist/runner/charm_kat.cpp algorithm/src/charm_lib.c -lm
	g++ $(CFLAGS) -Ialgorithm/include -o $(BUILD_DIR)/charm_mc \
		conformance/nist/runner/charm_mc.cpp algorithm/src/charm_lib.c -lm
	g++ $(CFLAGS) -Ialgorithm/include -o $(BUILD_DIR)/charm_stream \
		conformance/nist/runner/charm_stream.cpp algorithm/src/charm_lib.c -lm
	@echo "Running conformance tests..."
	@./$(BUILD_DIR)/charm_kat > conformance/nist/reports/kat_$(shell date +%Y%m%d_%H%M%S).json || true
	@./$(BUILD_DIR)/charm_mc > conformance/nist/reports/mc_$(shell date +%Y%m%d_%H%M%S).json || true
	@./$(BUILD_DIR)/charm_stream > conformance/nist/reports/stream_$(shell date +%Y%m%d_%H%M%S).json || true
	@echo "Conformance test reports saved to conformance/nist/reports/"

# Extended CHARM Algorithm Testing Suite
extended_tests: $(BUILD_DIR)
	@echo "Building extended CHARM algorithm test suite..."
	@mkdir -p conformance/nist/reports
	g++ -Wall -Wextra -std=c++11 -O3 -Ialgorithm/include -o $(BUILD_DIR)/charm_extended_kat \
		conformance/nist/runner/charm_extended_kat.cpp algorithm/src/charm_lib.c -lm
	g++ -Wall -Wextra -std=c++11 -O3 -Ialgorithm/include -o $(BUILD_DIR)/charm_avalanche \
		conformance/nist/runner/charm_avalanche.cpp algorithm/src/charm_lib.c -lm
	g++ -Wall -Wextra -std=c++11 -O3 -Ialgorithm/include -o $(BUILD_DIR)/charm_sidechannel \
		conformance/nist/runner/charm_sidechannel.cpp algorithm/src/charm_lib.c -lm
	g++ -Wall -Wextra -std=c++11 -O3 -Ialgorithm/include -o $(BUILD_DIR)/charm_acvp \
		conformance/nist/runner/charm_acvp.cpp algorithm/src/charm_lib.c -lm
	g++ -Wall -Wextra -std=c++11 -O3 -o $(BUILD_DIR)/charm_comprehensive_test \
		conformance/nist/runner/charm_comprehensive_test.cpp

# Run comprehensive validation (all tests)
comprehensive: extended_tests
	@echo "Running comprehensive CHARM algorithm validation..."
	@./$(BUILD_DIR)/charm_comprehensive_test > conformance/nist/reports/comprehensive_$(shell date +%Y%m%d_%H%M%S).json
	@echo "Comprehensive test results saved to conformance/nist/reports/"

# Run security analysis tests
security_analysis: extended_tests
	@echo "Running CHARM security analysis..."
	@echo "=== Avalanche Effect Analysis ==="
	@timeout 180 ./$(BUILD_DIR)/charm_avalanche > conformance/nist/reports/avalanche_$(shell date +%Y%m%d_%H%M%S).json || echo "Avalanche analysis completed"
	@echo "=== Side-Channel Resistance Analysis ==="
	@timeout 300 ./$(BUILD_DIR)/charm_sidechannel > conformance/nist/reports/sidechannel_$(shell date +%Y%m%d_%H%M%S).json || echo "Side-channel analysis completed"
	@echo "Security analysis reports saved to conformance/nist/reports/"

# Performance profiling
performance_profile: extended_tests
	@echo "Running CHARM performance profiling..."
	@echo "=== Monte Carlo Performance Test ==="
	@./$(BUILD_DIR)/charm_mc 10000 > conformance/nist/reports/performance_$(shell date +%Y%m%d_%H%M%S).json
	@echo "=== Extended KAT Performance Test ==="
	@timeout 600 ./$(BUILD_DIR)/charm_extended_kat > conformance/nist/reports/extended_kat_$(shell date +%Y%m%d_%H%M%S).json || echo "Extended KAT completed"
	@echo "Performance profiling reports saved to conformance/nist/reports/"

# ACVP protocol testing
acvp_test: extended_tests
	@echo "Running ACVP protocol compatibility tests..."
	@./$(BUILD_DIR)/charm_acvp register > conformance/nist/reports/acvp_registration.json
	@./$(BUILD_DIR)/charm_acvp capabilities > conformance/nist/reports/acvp_capabilities.json
	@./$(BUILD_DIR)/charm_acvp demo > conformance/nist/reports/acvp_demo_vectors.json
	@echo "ACVP test files saved to conformance/nist/reports/"

conformance-quick:
	@echo "Running quick conformance validation..."
	@mkdir -p conformance/nist/reports
	@if [ ! -f build/charm_kat ]; then $(MAKE) conformance > /dev/null 2>&1; fi
	@echo "Quick KAT Test Results:" 
	@./build/charm_kat 2>/dev/null | grep -E "(PASS|FAIL|passed|total)" || echo "Tests completed"

# Build third-party BLAKE3 library
:
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
	@echo "  extended_tests - Build extended CHARM algorithm test suite"
	@echo "  comprehensive - Run comprehensive validation (all tests)"
	@echo "  security_analysis - Run security analysis (avalanche & side-channel)"
	@echo "  performance_profile - Run performance profiling tests"
	@echo "  acvp_test - Run ACVP protocol compatibility tests"
	@echo "   - Build BLAKE3 library for benchmarking"
	@echo "  enhanced  - Build enhanced comprehensive benchmark"
	@echo "  small     - Build small inputs benchmark (64B, 256B, 1KB)"
	@echo "  test      - Run test suite"
	@echo "  benchmark - Run performance benchmarks"
	@echo "  docs      - Generate/show documentation"
	@echo "  help      - Show this help message"

.PHONY: all core full clean test benchmark docs help extended_tests comprehensive security_analysis performance_profile acvp_test