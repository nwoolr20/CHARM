# Third Party Dependencies

This directory contains vendored third-party cryptographic libraries used for benchmarking and comparison purposes.

## BLAKE3

**Location**: `crypto/blake3/`  
**Purpose**: High-performance cryptographic hash function for benchmarking comparison  
**Upstream**: https://github.com/BLAKE3-team/BLAKE3  
**License**: CC0-1.0, Apache-2.0 dual license (see `crypto/blake3/LICENSE_*` files)  
**Vendored Commit**: Latest available at time of inclusion  

BLAKE3 is included for performance comparison and validation purposes. It represents the current state-of-the-art in high-performance cryptographic hashing and serves as a benchmark for CHARM's performance claims.

## Provenance and Reproducibility

- All vendored code maintains original license files and attribution
- Upstream commit hashes are tracked for reproducibility
- No modifications made to vendored code - clean interfaces used for integration
- Optional benchmarks can be disabled by building without third-party dependencies

## Usage

Third-party libraries are gated behind clean interfaces and are optional for core CHARM functionality. They are used primarily for:

1. Performance benchmarking and comparison
2. Validation against established cryptographic standards  
3. Demonstration of CHARM's competitive performance characteristics

To build without third-party dependencies:
```bash
make ENABLE_THIRD_PARTY=0
```