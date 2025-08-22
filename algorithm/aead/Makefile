# CHARM AEAD Makefile
# Builds CHARM AEAD implementation with comprehensive testing

CC = gcc
CFLAGS = -Wall -Wextra -O3 -flto -ffast-math -funroll-loops -mavx2 -march=native -mtune=native -fopenmp -fomit-frame-pointer -finline-functions -std=c99 -I./include -I../include
LDFLAGS = -lm -flto

# Directories
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests
BUILD_DIR = build

# Source files
AEAD_SOURCES = $(SRC_DIR)/charm_aead.c
ALGORITHM_SOURCES = ../src/charm_lib.c  # Main CHARM algorithm implementation
TEST_SOURCES = $(TEST_DIR)/charm_aead_tests.c
BENCHMARK_SOURCES = $(TEST_DIR)/charm_aead_benchmark_comparison.c

# Object files
AEAD_OBJECTS = $(BUILD_DIR)/charm_aead.o
ALGORITHM_OBJECTS = $(BUILD_DIR)/charm.o
TEST_OBJECTS = $(BUILD_DIR)/charm_aead_tests.o
BENCHMARK_OBJECTS = $(BUILD_DIR)/charm_aead_benchmark_comparison.o

# Targets
AEAD_LIB = $(BUILD_DIR)/libcharm_aead.a
TEST_BINARY = $(BUILD_DIR)/charm_aead_tests
BENCHMARK_BINARY = $(BUILD_DIR)/charm_aead_benchmark

.PHONY: all clean test benchmark lib install

all: lib test benchmark

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build static library
lib: $(AEAD_LIB)

$(AEAD_LIB): $(BUILD_DIR) $(AEAD_OBJECTS) $(ALGORITHM_OBJECTS)
	ar rcs $@ $(AEAD_OBJECTS) $(ALGORITHM_OBJECTS)
	@echo "Built CHARM AEAD library: $@"

# Build object files
$(BUILD_DIR)/charm_aead.o: $(SRC_DIR)/charm_aead.c $(INCLUDE_DIR)/charm_aead.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/charm.o: ../src/charm_lib.c ../include/charm.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build test binary
test: $(TEST_BINARY)

$(TEST_BINARY): $(BUILD_DIR) $(TEST_OBJECTS) $(AEAD_LIB)
	$(CC) $(CFLAGS) $(TEST_OBJECTS) -L$(BUILD_DIR) -lcharm_aead -L../../CHARM-B/build -lcharmb $(LDFLAGS) -o $@
	@echo "Built test binary: $@"

$(BUILD_DIR)/charm_aead_tests.o: $(TEST_DIR)/charm_aead_tests.c $(INCLUDE_DIR)/charm_aead.h
	$(CC) $(CFLAGS) -c $< -o $@

# Build benchmark binary
benchmark: $(BENCHMARK_BINARY)

$(BENCHMARK_BINARY): $(BUILD_DIR) $(BENCHMARK_OBJECTS) $(AEAD_LIB)
	$(CC) $(CFLAGS) $(BENCHMARK_OBJECTS) -L$(BUILD_DIR) -lcharm_aead -L../../CHARM-B/build -lcharmb_aead -lcharmb $(LDFLAGS) -o $@
	@echo "Built benchmark binary: $@"

$(BUILD_DIR)/charm_aead_benchmark_comparison.o: $(TEST_DIR)/charm_aead_benchmark_comparison.c
	$(CC) $(CFLAGS) -I../../CHARM-B/AEAD/include -c $< -o $@

# Run tests
run-tests: $(TEST_BINARY)
	@echo "Running CHARM AEAD tests..."
	./$(TEST_BINARY)

# Run benchmarks
run-benchmark: $(BENCHMARK_BINARY)
	@echo "Running CHARM AEAD benchmarks..."
	./$(BENCHMARK_BINARY)

# Install headers and library (optional)
install: $(AEAD_LIB)
	@echo "Installing CHARM AEAD..."
	mkdir -p /usr/local/include/charm
	mkdir -p /usr/local/lib
	cp $(INCLUDE_DIR)/charm_aead.h /usr/local/include/charm/
	cp $(AEAD_LIB) /usr/local/lib/
	@echo "Installation complete"

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

# Help target
help:
	@echo "CHARM AEAD Build System"
	@echo "======================="
	@echo ""
	@echo "Targets:"
	@echo "  all         - Build library, tests, and benchmarks"
	@echo "  lib         - Build static library only"
	@echo "  test        - Build test binary"
	@echo "  benchmark   - Build benchmark binary"
	@echo "  run-tests   - Build and run tests"
	@echo "  run-benchmark - Build and run benchmarks"
	@echo "  install     - Install library and headers"
	@echo "  clean       - Remove build artifacts"
	@echo "  help        - Show this help message"