# CHARM Conformance Testing Guide

This document describes how to run the NIST-style conformance test harness for the CHARM algorithm and provides guidance for validation and certification activities.

## Overview

The CHARM conformance testing framework implements validation methodologies equivalent to those used by NIST for SHA-2 (FIPS 180-4), SHA-3 (FIPS 202), and derived functions (SP 800-185). While CHARM is not a FIPS-approved algorithm, this framework demonstrates methodological parity with NIST validation approaches.

## Test Categories

### 1. Known Answer Tests (KATs)

**Purpose**: Validate exact algorithm implementation against reference vectors  
**Coverage**: All CHARM variants (256, 384, 512) in keyed and unkeyed modes  
**Pattern**: Input→Expected Output validation

```bash
# Run KAT tests for all variants
./conformance/nist/runner/charm_kat

# Run KATs for specific variant  
./conformance/nist/runner/charm_kat --variant 256

# Generate new KAT vectors (reference implementation)
./conformance/nist/runner/charm_kat --generate --output kat_vectors.json
```

**Test Inputs Include**:
- Empty string
- Single byte inputs (0x00 through 0xFF)
- Standard test strings ("abc", "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
- Block boundary tests (63, 64, 65 bytes)
- Chunk boundary tests (16383, 16384, 16385 bytes)
- Large inputs (1MB random data)

### 2. Monte Carlo Tests

**Purpose**: Detect statistical weaknesses and state periodicity  
**Pattern**: H_{i+1} = CHARM(H_i || seed_i) for 10,000-100,000 iterations

```bash
# Run Monte Carlo validation
./conformance/nist/runner/charm_mc --iterations 10000

# Extended Monte Carlo (research grade)
./conformance/nist/runner/charm_mc --iterations 100000 --variants all
```

**Analysis**:
- Chi-square tests on output distribution
- Autocorrelation analysis
- Period detection
- Entropy measurements of output sequence

### 3. Variable-Length Message Tests

**Purpose**: Validate algorithm behavior across input sizes  
**Coverage**: 0 to 1MB with special attention to boundary conditions

```bash
# Run variable-length tests
./conformance/nist/runner/charm_varlen

# Focus on specific size ranges
./conformance/nist/runner/charm_varlen --min-size 0 --max-size 1024
```

**Size Classes**:
- Tiny: 0-16 bytes (empty string handling)
- Small: 17-128 bytes (sub-block processing)  
- Medium: 129-16KB (multi-block, single chunk)
- Large: 16KB+ (multi-chunk processing)
- Boundaries: Exact block/chunk size transitions

### 4. Bit-Level Avalanche Tests

**Purpose**: Verify cryptographic avalanche properties  
**Method**: Flip each bit of base message, measure output differences

```bash
# Run avalanche analysis
./conformance/nist/runner/charm_avalanche

# Detailed statistical analysis
./conformance/nist/runner/charm_avalanche --detailed --output avalanche_report.json
```

**Metrics**:
- Strict Avalanche Criterion (SAC): Each bit flip changes ≥50% of output bits
- Bit Independence Criterion (BIC): Output bit pairs are statistically independent
- Avalanche distribution: Statistical analysis of difference patterns

### 5. Streaming Invariance Tests

**Purpose**: Verify implementation independence from input chunking  
**Method**: Compare single-shot vs. streaming with various chunk sizes

```bash
# Run streaming tests
./conformance/nist/runner/charm_stream

# Test specific chunk patterns
./conformance/nist/runner/charm_stream --patterns "1,31,64,1024,16384"
```

**Chunk Patterns**:
- Single byte: Maximum update() calls
- Prime sizes: 31, 127, 251 bytes (detect alignment issues)
- Block boundaries: 63, 64, 65 bytes
- Chunk boundaries: 16383, 16384, 16385 bytes
- Random splits: Verify robustness

### 6. Determinism Verification

**Purpose**: Ensure reproducible results across platforms and implementations  
**Coverage**: Environment independence and parameter stability

```bash
# Run determinism tests
./conformance/nist/runner/charm_determinism

# Cross-platform validation
./conformance/nist/runner/charm_determinism --iterations 1000 --log-state
```

**Validation**:
- Same input always produces identical output
- No dependency on system endianness
- No dependency on CPU features or timing
- Consistent behavior across compiler optimizations

## ACVP Compatibility

### JSON Interface

The conformance framework provides ACVP-compatible JSON interfaces:

```bash
# Generate ACVP-style test vectors
python3 conformance/nist/runner/acvp_adapter.py \
    --operation generate \
    --algorithm CHARM \
    --mode hash \
    --variants 256,384,512 \
    --output acvp_vectors.json

# Process ACVP response format
python3 conformance/nist/runner/acvp_adapter.py \
    --operation validate \
    --input acvp_responses.json \
    --output validation_results.json
```

### Schema Compatibility

Test vectors follow ACVP SHA schema patterns:
- `testGroups`: Organized by variant and test type
- `tests`: Individual test cases with `msg` and `md` fields
- `responses`: Implementation responses for validation

Example ACVP-compatible test vector:
```json
{
    "vsId": 1,
    "algorithm": "CHARM",
    "mode": "hash",
    "revision": "1.0",
    "testGroups": [{
        "tgId": 1,
        "variant": "CHARM-256",
        "tests": [{
            "tcId": 1,
            "msg": "",
            "len": 0,
            "md": "a1b2c3d4e5f6789012345678901234567890abcdef012345678901234567890"
        }]
    }]
}
```

## Running the Complete Test Suite

### Quick Validation
```bash
# Run essential tests (5-10 minutes)
make conformance-quick
```

### Full Validation  
```bash
# Run complete test suite (30-60 minutes)
make conformance
```

### Research-Grade Validation
```bash
# Extended testing for certification (2-4 hours)
make conformance-extended
```

## Test Reports

### Machine-Readable Output
```json
{
    "timestamp": "2024-01-01T00:00:00Z",
    "algorithm": "CHARM",
    "version": "1.0",
    "test_results": {
        "kat": {"passed": 1250, "failed": 0, "status": "PASS"},
        "monte_carlo": {"iterations": 10000, "status": "PASS"},
        "avalanche": {"sac_score": 0.52, "bic_score": 0.48, "status": "PASS"},
        "streaming": {"patterns": 15, "mismatches": 0, "status": "PASS"},
        "determinism": {"trials": 1000, "consistency": 1.0, "status": "PASS"}
    },
    "overall_status": "PASS",
    "coverage": {
        "variants": ["CHARM-256", "CHARM-384", "CHARM-512"],
        "modes": ["unkeyed", "keyed"],
        "total_vectors": 3750
    }
}
```

### Human-Readable Summary
```
CHARM Conformance Test Report
============================
Date: 2024-01-01 00:00:00 UTC
Algorithm: CHARM v1.0
Duration: 45 minutes

Test Results:
✓ Known Answer Tests: 1,250 vectors PASSED
✓ Monte Carlo Tests: 10,000 iterations PASSED  
✓ Avalanche Analysis: SAC=0.52, BIC=0.48 PASSED
✓ Streaming Tests: 15 patterns PASSED
✓ Determinism Tests: 1,000 trials PASSED

Coverage:
- Variants: CHARM-256, CHARM-384, CHARM-512
- Modes: Unkeyed, Keyed
- Total Test Vectors: 3,750
- Input Sizes: 0 bytes to 1 MB

Overall Status: ✓ PASSED

All tests completed successfully. CHARM implementation 
demonstrates compliance with NIST-style validation 
methodology equivalent to FIPS 180-4 and FIPS 202 standards.
```

## Integration with CI/CD

### GitHub Actions Example
```yaml
name: CHARM Conformance Testing
on: [push, pull_request]

jobs:
  conformance:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - name: Build CHARM
      run: make core
    - name: Run Conformance Tests
      run: make conformance
    - name: Upload Test Reports
      uses: actions/upload-artifact@v3
      with:
        name: conformance-reports
        path: conformance/nist/reports/
```

### Local Development
```bash
# Pre-commit validation
git add . && make conformance-quick && git commit -m "..."

# Release validation
make conformance && echo "Ready for release"
```

## Certification Guidance

### For Academic Research
1. Run `make conformance-extended` 
2. Document test environment and results
3. Include full test reports in research submission
4. Reference NIST validation methodology compliance

### For Commercial Use
1. Engage with cryptographic evaluation lab
2. Provide complete test suite and reports
3. Reference CHARM algorithm specification
4. Demonstrate ACVP-compatible interfaces

### For Standards Submission
1. Complete extended validation (100k+ vectors)
2. Generate comprehensive security analysis
3. Provide reference implementation source
4. Document compliance with relevant standards

## Scope and Limitations

### What This Framework Validates ✓
- Bit-for-bit algorithm correctness
- Statistical properties of outputs
- Implementation robustness and determinism
- Compliance with algorithm specification
- ACVP-compatible interfaces

### What This Framework Does NOT Provide ❌
- FIPS 140-2 validation (requires separate process)
- Side-channel attack resistance validation
- Formal security proofs
- Government certification or approval
- Export control compliance

## Troubleshooting

### Common Issues

**Test Failures Due to Precision**:
```bash
# Verify floating-point consistency
./conformance/nist/runner/charm_precision_test
```

**Performance Issues**:
```bash
# Run lightweight test subset
make conformance-light
```

**Platform Compatibility**:
```bash
# Disable platform-specific optimizations
make conformance DISABLE_SIMD=1
```

### Getting Help

- Review test logs in `conformance/nist/reports/`
- Check algorithm specification compliance
- Verify reference implementation consistency
- Contact development team with full error logs

## Conclusion

The CHARM conformance testing framework provides comprehensive validation equivalent to NIST standards for cryptographic hash functions. Regular execution of these tests ensures algorithm correctness, implementation quality, and readiness for evaluation by certification authorities.