/**
 * @file charm_aeas.h
 * @brief CHARM with Advanced Encryption and Adaptive Streaming (AEAS)
 * 
 * Experimental implementation combining CHARM's security properties with
 * AEAS optimization techniques for enhanced performance on small to medium payloads.
 */

#ifndef CHARM_AEAS_H
#define CHARM_AEAS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM-AEAS Constants
 */
#define CHARM_AEAS_KEY_SIZE     32    // 256-bit key
#define CHARM_AEAS_NONCE_SIZE   16    // 128-bit nonce
#define CHARM_AEAS_TAG_SIZE     32    // 256-bit authentication tag
#define CHARM_AEAS_BATCH_SIZE   128   // Maximum batch size for keystream generation

/**
 * @brief CHARM-AEAS Status Codes
 */
typedef enum {
    CHARM_AEAS_SUCCESS = 0,
    CHARM_AEAS_ERROR_NULL_POINTER = -1,
    CHARM_AEAS_ERROR_INVALID_SIZE = -2,
    CHARM_AEAS_ERROR_AUTH_FAILED = -3,
    CHARM_AEAS_ERROR_BUFFER_TOO_SMALL = -4
} charm_aeas_status_t;

/**
 * @brief CHARM-AEAS Optimization Modes
 */
typedef enum {
    CHARM_AEAS_MODE_STANDARD = 0,     // Standard CHARM with AEAS optimizations
    CHARM_AEAS_MODE_BATCHED = 1,      // Batched keystream generation
    CHARM_AEAS_MODE_SIMD = 2,         // SIMD-optimized operations
    CHARM_AEAS_MODE_STACK = 3,        // Stack-based allocation
    CHARM_AEAS_MODE_POLY1305 = 4      // Poly1305 authentication
} charm_aeas_mode_t;

/**
 * @brief CHARM-AEAS Context for batched operations
 */
typedef struct {
    uint8_t key[CHARM_AEAS_KEY_SIZE];
    uint8_t nonce[CHARM_AEAS_NONCE_SIZE];
    uint8_t keystream_batch[CHARM_AEAS_BATCH_SIZE];
    size_t batch_size;
    uint32_t counter;
    charm_aeas_mode_t mode;
    bool initialized;
} charm_aeas_context_t;

/**
 * @brief Initialize CHARM-AEAS context for batched operations
 * 
 * @param ctx Context to initialize
 * @param key Encryption key (32 bytes)
 * @param nonce Nonce (16 bytes)
 * @param mode Optimization mode
 * @return Status code
 */
charm_aeas_status_t charm_aeas_init(
    charm_aeas_context_t* ctx,
    const uint8_t key[CHARM_AEAS_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAS_NONCE_SIZE],
    charm_aeas_mode_t mode
);

/**
 * @brief CHARM-AEAS Encryption with batched keystream generation
 * 
 * Optimized for small payloads using pre-generated keystream batches
 * and SIMD operations where available.
 * 
 * @param ctx Initialized CHARM-AEAS context
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag (32 bytes)
 * @return Status code
 */
charm_aeas_status_t charm_aeas_encrypt(
    charm_aeas_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAS_TAG_SIZE]
);

/**
 * @brief CHARM-AEAS Decryption with batched keystream generation
 * 
 * @param ctx Initialized CHARM-AEAS context
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag (32 bytes)
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code (CHARM_AEAS_ERROR_AUTH_FAILED if authentication fails)
 */
charm_aeas_status_t charm_aeas_decrypt(
    charm_aeas_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAS_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief SIMD-optimized XOR operation
 * 
 * Uses SSE2/AVX2 when available for 64-bit parallel XOR operations
 * 
 * @param a Input buffer A
 * @param b Input buffer B
 * @param out Output buffer (a XOR b)
 * @param len Length of buffers
 */
void charm_aeas_simd_xor(
    const uint8_t* a,
    const uint8_t* b,
    uint8_t* out,
    size_t len
);

/**
 * @brief Generate batched keystream for small payloads
 * 
 * Pre-generates multiple keystream blocks to amortize overhead
 * 
 * @param ctx CHARM-AEAS context
 * @param batch_size Size of batch to generate (≤128 bytes)
 * @return Status code
 */
charm_aeas_status_t charm_aeas_generate_batch(
    charm_aeas_context_t* ctx,
    size_t batch_size
);

/**
 * @brief Stack-based HMAC for small data
 * 
 * Optimized HMAC implementation using stack allocation
 * for improved performance on small inputs
 * 
 * @param key HMAC key
 * @param key_len Key length
 * @param data Data to authenticate
 * @param data_len Data length (must be ≤256 bytes)
 * @param hmac Output HMAC (32 bytes)
 * @return Status code
 */
charm_aeas_status_t charm_aeas_stack_hmac(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[32]
);

/**
 * @brief Poly1305 authentication (RFC 7539 compatible)
 * 
 * Alternative authentication option for AEAS mode
 * 
 * @param key Poly1305 key (32 bytes)
 * @param data Data to authenticate
 * @param data_len Data length
 * @param tag Output tag (16 bytes)
 * @return Status code
 */
charm_aeas_status_t charm_aeas_poly1305(
    const uint8_t key[32],
    const uint8_t* data, size_t data_len,
    uint8_t tag[16]
);

/**
 * @brief Benchmark CHARM-AEAS performance
 * 
 * @param mode AEAS optimization mode to test
 * @param payload_size Size of payload to test
 * @param iterations Number of iterations
 * @param encrypt_mbps Output encryption throughput
 * @param decrypt_mbps Output decryption throughput
 * @return Status code
 */
charm_aeas_status_t charm_aeas_benchmark(
    charm_aeas_mode_t mode,
    size_t payload_size, int iterations,
    double* encrypt_mbps, double* decrypt_mbps
);

/**
 * @brief Clean up CHARM-AEAS context
 * 
 * Securely clears sensitive data from context
 * 
 * @param ctx Context to clean
 */
void charm_aeas_cleanup(charm_aeas_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_AEAS_H */