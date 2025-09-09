# CHARM Algorithm Reference Implementation

This directory contains the reference and library implementations of the CHARM hash algorithm with frozen specification (CHARM/1.0).

## Directory Structure

- **`include/`**: Frozen algorithm API specification
- **`src/`**: Reference and library implementations  
- **`aead/`**: Enterprise-grade authenticated encryption implementation
- **`tests/`**: Unit and property tests (when available)

## Algorithm Overview

**CHARM/1.0** is a non-linear, sequential streaming hash algorithm designed for entropy-native cryptographic operations. It provides:

### Core Features
- **Fixed Structure**: 64-byte blocks with 16KB processing chunks
- **Output Variants**: 256/384/512-bit hash outputs
- **Adaptive Processing**: Round scheduling driven by entropy diagnostics
- **Entropy-Native**: Chaotic-style mixing with entropic collapse finalization
- **Frozen Specification**: Immutable CHARM/1.0 standard with version tracking

### Algorithm Properties
- **Non-Merkle Construction**: Not based on Merkle-Damgård paradigm
- **Streaming Capable**: Processes data in fixed-size chunks efficiently
- **Entropy Diagnostics**: Built-in entropy quality assessment
- **Chaotic Dynamics**: Leverages chaos theory for mixing operations

## API Specification

The algorithm implements a clean, frozen API defined in `include/charm.h`:

```c
#include "charm.h"

// Initialize CHARM context
charm_ctx_t ctx;
charm_init(&ctx, CHARM_256);

// Process data
charm_update(&ctx, data, data_len);

// Finalize hash
uint8_t hash[32];
charm_final(&ctx, hash);
```

## Implementation Files

### Core Algorithm
- **`src/charm_ref.c`**: Reference implementation with test main()
- **`src/charm_lib.c`**: Library version for linking with applications
- **`include/charm.h`**: Clean algorithm API (frozen specification)

### Authenticated Encryption
- **`aead/`**: CHARM AEAD implementation using CHARM-512
- Enterprise-grade authenticated encryption with SIV support
- Misuse-resistant design for production environments

## Building

```bash
# Build from repository root
make core
make algorithm

# Or build specific components
cd algorithm/src
gcc -O3 -o charm_ref charm_ref.c
gcc -O3 -c charm_lib.c  # For linking
```

## Algorithm Specification

CHARM/1.0 implements a **frozen specification** that includes:

- **Version Encoding**: Embedded version tracking (version byte = 1)
- **Parameter Block**: Structured configuration with output size selection
- **Processing Model**: Fixed 64-byte blocks with 16KB chunk processing
- **Round Scheduling**: Adaptive rounds based on entropy diagnostics
- **Finalization**: Entropic collapse for secure output generation

### Security Properties
- **Collision Resistance**: Designed for cryptographic security
- **Preimage Resistance**: Computationally infeasible to reverse
- **Entropy Preservation**: Maintains input entropy throughout processing
- **Quantum Considerations**: Entropy-native design for post-quantum resilience

## Standards Compliance

- **Frozen Specification**: CHARM/1.0 is immutable and versioned
- **API Stability**: Clean interface with backward compatibility guarantee
- **Test Vectors**: Reference implementation provides canonical test vectors
- **Bit-for-bit Compatibility**: Required across all implementations

## Integration

The CHARM algorithm serves as the foundation for:
- **CHARM Framework**: Core hashing for ECE, CAEDS, and CEE
- **CHARM AEAD**: Authenticated encryption with CHARM-512
- **Security Suite**: Enterprise cryptographic operations
- **PBKDF2**: Password-based key derivation using HMAC-CHARM

## Performance

Optimized for various input sizes:
- **Small Inputs**: Efficient processing for ≤64 byte inputs
- **Streaming**: Optimal for large file processing
- **Parallel**: SIMD-friendly design for modern processors

For detailed algorithm specification, see `../documents/CHARM-Algorithm-Spec.md`.