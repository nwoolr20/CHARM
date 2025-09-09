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
 * @file aeas.h
 * @brief CHARM-AEAS (Authenticated Encryption with Adaptive State) Public API
 * 
 * CHARM-AEAS is an AEAD construction using CHARM as the underlying primitive
 * with Encrypt-then-MAC design similar to ChaCha20-Poly1305.
 * 
 * Construction:
 * - Key derivation using HKDF-CHARM
 * - Stream encryption using HMAC-CHARM keystream
 * - Authentication using Poly1305 MAC
 * - 96-bit nonces, 32-byte keys, 16-byte tags
 */

#ifndef AEAS_H
#define AEAS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AEAS algorithm constants
 */
#define AEAS_KEY_SIZE        32  // Master key size in bytes
#define AEAS_NONCE_SIZE      12  // Nonce size in bytes (96-bit)
#define AEAS_TAG_SIZE        16  // Authentication tag size in bytes

/**
 * @brief AEAS return codes
 */
#define AEAS_SUCCESS         0   // Operation successful
#define AEAS_ERROR_INVALID  -1   // Invalid parameter
#define AEAS_ERROR_AUTH     -2   // Authentication failure
#define AEAS_ERROR_MEMORY   -3   // Memory allocation failure

/**
 * @brief AEAS nonce context for counter-based nonces
 * 
 * Provides helper for applications that want counter-based nonces.
 * The context maintains a 64-bit counter to populate the low bits
 * of a nonce prefix.
 */
typedef struct {
    uint8_t prefix[4];          // 32-bit nonce prefix
    uint64_t counter;           // 64-bit message counter
} aeas_nonce_ctx_t;

/**
 * @brief Encrypt and authenticate data using CHARM-AEAS
 * 
 * @param key 32-byte master key
 * @param nonce 12-byte nonce (must be unique for each message with same key)
 * @param aad Additional authenticated data (can be NULL if aad_len is 0)
 * @param aad_len Length of AAD in bytes
 * @param plaintext Input plaintext (can be NULL if pt_len is 0)
 * @param pt_len Length of plaintext in bytes
 * @param ciphertext Output ciphertext buffer (must be at least pt_len bytes)
 * @param tag Output authentication tag (16 bytes)
 * @return AEAS_SUCCESS on success, negative error code on failure
 */
int aeas_encrypt(const uint8_t key[AEAS_KEY_SIZE],
                 const uint8_t nonce[AEAS_NONCE_SIZE],
                 const uint8_t* aad, size_t aad_len,
                 const uint8_t* plaintext, size_t pt_len,
                 uint8_t* ciphertext,
                 uint8_t tag[AEAS_TAG_SIZE]);

/**
 * @brief Decrypt and verify data using CHARM-AEAS
 * 
 * @param key 32-byte master key
 * @param nonce 12-byte nonce
 * @param aad Additional authenticated data (can be NULL if aad_len is 0)
 * @param aad_len Length of AAD in bytes
 * @param ciphertext Input ciphertext (can be NULL if ct_len is 0)
 * @param ct_len Length of ciphertext in bytes
 * @param tag Authentication tag to verify (16 bytes)
 * @param plaintext Output plaintext buffer (must be at least ct_len bytes)
 * @return AEAS_SUCCESS on success, AEAS_ERROR_AUTH on auth failure, other negative on error
 */
int aeas_decrypt(const uint8_t key[AEAS_KEY_SIZE],
                 const uint8_t nonce[AEAS_NONCE_SIZE],
                 const uint8_t* aad, size_t aad_len,
                 const uint8_t* ciphertext, size_t ct_len,
                 const uint8_t tag[AEAS_TAG_SIZE],
                 uint8_t* plaintext);

/**
 * @brief Initialize nonce context for counter-based nonces
 * 
 * @param ctx Nonce context to initialize
 * @param prefix 4-byte nonce prefix (application-specific)
 * @return AEAS_SUCCESS on success, negative on error
 */
int aeas_nonce_init(aeas_nonce_ctx_t* ctx, const uint8_t prefix[4]);

/**
 * @brief Generate next nonce from context
 * 
 * @param ctx Nonce context
 * @param nonce Output 12-byte nonce
 * @return AEAS_SUCCESS on success, negative on error (e.g., counter overflow)
 */
int aeas_nonce_next(aeas_nonce_ctx_t* ctx, uint8_t nonce[AEAS_NONCE_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* AEAS_H */