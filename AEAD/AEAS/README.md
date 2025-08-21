# CHARM-AEAS: Authenticated Encryption with Adaptive State

CHARM-AEAS is an Authenticated Encryption with Associated Data (AEAD) construction built on top of the CHARM cryptographic hash function. It provides confidentiality and integrity in a single operation using an Encrypt-then-MAC design similar to RFC 7539 ChaCha20-Poly1305.

## Design Overview

### Construction
- **Base Primitive**: CHARM-256 hash function
- **Design Pattern**: Encrypt-then-MAC (RFC 7539 style)
- **Key Derivation**: HKDF-CHARM for deriving encryption and MAC keys
- **Stream Cipher**: HMAC-CHARM-based keystream generation
- **Authentication**: Poly1305 one-time MAC
- **Security Level**: 128-bit (with 256-bit keys)

### Parameters
- **Master Key**: 32 bytes (256-bit)
- **Nonce**: 12 bytes (96-bit) - must be unique per key
- **Tag**: 16 bytes (128-bit authentication tag)
- **Block Size**: 32 bytes (HMAC-CHARM output size)

### Algorithm Flow

1. **Key Derivation** (HKDF-CHARM):
   ```
   k_enc = HKDF-CHARM(key, salt=nonce, info="AEAS-enc", L=32)
   k_mac = HKDF-CHARM(key, salt=nonce, info="AEAS-mac", L=32)
   ```

2. **Stream Encryption**:
   ```
   KS[i] = HMAC-CHARM(k_enc, nonce || LE64(counter) || LE32(i))
   ciphertext = plaintext XOR keystream
   ```

3. **Authentication** (Poly1305):
   ```
   otk = HMAC-CHARM(k_mac, nonce)
   tag = Poly1305(otk, AAD || pad16 || ciphertext || pad16 || LE64(|AAD|) || LE64(|ciphertext|))
   ```

## Security Properties

### Cryptographic Strength
- **Confidentiality**: Based on CHARM-256 PRF security
- **Integrity**: 128-bit authentication strength via Poly1305
- **Nonce Misuse**: Not nonce-misuse resistant (like ChaCha20-Poly1305)
- **Forward Security**: No forward secrecy (stateless design)

### Constant-Time Implementation
- ✅ Constant-time memory comparison for tag verification
- ✅ Constant-time Poly1305 implementation
- ✅ No secret-dependent branches in critical paths
- ✅ CHARM primitive provides constant-time guarantees

### Side-Channel Resistance
- **Timing Attacks**: Mitigated through constant-time operations
- **Cache Attacks**: CHARM uses predictable memory access patterns
- **Power Analysis**: Not specifically hardened (implementation dependent)

## Nonce Management

### Critical Security Requirement
**Nonces MUST NEVER repeat for the same key.** Nonce reuse completely breaks confidentiality and may compromise integrity.

### Nonce Generation Strategies

#### Counter-Based (Recommended)
```c
aeas_nonce_ctx_t ctx;
uint8_t prefix[4] = {0x01, 0x02, 0x03, 0x04};  // Application-specific
uint8_t nonce[12];

aeas_nonce_init(&ctx, prefix);
for (each message) {
    aeas_nonce_next(&ctx, nonce);  // Auto-incrementing counter
    // Use nonce for encryption...
}
```

#### Random Nonces (Caution)
- Use cryptographically secure random number generator
- Birthday bound: ~2^48 messages before collision risk
- Not recommended for high-volume applications

### Nonce Format (Counter-Based)
```
| Application Prefix | Message Counter |
|      4 bytes       |    8 bytes      |
```

## API Usage

### Basic Encryption/Decryption
```c
#include "aeas.h"

uint8_t key[32], nonce[12], tag[16];
uint8_t plaintext[] = "Hello, CHARM-AEAS!";
uint8_t ciphertext[sizeof(plaintext)];
uint8_t decrypted[sizeof(plaintext)];

// Encryption
int result = aeas_encrypt(key, nonce, NULL, 0,                    // No AAD
                         plaintext, sizeof(plaintext),
                         ciphertext, tag);

// Decryption
result = aeas_decrypt(key, nonce, NULL, 0,
                     ciphertext, sizeof(plaintext), tag,
                     decrypted);
```

### With Additional Authenticated Data (AAD)
```c
uint8_t aad[] = "Protocol header data";

// Encrypt with AAD
aeas_encrypt(key, nonce, aad, sizeof(aad),
            plaintext, pt_len, ciphertext, tag);

// Decrypt with AAD
aeas_decrypt(key, nonce, aad, sizeof(aad),
            ciphertext, ct_len, tag, decrypted);
```

