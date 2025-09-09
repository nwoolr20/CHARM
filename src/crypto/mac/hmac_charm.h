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
 * @file hmac_charm.h
 * @brief HMAC construction using CHARM hash function
 * 
 * Implementation of HMAC (RFC 2104) using CHARM as the underlying hash function.
 * Provides message authentication with 128-256 bit security levels.
 */

#ifndef HMAC_CHARM_H
#define HMAC_CHARM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HMAC-CHARM status codes
 */
typedef enum {
    HMAC_CHARM_SUCCESS = 0,
    HMAC_CHARM_ERROR_NULL_POINTER = -1,
    HMAC_CHARM_ERROR_INVALID_SIZE = -2,
    HMAC_CHARM_ERROR_HASH_FAILED = -3
} hmac_charm_status_t;

/**
 * @brief HMAC-CHARM output sizes
 */
#define HMAC_CHARM_256_SIZE 32  // 256-bit HMAC output
#define HMAC_CHARM_384_SIZE 48  // 384-bit HMAC output (future)
#define HMAC_CHARM_512_SIZE 64  // 512-bit HMAC output (future)

/**
 * @brief HMAC-CHARM context for streaming computation
 */
typedef struct {
    uint8_t key_pad[64];        // Padded key for inner/outer hash
    uint8_t inner_state[128];   // CHARM context for inner hash
    uint8_t outer_state[128];   // CHARM context for outer hash
    size_t key_len;             // Original key length
    uint8_t initialized;        // Initialization flag
} hmac_charm_ctx_t;

/**
 * @brief One-shot HMAC-CHARM computation
 * 
 * Computes HMAC-CHARM-256 in a single operation for maximum performance.
 * 
 * @param key HMAC key (any length, will be hashed if > 64 bytes)
 * @param key_len Length of key in bytes
 * @param data Message to authenticate
 * @param data_len Length of message in bytes
 * @param hmac Output buffer for HMAC (32 bytes)
 * @return Status code
 */
hmac_charm_status_t hmac_charm_256(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[HMAC_CHARM_256_SIZE]
);

/**
 * @brief Initialize HMAC-CHARM context for streaming computation
 * 
 * @param ctx HMAC context to initialize
 * @param key HMAC key
 * @param key_len Length of key in bytes
 * @return Status code
 */
hmac_charm_status_t hmac_charm_init(
    hmac_charm_ctx_t* ctx,
    const uint8_t* key, size_t key_len
);

/**
 * @brief Update HMAC-CHARM with additional data
 * 
 * @param ctx HMAC context
 * @param data Input data
 * @param data_len Length of data in bytes
 * @return Status code
 */
hmac_charm_status_t hmac_charm_update(
    hmac_charm_ctx_t* ctx,
    const uint8_t* data, size_t data_len
);

/**
 * @brief Finalize HMAC-CHARM computation
 * 
 * @param ctx HMAC context
 * @param hmac Output buffer for HMAC (32 bytes)
 * @return Status code
 */
hmac_charm_status_t hmac_charm_final(
    hmac_charm_ctx_t* ctx,
    uint8_t hmac[HMAC_CHARM_256_SIZE]
);

/**
 * @brief Constant-time memory comparison for HMAC verification
 * 
 * Verifies HMAC tag in constant time to prevent timing attacks.
 * 
 * @param hmac1 First HMAC value
 * @param hmac2 Second HMAC value
 * @param len Length to compare (typically 32 bytes)
 * @return 0 if equal, non-zero if different
 */
int hmac_charm_verify(
    const uint8_t* hmac1,
    const uint8_t* hmac2,
    size_t len
);

/**
 * @brief Benchmark HMAC-CHARM performance
 * 
 * @param message_size Size of message to test
 * @param iterations Number of iterations
 * @param mbps Output throughput in MB/s
 * @return Status code
 */
hmac_charm_status_t hmac_charm_benchmark(
    size_t message_size, int iterations,
    double* mbps
);

#ifdef __cplusplus
}
#endif

#endif /* HMAC_CHARM_H */