# CHARM Algorithm Test Suite Documentation

## Overview

This document provides comprehensive documentation for the **CHARM Algorithm test suite**, which is distinct from the previous CHARM engine testing framework. The algorithm test suite focuses specifically on validating the frozen CHARM/1.0 specification through NIST-style conformance testing.

---

## Test Suite Architecture

### Directory Structure

```
conformance/nist/
├── README.md                     # Testing methodology guide
├── schemas/
│   └── acvp-sha.json            # ACVP compatibility schema
├── runner/
│   ├── charm_kat.cpp            # Known Answer Tests
│   ├── charm_mc.cpp             # Monte Carlo Tests
│   └── charm_stream.cpp         # Streaming Tests
└── reports/                     # Generated test reports
```

### Algorithm Implementation Structure

```
algorithm/
├── include/
│   └── charm.h                  # Frozen API specification (CHARM/1.0)
└── src/
    ├── charm_ref.c              # Reference implementation with main()
    └── charm_lib.c              # Library implementation for testing
```

---

## Test Categories

### 1. Known Answer Tests (KAT)

**File**: `conformance/nist/runner/charm_kat.cpp`

**Purpose**: Validate exact algorithm implementation against reference vectors

**Test Vectors**:
```cpp
// Empty string tests
CHARM-256("") → Expected output
CHARM-384("") → Expected output  
CHARM-512("") → Expected output

// Standard test string
CHARM-256("abc") → Expected output
CHARM-384("abc") → Expected output
CHARM-512("abc") → Expected output
```

**Output Format**:
```json
{
  "timestamp": "1755627259",
  "test_type": "KAT", 
  "algorithm": "CHARM",
  "version": "1.0",
  "CHARM-256_empty": "PASS",
  "CHARM-384_empty": "PASS",
  "CHARM-512_empty": "PASS",
  "CHARM-256_abc": "PASS", 
  "CHARM-384_abc": "PASS",
  "CHARM-512_abc": "PASS",
  "summary": {
    "passed": 6,
    "total": 6,
    "status": "PASS"
  }
}
```

**Key Features**:
- Validates all three CHARM variants (256, 384, 512)
- Tests both empty and non-empty inputs
- Produces ACVP-compatible JSON output
- Generates reproducible test vectors

### 2. Monte Carlo Tests

**File**: `conformance/nist/runner/charm_mc.cpp`

**Purpose**: Statistical validation through long-run chained hashes

**Algorithm**:
```cpp
// Initialize with fixed seed for reproducibility
seed = 0x42 repeated to digest_size
for (i = 0; i < 10000; i++) {
    digest = CHARM(seed || iteration_data)
    seed = digest  // Chain for next iteration
}
```

**Output Format**:
```json
{
  "timestamp": "1755627304",
  "test_type": "Monte_Carlo",
  "algorithm": "CHARM", 
  "version": "1.0",
  "iterations": 10000,
  "CHARM-256": {
    "status": "PASS",
    "duration_ms": 182
  },
  "CHARM-384": {
    "status": "PASS",
    "duration_ms": 142  
  },
  "CHARM-512": {
    "status": "PASS",
    "duration_ms": 230
  },
  "summary": {
    "passed": 3,
    "total": 3,
    "status": "PASS"
  }
}
```

**Key Features**:
- 10,000 iterations per variant for statistical significance
- Chained hash pattern to detect state flaws
- Performance timing for each variant
- Reproducible using fixed seed values

### 3. Streaming Tests

**File**: `conformance/nist/runner/charm_stream.cpp`

**Purpose**: Validate identical results regardless of input chunking patterns

**Test Patterns**:
```cpp
// Test data: 1KB of structured data
test_data = repeated pattern of bytes

// Chunking patterns tested:
1. Single byte chunks (1000 updates)
2. Small chunks (7-byte chunks)  
3. Block boundary (64-byte chunks)
4. Mixed sizes (varying chunk sizes)
5. Large chunks (256-byte chunks)
```

**Output Format**:
```json
{
  "timestamp": "1755627305",
  "test_type": "Streaming",
  "algorithm": "CHARM",
  "version": "1.0", 
  "CHARM-256": {
    "single_byte": "PASS",
    "small_chunks": "PASS",
    "block_boundary": "PASS",
    "mixed_sizes": "PASS",
    "large_chunk": "PASS"
  },
  // ... similar for CHARM-384 and CHARM-512
  "summary": {
    "passed": 15,
    "total": 15,
    "status": "PASS"
  }
}
```

