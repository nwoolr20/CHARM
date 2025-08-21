# CHARM Crypto Suite Structure

This directory contains the modular cryptographic suite built on the CHARM entropy-native framework.

## Module Organization

### Core Primitives
- **aead/**: Authenticated Encryption with Associated Data implementations
  - CHARM-B AEAD (optimized for small payloads)
  - Standards-based AEAD wrappers (AES-GCM, ChaCha20-Poly1305)
  
- **mac/**: Message Authentication Codes
  - HMAC-CHARM construction
  - CHARM-MAC (native construction)
  
- **kdf/**: Key Derivation Functions  
  - HKDF-CHARM (HKDF using CHARM hash)
  - CHARM-KDF (native key expansion)
  
- **xof/**: Extendable Output Functions
  - CHARM-XOF (sponge/duplex construction)

### Advanced Primitives
- **drbg/**: Deterministic Random Bit Generators
  - Hash_DRBG-CHARM (SP 800-90A-like)
  - CHARM-native DRBG with entropy health monitoring
  
- **symmetric/**: Symmetric Encryption
  - Standards wrappers (AES-GCM, ChaCha20-Poly1305)
  - Unified AEAD API for interoperability
  
- **asymmetric/**: Asymmetric Cryptography
  - Ed25519/X25519 wrappers
  - RSA support for legacy interop
  - Post-quantum cryptography integration

## Design Principles

1. **Entropy-Native Core**: All constructions leverage CHARM's entropy-adaptive properties
2. **Standards Interoperability**: Provide standard algorithm wrappers for compatibility  
3. **Performance Optimization**: Optimize for CHARM's strengths (small inputs, high throughput)
4. **Security First**: Conservative design with clear security documentation
5. **Modular Architecture**: Independent modules with clean interfaces

## Implementation Status

### Completed ✅
- CHARM-B AEAD (ultra-fast small payload encryption)

### In Progress 🚧
- Basic directory structure
- Security documentation framework

### Planned 📋
- HMAC-CHARM implementation
- CHARM-XOF design and implementation
- Standards-based AEAD wrappers
- Unified API design
- Key management framework

## API Design Goals

### Consistent Interface
```c
// Example unified AEAD API pattern
typedef enum {
    CHARM_AEAD_CHARMB,      // CHARM-B native AEAD
    CHARM_AEAD_AES_GCM,     // AES-256-GCM wrapper
    CHARM_AEAD_CHACHA20     // ChaCha20-Poly1305 wrapper
} charm_aead_algorithm_t;

int charm_aead_encrypt(
    charm_aead_algorithm_t algo,
    const uint8_t* key, size_t key_len,
    const uint8_t* nonce, size_t nonce_len,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext, uint8_t* tag
);
```

### Error Handling
- Consistent error codes across all modules
- Clear failure modes and security guarantees
- Defensive programming practices

### Performance Transparency
- Algorithm-specific optimizations clearly documented
- Performance characteristics measurable and comparable
- Entropy usage and health monitoring integrated

---

**Note**: This is the foundation for CHARM's evolution into a comprehensive cryptographic suite while maintaining its core entropy-native advantages.