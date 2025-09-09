# PBKDF2 Implementation for CHARM Security Suite

This directory contains a secure, RFC 2898 compliant implementation of PBKDF2 (Password-Based Key Derivation Function 2) integrated with the CHARM Security Suite.

## Overview

PBKDF2 is a key derivation function that applies a pseudorandom function (HMAC-CHARM-256 in this implementation) to derive cryptographic keys from passwords. It uses a salt and iteration count to make dictionary attacks computationally expensive.

## Features

- **RFC 2898 Compliant**: Full implementation of the PBKDF2 standard
- **HMAC-CHARM-256 PRF**: Uses HMAC-CHARM-256 as the pseudorandom function
- **Secure Defaults**: 100,000 iterations, 32-byte salt by default
- **CHARM Integration**: Optional integration with CHARM entropy subsystem
- **Memory Safety**: Secure memory cleanup and constant-time comparison
- **Configurable**: Customizable iteration counts, salt lengths, and key lengths

## Security Parameters

### Default Configuration
- **Iterations**: 100,000 (provides strong protection against GPU attacks)
- **Salt Length**: 32 bytes (256-bit salt)
- **Key Length**: 32 bytes (256-bit key)
- **PRF**: HMAC-CHARM-256

### Minimum Requirements
- **Iterations**: 10,000 (absolute minimum for security)
- **Salt Length**: 16 bytes (128-bit minimum)
- **Key Length**: 1 byte minimum, 1024 bytes maximum

## Files

- `pbkdf2.h` - Header file with API definitions and constants
- `pbkdf2.c` - Implementation of PBKDF2 algorithm
- `README.md` - This documentation file

## API Usage

### Basic Key Derivation

```c
#include "pbkdf2.h"

// Derive a key from password and salt
const char* password = "user_password";
uint8_t salt[32];
uint8_t derived_key[32];

// Generate secure salt
pbkdf2_generate_salt(salt, sizeof(salt), 1);

// Derive key with 100,000 iterations
pbkdf2_result_t result = pbkdf2_derive_key(
    (uint8_t*)password, strlen(password),
    salt, sizeof(salt),
    100000,
    derived_key, sizeof(derived_key)
);

if (result == PBKDF2_SUCCESS) {
    // Use derived_key for encryption, authentication, etc.
}
```

### Using Context API

```c
pbkdf2_config_t config;
pbkdf2_context_t ctx;

// Initialize with default secure configuration
pbkdf2_init_default_config(&config);
pbkdf2_init(&config, &ctx);

// Derive key using context
const char* password = "user_password";
result = pbkdf2_derive_with_context(&ctx, 
    (uint8_t*)password, strlen(password));

if (result == PBKDF2_SUCCESS) {
    // Access derived key: ctx.derived_key
    // Access salt: ctx.salt
}

// Clean up
pbkdf2_cleanup(&ctx);
```

### Password Verification

```c
// Verify a password against stored parameters
pbkdf2_result_t result = pbkdf2_verify_password(
    (uint8_t*)password, strlen(password),
    stored_salt, salt_length,
    iterations,
    stored_key, key_length
);

if (result == PBKDF2_SUCCESS) {
    // Password is correct
} else {
    // Password verification failed
}
```

## Integration with CHARM CLI

The PBKDF2 implementation is integrated into the CHARM Security Suite CLI with the following commands:

```bash
# Derive key from password
charm_security_suite --pbkdf2-derive --password "mypassword" --iterations 100000

# Generate secure salt
charm_security_suite --pbkdf2-salt --length 32

# Verify password
charm_security_suite --pbkdf2-verify --password "mypassword" --salt-file salt.bin --key-file key.bin

# Get recommended iterations for current system
charm_security_suite --pbkdf2-benchmark
```

## Security Considerations

### Iteration Count
- Use at least 10,000 iterations (enforced minimum)
- Default 100,000 iterations provides strong protection
- Use `pbkdf2_get_recommended_iterations()` to get system-optimized count
- Higher iteration counts increase security but reduce performance

### Salt Generation
- Always use a unique, random salt for each password
- Minimum 16 bytes, recommend 32 bytes
- Use `pbkdf2_generate_salt()` for secure generation
- Store salt alongside derived key for verification

### Memory Safety
- Implementation uses secure memory clearing
- Constant-time comparison prevents timing attacks
- All sensitive data is zeroed after use

### Key Storage
- Store derived keys securely (encrypted at rest)
- Never store passwords, only derived keys
- Include salt and iteration count in stored parameters

## Performance

Approximate performance on modern hardware:
- **100,000 iterations**: ~100ms (recommended for user authentication)
- **10,000 iterations**: ~10ms (minimum security level)
- **1,000,000 iterations**: ~1s (high security applications)

Use `pbkdf2_get_recommended_iterations()` to benchmark your system and get optimal iteration counts.

## Error Handling

All functions return `pbkdf2_result_t` codes:
- `PBKDF2_SUCCESS`: Operation completed successfully
- `PBKDF2_ERROR_INVALID_PARAMS`: Invalid input parameters
- `PBKDF2_ERROR_INSUFFICIENT_ITERATIONS`: Too few iterations
- `PBKDF2_ERROR_SALT_TOO_SHORT`: Salt length below minimum
- `PBKDF2_ERROR_KEY_TOO_LONG`: Requested key length too large
- `PBKDF2_ERROR_MEMORY`: Memory allocation failed
- `PBKDF2_ERROR_CRYPTO`: Cryptographic operation failed

Use `pbkdf2_result_to_string()` to get human-readable error messages.

## Standards Compliance

This implementation follows:
- **RFC 2898**: PBKDF2 specification
- **FIPS 140-2**: Cryptographic module standards
- **NIST SP 800-132**: Recommendation for Password-Based Key Derivation

## Dependencies

- CHARM Security Suite (for HMAC-CHARM-256 implementation)
- Standard C library
- POSIX-compliant system (for secure random number generation)

## License

This PBKDF2 implementation is part of the CHARM Security Suite and is licensed under the same terms as the main project.