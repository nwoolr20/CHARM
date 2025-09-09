# CHARM Algorithm Implementation

This directory contains the core CHARM algorithm implementations.

## Files

- **`charm_ref.c`**: Reference implementation of the CHARM hash algorithm with test main()
- **`charm_lib.c`**: Library version for linking with tests and applications

## Algorithm Overview

CHARM/1.0 is a non-linear, sequential streaming hash algorithm with:
- Fixed 64-byte blocks and 16KB processing chunks
- 256/384/512-bit output variants
- Adaptive round scheduling driven by entropy diagnostics
- Chaotic-style mixing and entropic collapse finalization

## Building

```bash
# Build from parent directory
cd ../..
make core

# Or build specific algorithm components
make algorithm
```

## Usage

The algorithm implements the frozen CHARM/1.0 specification with stable API defined in `../include/charm.h`.