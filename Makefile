# Updated Makefile for CHARM System

# Compiler and flags
CC = gcc
AR = ar
RANLIB = ranlib
CFLAGS = -Wall -Wextra -std=c99 -O2 -mavx2 -I./include
LDFLAGS = -pthread -lm

# Directories
CORE_DIR = ./core
CLI_DIR = ./cli
DRIVERS_DIR = ./drivers
INCLUDE_DIR = ./include
BUILD_DIR = ./build
DOCS_DIR = ./docs
LOGS_DIR = ./logs
AUTOMATION_DIR = ./automation

# Source files
CORE_SRC = $(wildcard $(CORE_DIR)/*.c)
CLI_SRC = $(wildcard $(CLI_DIR)/*.c)
DRIVERS_SRC = $(wildcard $(DRIVERS_DIR)/*.c)

# Object files
CORE_OBJ = $(CORE_SRC:.c=.o)
CLI_OBJ = $(CLI_SRC:.c=.o)
DRIVERS_OBJ = $(DRIVERS_SRC:.c=.o)

# Output files
LIB_STATIC = libcharm.a
CHARM_BIN = charm

# Default target
all: prepare $(LIB_STATIC) $(CHARM_BIN)

# Prepare build directories
prepare:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LOGS_DIR)

# Build static library
$(LIB_STATIC): $(CORE_OBJ) $(DRIVERS_OBJ)
	$(AR) rcs $@ $^
	$(RANLIB) $@

# Build main executable
$(CHARM_BIN): $(CLI_OBJ) $(LIB_STATIC)
	$(CC) $(CFLAGS) -o $@ $(CLI_OBJ) -L. -lcharm $(LDFLAGS)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Run tests
test:
	@echo "Running CHARM test suite..."
	@bash $(AUTOMATION_DIR)/test_all.sh

# Install
install:
	@echo "Installing CHARM system..."
	@bash $(AUTOMATION_DIR)/install.sh

# Clean build files
clean:
	@echo "Cleaning build files..."
	@rm -f $(CORE_OBJ) $(CLI_OBJ) $(DRIVERS_OBJ) $(LIB_STATIC) $(CHARM_BIN)
	@rm -rf $(BUILD_DIR)/*
	@echo "Clean complete."

# Help
help:
	@echo "CHARM Makefile"
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build library and executable (default)"
	@echo "  clean     - Remove build files"
	@echo "  test      - Run test suite"
	@echo "  install   - Install to system"
	@echo "  help      - Show this help message"

.PHONY: all prepare clean test install help
