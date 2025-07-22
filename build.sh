#!/bin/bash
# build.sh - Auto-detect CPU SIMD capabilities and configure build

# Set default build type
BUILD_TYPE="generic"
OUTPUT_DIR="build"
DEBUG=0
VERBOSE=0

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            DEBUG=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --output=*)
            OUTPUT_DIR="${1#*=}"
            shift
            ;;
        --build-type=*)
            BUILD_TYPE="${1#*=}"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--debug] [--verbose] [--output=DIR] [--build-type=TYPE]"
            echo "  --debug         Enable debug build"
            echo "  --verbose       Enable verbose output"
            echo "  --output=DIR    Set output directory (default: build)"
            echo "  --build-type=TYPE  Set build type (generic, avx2, neon, auto)"
            exit 1
            ;;
    esac
done

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

# Function to detect CPU features
detect_cpu_features() {
    if [[ $VERBOSE -eq 1 ]]; then
        echo "Detecting CPU features..."
    fi

    # Check for x86_64 architecture
    if [[ $(uname -m) == "x86_64" ]]; then
        # Check for AVX2 support
        if grep -q avx2 /proc/cpuinfo; then
            if [[ $VERBOSE -eq 1 ]]; then
                echo "AVX2 support detected"
            fi
            echo "avx2"
            return
        fi
    fi

    # Check for ARM architecture
    if [[ $(uname -m) == "aarch64" || $(uname -m) == "armv7l" ]]; then
        # Check for NEON support (most ARM64 processors have NEON)
        if grep -q neon /proc/cpuinfo || [[ $(uname -m) == "aarch64" ]]; then
            if [[ $VERBOSE -eq 1 ]]; then
                echo "NEON support detected"
            fi
            echo "neon"
            return
        fi
    fi

    # Default to generic
    if [[ $VERBOSE -eq 1 ]]; then
        echo "No SIMD extensions detected, using generic build"
    fi
    echo "generic"
}

# Auto-detect CPU features if requested
if [[ "$BUILD_TYPE" == "auto" ]]; then
    BUILD_TYPE=$(detect_cpu_features)
fi

# Create config.h with build settings
CONFIG_FILE="$OUTPUT_DIR/config.h"
echo "// Auto-generated configuration file" > "$CONFIG_FILE"
echo "#ifndef CHARM_CONFIG_H" >> "$CONFIG_FILE"
echo "#define CHARM_CONFIG_H" >> "$CONFIG_FILE"
echo "" >> "$CONFIG_FILE"

# Set build type defines
case $BUILD_TYPE in
    avx2)
        echo "#define CHARM_USE_AVX2 1" >> "$CONFIG_FILE"
        echo "#define CHARM_USE_NEON 0" >> "$CONFIG_FILE"
        ;;
    neon)
        echo "#define CHARM_USE_AVX2 0" >> "$CONFIG_FILE"
        echo "#define CHARM_USE_NEON 1" >> "$CONFIG_FILE"
        ;;
    *)
        echo "#define CHARM_USE_AVX2 0" >> "$CONFIG_FILE"
        echo "#define CHARM_USE_NEON 0" >> "$CONFIG_FILE"
        ;;
esac

# Set debug mode
if [[ $DEBUG -eq 1 ]]; then
    echo "#define CHARM_DEBUG 1" >> "$CONFIG_FILE"
else
    echo "#define CHARM_DEBUG 0" >> "$CONFIG_FILE"
fi

# Add timestamp
echo "#define CHARM_BUILD_TIMESTAMP \"$(date)\"" >> "$CONFIG_FILE"
echo "#define CHARM_VERSION \"2.0.0\"" >> "$CONFIG_FILE"

echo "" >> "$CONFIG_FILE"
echo "#endif // CHARM_CONFIG_H" >> "$CONFIG_FILE"

# Generate build.mk with compiler flags
BUILD_MK="$OUTPUT_DIR/build.mk"
echo "# Auto-generated build configuration" > "$BUILD_MK"
echo "" >> "$BUILD_MK"

# Set compiler and flags
echo "CC = gcc" >> "$BUILD_MK"
echo "AR = ar" >> "$BUILD_MK"
echo "RANLIB = ranlib" >> "$BUILD_MK"
echo "" >> "$BUILD_MK"

# Base compiler flags
echo "CFLAGS = -Wall -Wextra -std=c11 -I\$(TOPDIR)/include -I\$(BUILDDIR)" >> "$BUILD_MK"

# Add optimization flags
if [[ $DEBUG -eq 1 ]]; then
    echo "CFLAGS += -g -O0 -DDEBUG" >> "$BUILD_MK"
else
    echo "CFLAGS += -O2 -DNDEBUG" >> "$BUILD_MK"
fi

# Add architecture-specific flags
case $BUILD_TYPE in
    avx2)
        echo "CFLAGS += -mavx2 -DCHARM_USE_AVX2" >> "$BUILD_MK"
        ;;
    neon)
        echo "CFLAGS += -mfpu=neon -DCHARM_USE_NEON" >> "$BUILD_MK"
        ;;
esac

echo "" >> "$BUILD_MK"
echo "LDFLAGS = -pthread -lm" >> "$BUILD_MK"

# Print summary
echo "CHARM build configuration:"
echo "  Build type: $BUILD_TYPE"
echo "  Debug mode: $([ $DEBUG -eq 1 ] && echo "enabled" || echo "disabled")"
echo "  Output directory: $OUTPUT_DIR"
echo "  Configuration written to: $CONFIG_FILE"
echo "  Build settings written to: $BUILD_MK"

# Create a simple build info file
echo "Build type: $BUILD_TYPE" > "$OUTPUT_DIR/build_info.txt"
echo "Debug mode: $([ $DEBUG -eq 1 ] && echo "enabled" || echo "disabled")" >> "$OUTPUT_DIR/build_info.txt"
echo "Build timestamp: $(date)" >> "$OUTPUT_DIR/build_info.txt"
echo "Architecture: $(uname -m)" >> "$OUTPUT_DIR/build_info.txt"
echo "Compiler: $(gcc --version | head -n1)" >> "$OUTPUT_DIR/build_info.txt"

echo "Build configuration complete. Run 'make' to build the project."
