# CHARM

# CHARM

An experimental, entropy‑native cryptographic hashing framework with an integrated entropy collection/diagnostics layer, a small‑input-optimized variant (CHARM‑B), early AEAD experiments, and a NIST‑style conformance harness. Drawing on chaotic dynamics, it self-regulates via three core subsystems—CAEDS (entropy diagnostics), CEE (entropy expansion), and ECE (entropic collapse)—to adapt to input conditions, maintain stability, and ensure strong data integrity. It's designed for efficiency across workloads and supports use cases like blockchain security, distributed systems integrity, secure storage, and cryptographic protocols.

The CHARM algorithm in a nutshell: CHARM/1.0 is a non-linear, sequential streaming hash (not tree/Merkle) with fixed 64-byte blocks and 16KB processing chunks, offering 256/384/512-bit outputs. It uses an entropy-native design with adaptive round scheduling (between a fixed min/max) driven by lightweight entropy diagnostics, applying non-linear, chaotic-style mixing during block processing and an "entropic collapse" finalization to extract the digest. The internal state is compact (128 bytes), the spec is frozen and deterministic, and implementations provide a clean init/update/final API with optional keyed mode. The design emphasizes constant-time operations, SIMD acceleration (AVX2/NEON), streaming invariance, and auditability of a simple sequential core.

The CHARM framework includes specialized variants and early cryptographic primitives: **CHARM-B** provides ultra-small input optimization (≤64 bytes) with revolutionary performance gains, **AEAD/AEAS** offers authenticated encryption experiments, and **CHARM-B AEAD** delivers entropy-native authenticated encryption for constrained environments. The framework includes comprehensive conformance testing, NIST-style validation harnesses, and extensive benchmarking against established algorithms.

What CHARM aims to do versus SHA and BLAKE is deliver a modern, entropy-aware hash that keeps the simplicity and determinism of a streaming design while outperforming SHA-256 and BLAKE3 at practical payload sizes—without sacrificing cryptographic strength or forward-looking ("quantum-ready") properties. According to the repo's comprehensive benchmark reports, CHARM now beats SHA-256 starting at 4KB inputs (about +3% at 4KB), and shows large advantages for bigger data (roughly +247–283% at 16KB+). Against BLAKE3, it remains competitive on tiny inputs, and leads for larger sizes (about +100–150% at 16KB+). Security-wise, the project includes NIST-style conformance harnesses (KATs, Monte Carlo, avalanche, streaming invariance), passes its internal security analysis, and provides constant-time paths with SIMD optimizations.

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
5. **IoT and Embedded Systems**: CHARM-B provides ultra-efficient hashing for constrained devices, with CHARM-B AEAD offering authenticated encryption without hardware dependencies.
6. **Research and Development**: Comprehensive testing frameworks and benchmarking tools support cryptographic research and algorithm development.

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

Detailed documentation is available in the `documents` directory:

- [System Overview](documents/SYSTEM_OVERVIEW.md)
- [CAEDS Documentation](documents/CAEDS.md)
- [CEE Documentation](documents/CEE.md)
- [ECE Documentation](documents/ECE.md)
- [Entropy Bus Documentation](documents/entropy_bus.md)

### CHARM Framework Components

- **[CHARM-B](CHARM-B/README.md)**: Ultra-small input optimized hash with comprehensive performance analysis
- **[AEAD/AEAS](AEAD/AEAS/README.md)**: Authenticated encryption experiments with performance studies
- **[CHARM-B AEAD](CHARM-B/AEAD/README.md)**: Entropy-native AEAD for constrained environments
- **[Conformance Testing](documents/Conformance-Testing.md)**: NIST-style validation framework

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to CHARM. All contributions must be backed by empirical test results demonstrating performance, stability, or security improvements.

## License

This project is licensed under the CHARM License 2025. See [LICENSE](LICENSE) for details.

---

CHARM is more than a hash function—it is a comprehensive cryptographic framework that integrates entropy management, high-performance hashing, authenticated encryption experiments, and rigorous validation. The framework provides tools for research, development, and specialized applications where entropy-native cryptography offers advantages over conventional approaches.