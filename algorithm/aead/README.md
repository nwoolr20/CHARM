# CHARM AEAD - Authenticated Encryption with Associated Data

CHARM AEAD provides enterprise-grade authenticated encryption using the entropy-native CHARM algorithm. It offers both traditional nonce-based AEAD and a misuse-resistant SIV (Synthetic IV) variant for applications requiring nonce-misuse resistance.

## Features

### 🔐 **Dual-Mode Operation**
- **Regular Mode**: High-performance nonce-based AEAD for standard use cases
- **SIV Mode**: Misuse-resistant deterministic encryption safe with repeated nonces

### 🛡️ **Enhanced Security**
- **256-bit keys** and **256-bit authentication tags** for maximum security margin
- **Entropy-native design** built on CHARM's advanced cryptographic properties
- **Nonce-misuse resistance** in SIV mode eliminates catastrophic key recovery attacks
- **Deterministic encryption** in SIV mode for consistent outputs

### ⚡ **Optimized Performance**
- Built on CHARM-512 for robust security with acceptable performance
- Streamlined keystream generation and authentication
- Efficient memory usage and minimal overhead

## Quick Start

### Basic Usage

```c
#include "charm_aead.h"

// Regular AEAD Mode
uint8_t key[CHARM_AEAD_KEY_SIZE] = {/* 32-byte key */};
uint8_t nonce[CHARM_AEAD_NONCE_SIZE] = {/* 16-byte nonce */};
uint8_t plaintext[] = "Hello, CHARM AEAD!";
uint8_t ciphertext[sizeof(plaintext)];
uint8_t tag[CHARM_AEAD_TAG_SIZE];

// Encrypt
charm_aead_encrypt(key, nonce, NULL, 0, plaintext, sizeof(plaintext), ciphertext, tag);

// Decrypt
uint8_t recovered[sizeof(plaintext)];
charm_aead_decrypt(key, nonce, NULL, 0, ciphertext, sizeof(plaintext), tag, recovered);
```

### Misuse-Resistant SIV Mode

```c
// SIV Mode - No nonce required, misuse-resistant
uint8_t key[CHARM_AEAD_KEY_SIZE] = {/* 32-byte key */};
uint8_t plaintext[] = "Deterministic encryption";
uint8_t ciphertext[sizeof(plaintext)];
uint8_t tag[CHARM_AEAD_TAG_SIZE];

// Encrypt (deterministic - same inputs always produce same outputs)
charm_aead_siv_encrypt(key, NULL, 0, plaintext, sizeof(plaintext), ciphertext, tag);

// Decrypt (same function works even with repeated encryption)
uint8_t recovered[sizeof(plaintext)];
charm_aead_siv_decrypt(key, NULL, 0, ciphertext, sizeof(plaintext), tag, recovered);
```

## Building

```bash
# Build everything
make all

# Build only the library
make lib

# Build and run tests
make test run-tests

# Build and run benchmarks
make benchmark run-benchmark

# Clean build artifacts
make clean
```

## API Reference

### Constants

```c
#define CHARM_AEAD_KEY_SIZE     32    // 256-bit key
#define CHARM_AEAD_NONCE_SIZE   16    // 128-bit nonce (regular mode only)
#define CHARM_AEAD_TAG_SIZE     32    // 256-bit authentication tag
```

### Core Functions

#### Regular AEAD Mode

```c
charm_aead_status_t charm_aead_encrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAD_TAG_SIZE]
);

charm_aead_status_t charm_aead_decrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAD_TAG_SIZE],
    uint8_t* plaintext
);
```

#### SIV Mode (Misuse-Resistant)

```c
charm_aead_status_t charm_aead_siv_encrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAD_TAG_SIZE]
);

charm_aead_status_t charm_aead_siv_decrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAD_TAG_SIZE],
    uint8_t* plaintext
);
```

### Utility Functions

```c
// HMAC for authentication
charm_aead_status_t charm_hmac(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[32]
);

// Key Derivation Function
charm_aead_status_t charm_kdf(
    const uint8_t* key, size_t key_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* output, size_t output_len
);

// Performance benchmarking
charm_aead_status_t charm_aead_benchmark(
    bool mode_siv, size_t payload_size, int iterations,
    double* encrypt_mbps, double* decrypt_mbps
);
```

