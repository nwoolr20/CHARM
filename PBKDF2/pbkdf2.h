/*
 * CHARM – High-Performance Entropy-Native Cryptographic Framework
 * Copyright (c) 2025 Nicholas Woolridge & NOCTRL™ (Nô)
 *
 * This software is licensed under the CHARM License 2025.
 * Use, modification, and distribution are permitted only with
 * verified, real-world test results demonstrating correct
 * functionality, performance, and security.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 * See LICENSE in the repository root for full license details.
 */

/**
 * @file pbkdf2.h
 * @brief PBKDF2 (Password-Based Key Derivation Function 2) Implementation for CHARM
 *
 * This module provides a secure implementation of PBKDF2 (RFC 2898) integrated
 * with the CHARM Security Suite. It uses HMAC-SHA256 as the underlying PRF
 * and provides secure key derivation from passwords with salt and iteration count.
 *
 * Features:
 * - RFC 2898 compliant PBKDF2 implementation
 * - HMAC-SHA256 pseudorandom function
 * - Configurable iteration counts (minimum 10,000 recommended)
 * - Support for arbitrary salt lengths (minimum 16 bytes recommended)
 * - Integration with CHARM entropy subsystem
 * - Memory-safe implementation with secure cleanup
 */

#ifndef PBKDF2_H
#define PBKDF2_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* PBKDF2 Configuration Constants */
#define PBKDF2_MIN_ITERATIONS 10000      /* Minimum recommended iterations */
#define PBKDF2_DEFAULT_ITERATIONS 100000 /* Default iteration count */
#define PBKDF2_MIN_SALT_LENGTH 16        /* Minimum salt length in bytes */
#define PBKDF2_DEFAULT_SALT_LENGTH 32    /* Default salt length in bytes */
#define PBKDF2_MAX_KEY_LENGTH 1024       /* Maximum derived key length */

/* PBKDF2 Error Codes */
typedef enum {
    PBKDF2_SUCCESS = 0,
    PBKDF2_ERROR_INVALID_PARAMS = -1,
    PBKDF2_ERROR_INSUFFICIENT_ITERATIONS = -2,
    PBKDF2_ERROR_SALT_TOO_SHORT = -3,
    PBKDF2_ERROR_KEY_TOO_LONG = -4,
    PBKDF2_ERROR_MEMORY = -5,
    PBKDF2_ERROR_CRYPTO = -6
} pbkdf2_result_t;

/* PBKDF2 Configuration Structure */
typedef struct {
    uint32_t iterations;           /* Number of iterations */
    size_t salt_length;           /* Salt length in bytes */
    size_t derived_key_length;    /* Desired output key length */
    uint8_t use_charm_entropy;    /* Use CHARM entropy for enhanced security */
    uint8_t secure_cleanup;       /* Perform secure memory cleanup */
} pbkdf2_config_t;

/* PBKDF2 Context Structure */
typedef struct {
    pbkdf2_config_t config;
    uint8_t* salt;
    uint8_t* derived_key;
    size_t password_length;
    uint8_t initialized;
} pbkdf2_context_t;

/**
 * @brief Initialize PBKDF2 with default secure configuration
 * 
 * Sets up a PBKDF2 context with secure default parameters:
 * - 100,000 iterations
 * - 32-byte salt length
 * - CHARM entropy integration enabled
 * - Secure memory cleanup enabled
 * 
 * @param config Output configuration structure
 * @return PBKDF2_SUCCESS on success, error code on failure
 */
pbkdf2_result_t pbkdf2_init_default_config(pbkdf2_config_t* config);

/**
 * @brief Initialize PBKDF2 with custom configuration
 * 
 * @param config PBKDF2 configuration parameters
 * @param ctx PBKDF2 context to initialize
 * @return PBKDF2_SUCCESS on success, error code on failure
 */
pbkdf2_result_t pbkdf2_init(const pbkdf2_config_t* config, pbkdf2_context_t* ctx);

/**
 * @brief Generate cryptographically secure salt
 * 
 * Generates a random salt using the system's secure random number generator,
 * optionally enhanced with CHARM entropy subsystem.
 * 
 * @param salt Output buffer for salt (must be pre-allocated)
 * @param salt_length Length of salt to generate
 * @param use_charm_entropy Whether to use CHARM entropy enhancement
 * @return PBKDF2_SUCCESS on success, error code on failure
 */
pbkdf2_result_t pbkdf2_generate_salt(uint8_t* salt, size_t salt_length, uint8_t use_charm_entropy);

/**
 * @brief Derive key from password using PBKDF2
 * 
 * Core PBKDF2 function that derives a cryptographic key from a password
 * using the specified salt and iteration count.
 * 
 * @param password Input password
 * @param password_length Length of password in bytes
 * @param salt Salt value
 * @param salt_length Length of salt in bytes
 * @param iterations Number of iterations (minimum 10,000)
 * @param derived_key Output buffer for derived key
 * @param key_length Desired length of derived key
 * @return PBKDF2_SUCCESS on success, error code on failure
 */
pbkdf2_result_t pbkdf2_derive_key(
    const uint8_t* password, size_t password_length,
    const uint8_t* salt, size_t salt_length,
    uint32_t iterations,
    uint8_t* derived_key, size_t key_length
);

/**
 * @brief Complete PBKDF2 operation with context
 * 
 * High-level function that performs complete PBKDF2 operation using
 * a pre-configured context.
 * 
 * @param ctx PBKDF2 context (must be initialized)
 * @param password Input password
 * @param password_length Length of password
 * @return PBKDF2_SUCCESS on success, error code on failure
 */
pbkdf2_result_t pbkdf2_derive_with_context(
    pbkdf2_context_t* ctx,
    const uint8_t* password, size_t password_length
);

/**
 * @brief Verify password against stored key
 * 
 * Verifies a password by deriving a key with the same parameters
 * and comparing it to a stored key using constant-time comparison.
 * 
 * @param password Password to verify
 * @param password_length Length of password
 * @param salt Salt used in original derivation
 * @param salt_length Length of salt
 * @param iterations Iteration count used in original derivation
 * @param stored_key Previously derived key to compare against
 * @param key_length Length of stored key
 * @return PBKDF2_SUCCESS if password matches, error code otherwise
 */
pbkdf2_result_t pbkdf2_verify_password(
    const uint8_t* password, size_t password_length,
    const uint8_t* salt, size_t salt_length,
    uint32_t iterations,
    const uint8_t* stored_key, size_t key_length
);

/**
 * @brief Get recommended iteration count for current system
 * 
 * Benchmarks PBKDF2 performance on the current system and returns
 * an iteration count that provides reasonable security (target: 100ms).
 * 
 * @return Recommended iteration count
 */
uint32_t pbkdf2_get_recommended_iterations(void);

/**
 * @brief Convert PBKDF2 result to string
 * 
 * @param result PBKDF2 result code
 * @return Human-readable string describing the result
 */
const char* pbkdf2_result_to_string(pbkdf2_result_t result);

/**
 * @brief Securely clear PBKDF2 context
 * 
 * Securely clears all sensitive data from the PBKDF2 context,
 * including derived keys and internal state.
 * 
 * @param ctx PBKDF2 context to clear
 */
void pbkdf2_cleanup(pbkdf2_context_t* ctx);

/**
 * @brief Print PBKDF2 configuration information
 * 
 * @param config Configuration to display
 */
void pbkdf2_print_config(const pbkdf2_config_t* config);

#ifdef __cplusplus
}
#endif

#endif /* PBKDF2_H */