# CHARM - Chaotic Hierarchical Adaptive Resilient Mechanism

CHARM is an advanced, entropy-native cryptographic system designed to provide high-security operations through a modular architecture leveraging chaotic dynamics, entropy field collapse, and nonlinear routing.

## System Components

CHARM consists of four primary subsystems:

1. **CAEDS** - Chaotic Anomaly and Entropy Diagnostic Subsystem
2. **CEE** - Chaotic Entropy Expansion
3. **ECE** - Entropy Collapse Engine
4. **Entropy Bus** - System integration layer

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
git clone https://github.com/charm-crypto/charm.git
cd charm

# Build the system
make

# Run tests
make test

# Install
sudo make install
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

This project is licensed under the [MIT License](LICENSE).

## Contact

For questions and support, please contact:
support@charm-crypto.org