## Security Properties

### Regular Mode
- **Confidentiality**: Secure under chosen-plaintext attacks (CPA)
- **Authenticity**: Secure under chosen-ciphertext attacks (CCA)
- **Nonce Requirements**: Nonce must be unique for each encryption with same key
- **Security Level**: 256-bit security margin

### SIV Mode
- **Nonce-Misuse Resistant**: Safe even with repeated or predictable nonces
- **Deterministic**: Same inputs always produce identical outputs
- **CCA Security**: Maintains authenticity even under nonce misuse
- **SIV-based**: Uses Synthetic IV derived from key, AAD, and plaintext

## Performance Characteristics

Performance varies by payload size and mode:

| Payload Size | Regular Mode | SIV Mode | Use Case |
|--------------|--------------|----------|----------|
| 16 bytes     | ~XXX MB/s    | ~XXX MB/s | IoT messages, tokens |
| 64 bytes     | ~XXX MB/s    | ~XXX MB/s | Small packets, metadata |
| 256 bytes    | ~XXX MB/s    | ~XXX MB/s | Configuration data |
| 1024 bytes   | ~XXX MB/s    | ~XXX MB/s | Documents, files |
| 4096 bytes   | ~XXX MB/s    | ~XXX MB/s | Large data blocks |

*Note: Run `make run-benchmark` for actual performance measurements on your system.*

## Comparison with CHARM-B AEAD

| Feature | CHARM AEAD | CHARM-B AEAD |
|---------|------------|---------------|
| **Optimization** | General purpose | Ultra-small payloads (≤64 bytes) |
| **Security** | CHARM-512 based | CHARM-256 based |
| **Tag Size** | 256-bit | 128-bit |
| **Nonce Size** | 128-bit | 96-bit |
| **Performance** | High | Ultra-high (small payloads) |
| **Use Cases** | Enterprise, general | IoT, embedded, real-time |

## Use Cases

### Regular Mode
- **Enterprise Applications**: General-purpose authenticated encryption
- **Document Protection**: File and database encryption
- **Communication Protocols**: Secure messaging and data transfer
- **Long-term Storage**: Archival data protection

### SIV Mode
- **Configuration Protection**: Database encryption, settings storage
- **Deterministic Requirements**: Consistent outputs for same inputs
- **Nonce-Constrained Environments**: Applications with nonce management challenges
- **High-Security Systems**: Zero-tolerance for nonce misuse

## Testing

The implementation includes comprehensive tests:

```bash
# Run all tests
make run-tests

# Expected output:
# ✅ Basic AEAD test passed
# ✅ SIV deterministic test passed  
# ✅ SIV different inputs test passed
# ✅ Authentication failure test passed
# ✅ No AAD test passed
# ✅ Performance test completed
# 🎉 All tests passed!
```

## Benchmarking

Compare performance against industry standards:

```bash
# Run comprehensive benchmarks
make run-benchmark

# Compares against:
# - AES-128-GCM
# - ChaCha20-Poly1305
# - AES-256-GCM
# - CHARM-B AEAD variants
```

## Integration

### With CHARM Security Suite
CHARM AEAD integrates seamlessly with the broader CHARM Security Suite for enterprise deployment.

### With CHARM-B AEAD
Use both implementations together:
- **CHARM AEAD**: General-purpose applications
- **CHARM-B AEAD**: Ultra-small payload optimization

## Standards Compliance

- **AEAD Interface**: Compatible with standard AEAD API patterns
- **SIV Construction**: Follows established SIV design principles
- **Cryptographic Standards**: Built on proven cryptographic primitives

## Contributing

See [CONTRIBUTING.md](../../../CONTRIBUTING.md) for guidelines on contributing to CHARM AEAD development.

## License

CHARM AEAD is licensed under the CHARM License 2025. See [LICENSE](../../../LICENSE) for details.

---

CHARM AEAD represents the next generation of authenticated encryption, combining the entropy-native security of CHARM with modern AEAD constructions and misuse-resistant SIV variants for comprehensive data protection.