**Key Features**:
- Tests all chunking patterns against single-shot processing
- Validates streaming API consistency
- Covers edge cases around block boundaries (64 bytes)
- Ensures deterministic behavior across update patterns

---

## Build Instructions

### Prerequisites
- GCC or compatible C++ compiler
- C++11 standard support
- CHARM algorithm implementation files

### Compilation Commands

```bash
# Build Known Answer Tests
g++ -o conformance/nist/runner/charm_kat \
    conformance/nist/runner/charm_kat.cpp \
    algorithm/src/charm_lib.c \
    -I algorithm/include \
    -std=c++11

# Build Monte Carlo Tests  
g++ -o conformance/nist/runner/charm_mc \
    conformance/nist/runner/charm_mc.cpp \
    algorithm/src/charm_lib.c \
    -I algorithm/include \
    -std=c++11

# Build Streaming Tests
g++ -o conformance/nist/runner/charm_stream \
    conformance/nist/runner/charm_stream.cpp \
    algorithm/src/charm_lib.c \
    -I algorithm/include \
    -std=c++11
```

### Execution

```bash
# Run individual test suites
./conformance/nist/runner/charm_kat
./conformance/nist/runner/charm_mc  
./conformance/nist/runner/charm_stream

# Or use Makefile targets (when available)
make conformance        # Run all conformance tests
make conformance-quick  # Run quick validation
```

---

## ACVP Compatibility

### JSON Schema Alignment

The test framework follows ACVP (Automated Cryptographic Validation Protocol) patterns:

**Schema Reference**: `conformance/nist/schemas/acvp-sha.json`

**Input Format** (ACVP-style):
```json
{
  "algorithm": "CHARM",
  "version": "1.0",
  "testGroups": [
    {
      "testType": "AFT",
      "digestSize": 256,
      "tests": [
        {
          "tcId": 1,
          "msg": "",
          "len": 0
        }
      ]
    }
  ]
}
```

**Output Format** (ACVP-style):
```json
{
  "algorithm": "CHARM",
  "version": "1.0", 
  "testGroups": [
    {
      "tests": [
        {
          "tcId": 1,
          "md": "8af52df373396446f721be8f9b205b9533cb6561f6a508338784a8c0c1335c14"
        }
      ]
    }
  ]
}
```

### Tool Integration

The ACVP-compatible format enables integration with:
- Automated validation tools
- Cryptographic test harnesses  
- Continuous integration pipelines
- Third-party validation services

---

## API Specification

### CHARM Algorithm Interface

**Header**: `algorithm/include/charm.h`

**Core Functions**:
```c
// Context management
charm_ctx_t* charm_init(charm_variant_t variant);
void charm_free(charm_ctx_t* ctx);

// Streaming interface
int charm_update(charm_ctx_t* ctx, const void* data, size_t len);
int charm_finalize(charm_ctx_t* ctx, void* output);

// One-shot interface  
int charm_hash(charm_variant_t variant, const void* input, 
               size_t input_len, void* output);
```

**Variants**:
```c
typedef enum {
    CHARM_256 = 256,    // 32-byte output
    CHARM_384 = 384,    // 48-byte output  
    CHARM_512 = 512     // 64-byte output
} charm_variant_t;
```

**Constants**:
```c
#define CHARM_BLOCK_BYTES 64   // Block size for processing
#define CHARM_MAX_OUTPUT  64   // Maximum output size (512-bit)
```

---

## Implementation Details

### Reference Implementation

**File**: `algorithm/src/charm_ref.c`
- Complete CHARM algorithm with main() function
- Used for standalone testing and validation
- Normative implementation for specification compliance

### Library Implementation

**File**: `algorithm/src/charm_lib.c`  
- CHARM algorithm without main() function
- Suitable for linking with test harnesses
- Used by all conformance test runners

### Key Differences from Engine Testing

| Aspect | Algorithm Testing | Engine Testing |
|--------|------------------|----------------|
| **Focus** | Correctness validation | Performance measurement |
| **Scope** | CHARM/1.0 specification | Full CHARM system |
| **Methods** | NIST-style conformance | Benchmark comparisons |
| **Output** | Pass/fail validation | Throughput metrics |
| **Purpose** | Certification readiness | Optimization guidance |

---

## Test Execution Examples

### Running Complete Test Suite

