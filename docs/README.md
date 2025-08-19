# CHARM - High-Performance Entropy-Native Cryptographic Framework

CHARM is a standalone, entropy-native cryptographic framework that functions as both a high-performance hashing system and a dynamic entropy management engine. It leverages chaotic dynamics, adaptive resonance, and entropy-field collapse to maintain optimal stability, unpredictability, and efficiency across a wide range of computational scenarios. CHARM is designed for modern and future security challenges while optimizing performance for inputs of varying sizes.

At its core, CHARM ensures data integrity, cryptographic strength, and adaptive performance, combining concepts from chaos theory and cognitive-inspired entropy management.

## Core Modules

CHARM consists of three primary subsystems:

1. **CAEDS** - Chaotic Anomaly & Entropy Diagnostic Subsystem  
   Continuously monitors system entropy, detects irregularities or imbalances, and provides adaptive feedback to maintain stability.

2. **CEE** - Chaotic Entropy Expansion  
   Generates and amplifies structured entropy to enhance unpredictability, performance, and cryptographic resilience.

3. **ECE** - Entropic Collapse Engine  
   Stabilizes over-chaotic states by compressing excessive entropy into coherent outputs, ensuring consistent and secure operation.

Together, these modules make CHARM a self-regulating, high-performance hashing framework.

## Features

- Entropy-native design with >7.95 bits/byte density
- SIMD acceleration using AVX2 and NEON
- Chaotic permutations and entropic field collapse
- Independent of SHA, BLAKE3, or Merkle-style logic
- Comprehensive CLI tools and diagnostics
- Fallback entropy and watchdog monitoring

## Building CHARM

```bash
# Clone the repository
git clone https://github.com/nwoolr20/CHARM.git
cd CHARM

# Build the core system
make core

# Build benchmarks
make bench

# Run performance benchmarks
make benchmark
```

## Usage

CHARM provides a comprehensive command-line interface:

```bash
# Generate a cryptographic digest
charm --digest input.txt

# Process data in streaming mode
charm --stream-mode large_file.bin

# Check entropy status
charm --entropy-status

# Start the watchdog daemon
charm --watchdog-start

# Enable debug output
charm --debug
```

## Documentation

Detailed documentation is available in the `docs` directory:

- [System Overview](SYSTEM_OVERVIEW.md)
- [CAEDS Documentation](CAEDS.md)
- [CEE Documentation](CEE.md)
- [ECE Documentation](ECE.md)
- [Entropy Bus Documentation](entropy_bus.md)

## Requirements

- C99-compatible compiler
- POSIX-compliant operating system
- AVX2 or NEON support for full performance (optional)
- 64-bit architecture recommended

## License

This project is licensed under the CHARM License 2025. See [LICENSE](../LICENSE) for details.

## Contact

For questions and support, please contact:
support@charm-crypto.org