### Counter-Based Nonce Management
```c
aeas_nonce_ctx_t nonce_ctx;
uint8_t app_prefix[4] = {0xDE, 0xAD, 0xBE, 0xEF};
uint8_t nonce[12];

aeas_nonce_init(&nonce_ctx, app_prefix);

for (int i = 0; i < num_messages; i++) {
    aeas_nonce_next(&nonce_ctx, nonce);
    aeas_encrypt(key, nonce, /* ... */);
}
```

## Performance Characteristics

### Benchmark Results
Based on our testing with OpenSSL 3.0.13 on a modern system:

| Data Size | CHARM-AEAS | AES-256-GCM | ChaCha20-Poly1305 |
|-----------|-------------|-------------|-------------------|
| 64B       | 14.7 MB/s   | 583.9 MB/s  | 546.6 MB/s       |
| 1KB       | 8.9 MB/s    | 1569.1 MB/s | 1288.1 MB/s      |
| 16KB      | 2.5 MB/s    | 1522.0 MB/s | 915.1 MB/s       |
| 1MB       | 255.5 MB/s  | 182.8 MB/s  | 102.9 MB/s       |

### Performance Analysis
- **Small Messages (< 4KB)**: AES-GCM and ChaCha20-Poly1305 significantly faster due to hardware acceleration
- **Large Messages (≥ 1MB)**: CHARM-AEAS becomes competitive, outperforming ChaCha20-Poly1305
- **Target Use Cases**: Applications without AES-NI hardware, embedded systems, or where CHARM's entropy properties provide value

### Optimization Opportunities
- **AVX2 Implementation**: Potential 4x parallelization of keystream generation
- **Hardware Acceleration**: Custom CHARM hardware could dramatically improve performance
- **Streaming Optimization**: Reduce HMAC-CHARM calls for large messages

## Build Instructions

### Basic Build
```bash
cd AEAD/AEAS
make all                    # Build library and tests
make test                   # Run unit tests
make lib                    # Build static library only
```

### With OpenSSL (for benchmarks)
```bash
make bench OPENSSL=1       # Build and run benchmarks vs OpenSSL
```

### With AVX2 (when implemented)
```bash
make all AVX2=1            # Enable AVX2 optimizations
```

## Testing

### Unit Tests
- ✅ Basic encrypt/decrypt functionality
- ✅ AAD handling and verification
- ✅ Tampering detection (ciphertext and tag)
- ✅ Empty input edge cases
- ✅ Nonce context management
- ✅ Parameter validation

### Test Results
All 21 unit tests pass successfully. See `results/test_report.md` for detailed results.

## Limitations and Considerations

### Known Limitations
1. **Performance**: Slower than hardware-accelerated AES-GCM for small messages
2. **Standards**: Not standardized (experimental implementation)
3. **Audit Status**: Not yet third-party audited
4. **Nonce Misuse**: Not misuse-resistant (consider SIV mode for future)

### Recommended Use Cases
- Research and evaluation of CHARM-based constructions
- Systems without AES hardware acceleration
- Applications requiring CHARM's specific entropy properties
- Educational purposes and cryptographic research

### Not Recommended For
- Production systems requiring standardized algorithms
- High-performance applications with small message sizes
- Systems where nonce uniqueness cannot be guaranteed

## Future Enhancements

### Planned Improvements
- **AVX2 Keystream Generation**: 4-way parallel HMAC-CHARM computation
- **SIV Mode**: Misuse-resistant variant for nonce-reuse scenarios
- **Hardware Acceleration**: FPGA/ASIC implementations of CHARM primitive
- **Standards Track**: Potential submission to cryptographic standards bodies

### Research Directions
- Formal security analysis and proofs
- Post-quantum security evaluation
- Integration with quantum key distribution systems
- Performance optimization studies

## License and Attribution

### CHARM-AEAS Implementation
This implementation is part of the CHARM project. See main repository LICENSE for terms.

### Poly1305 Implementation
The Poly1305 implementation is based on the public domain specification (RFC 7539) and follows established constant-time techniques. See `LICENSE.poly1305.txt` for specific attribution.

## References

1. RFC 7539: ChaCha20 and Poly1305 for IETF Protocols
2. RFC 5869: HMAC-based Extract-and-Expand Key Derivation Function (HKDF)
3. RFC 2104: HMAC: Keyed-Hashing for Message Authentication
4. CHARM Algorithm Specification (see main repository)

---

**⚠️ Security Notice**: This is an experimental implementation for research purposes. Do not use in production systems without thorough security review and testing.