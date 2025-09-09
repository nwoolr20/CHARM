# Experimental Optimizations Makefile
# Builds benchmarking suite for CHARM experimental implementations

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O3 -flto -ffast-math -funroll-loops \
         -mavx2 -march=native -mtune=native -fopenmp \
         -fomit-frame-pointer -finline-functions -g
LDFLAGS = -lm -flto

# Directories
EXPERIMENTAL_DIR = .
AEAD_DIR = ../algorithm/aead
CHARM_B_DIR = ../CHARM-B

# Include paths
INCLUDES = -I$(AEAD_DIR)/include -I../algorithm/include -I$(CHARM_B_DIR)/include

# Libraries
LIBS = -L$(AEAD_DIR)/build -lcharm_aead -L$(CHARM_B_DIR)/build -lcharmb

# Targets
BENCHMARK_BIN = experimental_benchmark

.PHONY: all clean benchmark

all: $(BENCHMARK_BIN)

$(BENCHMARK_BIN): benchmark.c
	@echo "Building experimental benchmark suite..."
	$(CC) $(CFLAGS) $(INCLUDES) benchmark.c $(LIBS) $(LDFLAGS) -o $@
	@echo "Built: $@"

benchmark: $(BENCHMARK_BIN)
	@echo "Running experimental optimization benchmarks..."
	./$(BENCHMARK_BIN)

clean:
	@echo "Cleaning experimental directory..."
	rm -f $(BENCHMARK_BIN)
	@echo "Clean complete."

# Help target
help:
	@echo "Available targets:"
	@echo "  all        - Build benchmark suite"
	@echo "  benchmark  - Run performance benchmarks"
	@echo "  clean      - Clean build files"
	@echo "  help       - Show this help"
	@echo ""
	@echo "Experimental Projects:"
	@echo "  aeas/      - AEAS integration optimizations"
	@echo "  hybrid/    - Hybrid optimization system"