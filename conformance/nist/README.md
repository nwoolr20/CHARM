# CHARM NIST-Style Conformance Testing

This directory contains a standalone, reproducible test harness that mirrors CAVP/ACVP methodology for hash algorithms. While CHARM is not a FIPS algorithm, it adopts the same testing structure used by NIST to demonstrate algorithmic rigor and provide ACVP-like JSON compatibility.

## Purpose

Demonstrate CHARM algorithm conformance through systematic validation that mirrors:
- FIPS 180-4 (SHA-2) testing methodology
- FIPS 202 (SHA-3/SHAKE) validation approach  
- SP 800-185 derived function testing
- ACVP SHA profile compatibility

## Directory Structure

```
conformance/nist/
├── README.md               # This file
├── schemas/
│   └── acvp-sha.json      # ACVP SHA schema reference
├── vectors/
│   ├── kat/               # Known Answer Tests
│   ├── monte-carlo/       # Long-run sequences  
│   ├── variable-msg/      # Variable length messages
│   ├── bit-flip/          # Bit-level avalanche tests
│   ├── streaming/         # Chunk boundary tests
│   └── xof/               # Extended output function tests
├── runner/
│   ├── acvp_adapter.py    # JSON I/O compatible with ACVP
│   ├── charm_kat.cpp      # Known Answer Test runner
│   ├── charm_mc.cpp       # Monte Carlo test runner
│   └── charm_stream.cpp   # Streaming test runner
└── reports/
    ├── <timestamp>.json   # Machine-readable results
    └── <timestamp>.md     # Human-readable reports
```

## Test Types

### Known Answer Tests (KATs)
Exact input→output vectors for each CHARM variant:
- CHARM-256, CHARM-384, CHARM-512
- Keyed vs unkeyed modes
- Various input lengths including edge cases

### Monte Carlo Tests  
Long-run chained hashes to detect state flaws:
- H_{i+1} = CHARM(H_i || seed_i) for 10k-100k iterations
- Statistical analysis of output distribution
- Period detection and bias analysis

### Variable-Length Tests
Systematic testing across input sizes:
- 0, 1, 2, ..., 1024, 1025, 2048 bytes
- Powers of 2 and boundaries around CHARM_BLOCK_BYTES (64)
- Large inputs up to 1MB

### Bit-Level Tests
Avalanche effect verification:
- Flip each bit of base messages
- Measure full-width avalanche distribution
- Statistical analysis of bit independence

### Streaming Tests
Invariance verification:
- Compare single-shot vs streaming updates
- Various split patterns: 1B, 31B, 4KiB, 64KiB chunks
- Verify identical results regardless of chunking

### Determinism Tests
Reproducibility verification:
- Same input always produces same output
- No dependency on runtime environment
- Fixed algorithm parameters produce consistent results

## ACVP Compatibility

The test harness provides ACVP-style JSON interfaces:
- Input: ACVP-compatible test vector sets
- Output: ACVP-compatible response format  
- Schema: Based on ACVP SHA and XOF profiles

This enables integration with ACVP-based validation tools and demonstrates methodological parity with NIST validation approaches.

## Usage

### Run All Tests
```bash
make conformance
```

### Run Specific Test Types
```bash
./conformance/nist/runner/charm_kat     # Known Answer Tests
./conformance/nist/runner/charm_mc      # Monte Carlo Tests  
./conformance/nist/runner/charm_stream  # Streaming Tests
```

### Generate ACVP-Compatible Vectors
```bash
python3 conformance/nist/runner/acvp_adapter.py --generate --output vectors.json
```

### Validate Against Vectors
```bash
python3 conformance/nist/runner/acvp_adapter.py --validate --input vectors.json
```

## Reports

Test results are automatically saved to `reports/` with:
- Machine-readable JSON format for automation
- Human-readable Markdown summaries  
- Timestamped files for historical tracking
- Coverage metrics and pass/fail status

## Security Note

This conformance testing framework:
- ✅ Demonstrates methodological rigor equivalent to NIST validation
- ✅ Provides ACVP-compatible interfaces for tool integration
- ✅ Validates deterministic algorithm behavior
- ❌ Does NOT constitute FIPS approval or certification
- ❌ Does NOT make CHARM a FIPS-approved algorithm

The purpose is to prove that CHARM meets the same validation standards used for NIST algorithms, providing confidence in its algorithmic soundness and implementation correctness.