```bash
# Navigate to repository root
cd /path/to/CHARM

# Build all test runners
make conformance-build  # If available, or use manual compilation

# Execute tests with output capture
./conformance/nist/runner/charm_kat > kat_results.json
./conformance/nist/runner/charm_mc > mc_results.json  
./conformance/nist/runner/charm_stream > stream_results.json

# Verify all tests passed
grep '"status": "PASS"' *.json
```

### Automated Testing Script

```bash
#!/bin/bash
# charm_algorithm_test.sh

echo "CHARM Algorithm Conformance Testing"
echo "==================================="

# Build tests
echo "Building test runners..."
g++ -o conformance/nist/runner/charm_kat conformance/nist/runner/charm_kat.cpp algorithm/src/charm_lib.c -I algorithm/include -std=c++11
g++ -o conformance/nist/runner/charm_mc conformance/nist/runner/charm_mc.cpp algorithm/src/charm_lib.c -I algorithm/include -std=c++11  
g++ -o conformance/nist/runner/charm_stream conformance/nist/runner/charm_stream.cpp algorithm/src/charm_lib.c -I algorithm/include -std=c++11

# Execute tests
echo "Running KAT tests..."
./conformance/nist/runner/charm_kat

echo "Running Monte Carlo tests..."
./conformance/nist/runner/charm_mc

echo "Running Streaming tests..."  
./conformance/nist/runner/charm_stream

echo "All tests completed."
```

---

## Quality Assurance

### Test Coverage

**Algorithm Variants**: 
- ✅ CHARM-256 (32-byte output)
- ✅ CHARM-384 (48-byte output)  
- ✅ CHARM-512 (64-byte output)

**Input Patterns**:
- ✅ Empty strings
- ✅ Short strings ("abc")
- ✅ Long sequences (1KB+ data)
- ✅ Various chunking patterns

**Validation Methods**:
- ✅ Known Answer Tests (exact output validation)
- ✅ Monte Carlo Tests (statistical validation)
- ✅ Streaming Tests (chunking invariance)

### Reproducibility

All tests use:
- **Fixed Seeds**: Deterministic random number generation
- **Fixed Inputs**: Consistent test vectors across runs
- **Timestamped Output**: Traceable test execution
- **Version Tracking**: CHARM/1.0 specification adherence

---

## Security Notes

### Validation Scope

This test suite validates:
- ✅ **Algorithmic Correctness**: Implementation matches specification
- ✅ **Deterministic Behavior**: Consistent outputs for identical inputs
- ✅ **Streaming Consistency**: Chunking-invariant results
- ✅ **Statistical Properties**: Monte Carlo validation passes

### Limitations

This test suite does NOT provide:
- ❌ **FIPS Validation**: Not an official NIST validation
- ❌ **Security Analysis**: No cryptanalysis or attack resistance testing
- ❌ **Performance Optimization**: No throughput or latency optimization
- ❌ **Side-Channel Analysis**: No timing or power analysis protection

### Intended Use

The CHARM Algorithm test suite is designed for:
- **Academic Research**: Validating research implementations
- **Commercial Evaluation**: Ensuring implementation correctness
- **Standardization**: Demonstrating specification compliance  
- **Third-Party Testing**: Providing reference validation methodology

---

## Future Enhancements

### Planned Additions

1. **Extended Test Vectors**: Additional KAT patterns beyond empty/"abc"
2. **Bit-Flip Tests**: Avalanche effect validation
3. **Variable-Length Tests**: Systematic length testing (0-1MB)
4. **Performance Profiling**: Execution time analysis per test
5. **ACVP Adapter**: Full ACVP protocol implementation

### Integration Opportunities

- **CI/CD Integration**: Automated testing in build pipelines
- **Fuzzing Integration**: Property-based testing with QuickCheck-style tools
- **Benchmark Integration**: Combined correctness and performance testing
- **Documentation Generation**: Automated test report generation

---

## Conclusion

The CHARM Algorithm test suite provides comprehensive validation of the CHARM/1.0 specification through NIST-equivalent testing methodology. This framework ensures algorithmic correctness, implementation quality, and certification readiness while maintaining clear distinction from performance-focused engine testing.

**Key Achievements**:
- ✅ Complete NIST-style conformance testing framework
- ✅ ACVP-compatible JSON interfaces  
- ✅ All algorithm variants validated (256/384/512)
- ✅ Comprehensive test coverage (KAT/MC/Streaming)
- ✅ Reproducible and deterministic test execution

The test suite positions CHARM for academic research, commercial adoption, and potential standardization activities by demonstrating methodological parity with established cryptographic validation approaches.