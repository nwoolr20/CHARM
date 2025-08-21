/**
 * @file hmac_charm.h
 * @brief HMAC implementation using CHARM-256 as the underlying hash function
 * 
 * Implements HMAC (RFC 2104) with CHARM-256 as the hash primitive.
 * Block size: 64 bytes (CHARM block size)
 * Output size: 32 bytes (CHARM-256 output)
 */

#ifndef HMAC_CHARM_H
#define HMAC_CHARM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HMAC-CHARM constants
 */
#define HMAC_CHARM_BLOCK_SIZE  64  // CHARM block size
#define HMAC_CHARM_OUTPUT_SIZE 32  // CHARM-256 output size

/**
 * @brief HMAC-CHARM context for streaming operations
 */
typedef struct {
    uint8_t key_pad[HMAC_CHARM_BLOCK_SIZE];  // Padded key for inner/outer hash
    void* charm_ctx;                          // CHARM context for streaming
    uint8_t finalized;                        // Whether context is finalized
} hmac_charm_ctx_t;

/**
 * @brief One-shot HMAC-CHARM computation
 * 
 * @param key HMAC key (any length)
 * @param key_len Length of key in bytes
 * @param data Input data to authenticate
 * @param data_len Length of input data in bytes
 * @param out Output HMAC (32 bytes)
 * @return 0 on success, negative on error
 */
int hmac_charm(const uint8_t* key, size_t key_len,
               const uint8_t* data, size_t data_len,
               uint8_t out[HMAC_CHARM_OUTPUT_SIZE]);

/**
 * @brief Initialize HMAC-CHARM context
 * 
 * @param ctx Context to initialize
 * @param key HMAC key (any length)
 * @param key_len Length of key in bytes
 * @return 0 on success, negative on error
 */
int hmac_charm_init(hmac_charm_ctx_t* ctx, const uint8_t* key, size_t key_len);

/**
 * @brief Update HMAC-CHARM context with data
 * 
 * @param ctx Context to update
 * @param data Input data
 * @param data_len Length of input data
 * @return 0 on success, negative on error
 */
int hmac_charm_update(hmac_charm_ctx_t* ctx, const uint8_t* data, size_t data_len);

/**
 * @brief Finalize HMAC-CHARM computation
 * 
 * @param ctx Context to finalize
 * @param out Output HMAC (32 bytes)
 * @return 0 on success, negative on error
 */
int hmac_charm_final(hmac_charm_ctx_t* ctx, uint8_t out[HMAC_CHARM_OUTPUT_SIZE]);

/**
 * @brief Clear HMAC-CHARM context
 * 
 * @param ctx Context to clear
 */
void hmac_charm_clear(hmac_charm_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* HMAC_CHARM_H */