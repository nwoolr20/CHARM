# CHARM-B AEAD: Ultra-Small Input Authenticated Encryption

CHARM-B AEAD is an Authenticated Encryption with Associated Data (AEAD) construction built on top of the CHARM-B cryptographic hash function. It provides confidentiality and integrity for ultra-small payloads (≤64 bytes) using an optimized encrypt-then-MAC design.

## Design Overview

CHARM-B AEAD uses a two-key approach derived from a single master key:
- **Encryption Key**: Used for keystream generation via CHARM-B hash
- **Authentication Key**: Used for HMAC-CHARM tag computation

The construction follows the pattern: `C = P ⊕ KEYSTREAM` and `TAG = HMAC(auth_key, AAD || C)`

### Key Derivation

```
ENC_KEY = HKDF-EXPAND(HKDF-EXTRACT(NONCE, KEY), "CHARM-B-AEAD", 32)
AUTH_KEY = HKDF-EXPAND(HKDF-EXTRACT(NONCE, KEY), "CHARM-B-AEAD", 32)[32:]
```

### Encryption Process

1. Derive encryption and authentication keys from master key and nonce
2. Generate keystream using CHARM-B hash of (enc_key || nonce || counter)
3. XOR plaintext with keystream to produce ciphertext
4. Compute HMAC-CHARM tag over AAD || ciphertext
5. Return ciphertext and truncated 128-bit tag

### Decryption Process

1. Derive same encryption and authentication keys
2. Verify HMAC-CHARM tag over AAD || ciphertext
3. If tag valid, generate keystream and XOR with ciphertext
4. Return recovered plaintext

## Security Properties

### Cryptographic Strength
- **Confidentiality**: Based on CHARM-B PRF security
- **Integrity**: 128-bit authentication strength via HMAC-CHARM
- **Nonce Misuse**: Not nonce-misuse resistant (requires unique nonces)
- **Forward Security**: No forward secrecy (stateless design)

### Constant-Time Implementation
- ✅ Constant-time memory comparison for tag verification
- ✅ Constant-time HMAC implementation with chunked approach
- ✅ No secret-dependent branches in critical paths
- ✅ CHARM-B primitive provides deterministic operation with entropy resets

### Side-Channel Resistance
- **Timing Attacks**: Mitigated through constant-time operations
- **Cache Attacks**: CHARM-B uses predictable memory access patterns
- **Power Analysis**: Not specifically hardened (implementation dependent)

## Nonce Management

**Critical**: Nonces must be unique for each encryption with the same key.
- Nonce size: 96 bits (12 bytes)
- Recommended: Use a counter or random nonce with sufficient entropy
- **Never reuse** nonces with the same key

```c
// Good: Counter-based nonces
uint8_t nonce[12];
uint64_t counter = get_message_counter();
memcpy(nonce, &counter, 8);
memset(nonce + 8, 0, 4);

// Good: Random nonces (ensure sufficient entropy)
if (getrandom(nonce, 12, 0) != 12) {
    // Handle error
}
```

## API Usage

### Basic Encryption/Decryption

```c
#include "charmb_aead.h"

uint8_t key[32];        // 256-bit master key
uint8_t nonce[12];      // 96-bit nonce (must be unique)
uint8_t aad[] = "Additional data";
uint8_t plaintext[] = "Secret message";
uint8_t ciphertext[sizeof(plaintext)];
uint8_t tag[16];        // 128-bit authentication tag

// Encryption
charmb_aead_status_t status = charmb_aead_encrypt(
    key, nonce, 
    aad, strlen((char*)aad),
    plaintext, strlen((char*)plaintext),
    ciphertext, tag
);

if (status == CHARMB_AEAD_SUCCESS) {
    // Encryption successful
    // Store/transmit: nonce + aad + ciphertext + tag
}

// Decryption
uint8_t recovered[sizeof(plaintext)];
status = charmb_aead_decrypt(
    key, nonce,
    aad, strlen((char*)aad), 
    ciphertext, strlen((char*)plaintext),
    tag, recovered
);

if (status == CHARMB_AEAD_SUCCESS) {
    // Decryption and authentication successful
    // Use recovered plaintext
}
```

