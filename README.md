# CHARM

CHARM is a standalone, entropy-native cryptographic framework that functions as both a high-performance hashing system and a dynamic entropy management engine. It leverages chaotic dynamics, adaptive resonance, and entropy-field collapse to maintain optimal stability, unpredictability, and efficiency across a wide range of computational scenarios. CHARM is designed for modern and future security challenges while optimizing performance for inputs of varying sizes.

At its core, CHARM ensures data integrity, cryptographic strength, and adaptive performance, combining concepts from chaos theory and cognitive-inspired entropy management.

## Core Modules

1. **CAEDS – Chaotic Anomaly & Entropy Diagnostic Subsystem**  
   Continuously monitors system entropy, detects irregularities or imbalances, and provides adaptive feedback to maintain stability.

2. **CEE – Chaotic Entropy Expansion**  
   Generates and amplifies structured entropy to enhance unpredictability, performance, and cryptographic resilience.

3. **ECE – Entropic Collapse Engine**  
   Stabilizes over-chaotic states by compressing excessive entropy into coherent outputs, ensuring consistent and secure operation.

Together, these modules make CHARM a self-regulating, high-performance hashing framework.

## Functional Roles

- **Cryptographic Security**: High-performance, entropy-conditioned hashing with resistance to both classical and quantum attacks.
- **Adaptive Optimization**: Dynamically adjusts internal rounds, memory usage, and feature activation based on input size, avoiding performance bottlenecks without compromising security.
- **Stability Control**: Balances randomness and structure to prevent collisions, interference, or weak outputs, ensuring speed and reliability across all use cases.

## Distinctive Features

- Actively shapes entropy rather than relying solely on passive randomness sources.
- The combination of CAEDS, CEE, and ECE forms a self-regulating cryptographic engine that adapts to input conditions and threat models.
- High efficiency across input sizes, with optimized performance for both small data packets and large datasets.

## Applications

1. **Blockchain Security**: Entropy-conditioned hashing enhances block verification and signature security, maintaining low-latency performance for small transactions.
2. **Distributed Systems**: Ensures data integrity and consistency across nodes under varying load conditions, resilient to tampering or network noise.
3. **Secure Storage & Archiving**: Provides consistent, high-performance hashing across files of all sizes, with long-term quantum-resistant verification.
4. **Cryptographic Protocols**: Serves as a core component in key derivation functions, secure communication channels, and authentication mechanisms.

## Installation and Setup

### Prerequisites

- C99-compatible compiler
- POSIX-compliant operating system
- AVX2 or NEON support for full performance (optional)
- 64-bit architecture recommended

### Building CHARM

```bash
# Clone the repository
git clone https://github.com/nwoolr20/CHARM.git
cd CHARM

# Build the core system
make core

# Build benchmarks
make bench

# Run tests
make test

# Run performance benchmarks
make benchmark
```

## Usage Examples

```bash
# Generate a cryptographic digest
./build/charm --digest input.txt

# Process data in streaming mode
./build/charm --stream-mode large_file.bin

# Check entropy status
./build/charm --entropy-status

# Enable debug output
./build/charm --debug
```

## Documentation

Detailed documentation is available in the `docs` directory:

- [System Overview](docs/SYSTEM_OVERVIEW.md)
- [CAEDS Documentation](docs/CAEDS.md)
- [CEE Documentation](docs/CEE.md)
- [ECE Documentation](docs/ECE.md)
- [Entropy Bus Documentation](docs/entropy_bus.md)

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to CHARM. All contributions must be backed by empirical test results demonstrating performance, stability, or security improvements.

## License

This project is licensed under the CHARM License 2025. See [LICENSE](LICENSE) for details.

---

CHARM is more than a hash function—it is a dynamic entropy-management engine that works like a charm, integrating performance, adaptability, and cutting-edge cryptographic security, suitable for a broad spectrum of computing and security-critical applications.