### Error Handling

```c
switch (status) {
    case CHARMB_AEAD_SUCCESS:
        // Operation successful
        break;
    case CHARMB_AEAD_ERROR_NULL_POINTER:
        // Invalid NULL pointer argument
        break;
    case CHARMB_AEAD_ERROR_INVALID_SIZE:
        // Invalid size parameter
        break;
    case CHARMB_AEAD_ERROR_AUTH_FAILED:
        // Authentication tag verification failed
        break;
    case CHARMB_AEAD_ERROR_BUFFER_TOO_SMALL:
        // Output buffer too small
        break;
}
```

## Performance Characteristics

### Target Use Cases
CHARM-B AEAD is optimized for:
- Ultra-small payloads (8-64 bytes)
- IoT device authentication
- Embedded systems without AES-NI
- Research and experimental applications

### Benchmark Results
Performance on modern x86_64 system:

| Payload Size | CHARM-B AEAD | AES-256-GCM | Performance Gap |
|--------------|---------------|-------------|-----------------|
| 8B           | 15.2 MB/s     | 582.1 MB/s  | 38x slower      |
| 16B          | 18.7 MB/s     | 891.4 MB/s  | 47x slower      |
| 32B          | 22.3 MB/s     | 1156.8 MB/s | 52x slower      |
| 64B          | 28.1 MB/s     | 1398.7 MB/s | 50x slower      |

**Note**: Performance gap is expected for research implementation without hardware acceleration.

## Build Instructions

```bash
cd CHARM-B
make aead                    # Build AEAD library
make aead_benchmark         # Build benchmarks
./build/charmb_aead_benchmark   # Run tests
```

## Testing

### Functionality Tests
```bash
# Run built-in functionality tests
./build/charmb_aead_benchmark

# Expected output:
# ✅ Encryption successful
# ✅ Decryption successful  
# ✅ Plaintext matches!
```

### Custom Tests
```c
// Test vector validation
uint8_t key[32] = {1,2,3,...,32};
uint8_t nonce[12] = {1,2,3,...,12};
uint8_t plaintext[] = "Test message";
// ... perform encrypt/decrypt cycle
assert(memcmp(original, recovered, len) == 0);
```

## Limitations and Considerations

### Current Limitations
- **Performance**: Significantly slower than hardware-accelerated AES-GCM
- **Nonce Misuse**: Not resistant to nonce reuse attacks
- **Research Grade**: Not yet suitable for production without security review

### Security Considerations
- Requires careful nonce management
- CHARM-B hash function is experimental
- No formal security proofs available
- Side-channel analysis incomplete

### When to Use CHARM-B AEAD
✅ **Good for**:
- Research and prototyping
- Systems without AES-NI support
- Environments requiring CHARM-B specific properties
- Small, fixed-size payloads

❌ **Not recommended for**:
- High-throughput applications
- Production systems without security review
- Large payload encryption
- Applications requiring misuse resistance

## Future Enhancements

### Planned Improvements
- [ ] Performance optimization for larger payloads
- [ ] Misuse-resistant variant (SIV-style)
- [ ] Hardware acceleration support
- [ ] Formal security analysis
- [ ] Side-channel hardening

### Research Directions
- Integration with CHARM-B specific entropy properties
- Custom authentication modes leveraging CHARM-B features
- Optimizations for ultra-constrained environments

## License and Attribution

CHARM-B AEAD is part of the CHARM cryptographic framework.
Licensed under the CHARM License 2025.

## References

1. CHARM-B Algorithm Specification (see main repository)
2. RFC 5869: HMAC-based Extract-and-Expand Key Derivation Function (HKDF)
3. RFC 2104: HMAC: Keyed-Hashing for Message Authentication
4. CHARM-B Performance Analysis (see results/ directory)

---

**⚠️ Security Notice**: This is an experimental implementation for research purposes. Do not use in production systems without thorough security review and testing.