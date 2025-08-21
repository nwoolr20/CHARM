/**
 * @file poly1305.h
 * @brief Poly1305 one-time authenticator (RFC 7539)
 * 
 * Constant-time implementation of Poly1305 MAC algorithm.
 * Uses 32-byte key and produces 16-byte tag.
 */

#ifndef POLY1305_H
#define POLY1305_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Poly1305 constants
 */
#define POLY1305_KEY_SIZE    32  // Key size in bytes
#define POLY1305_TAG_SIZE    16  // Tag size in bytes
#define POLY1305_BLOCK_SIZE  16  // Block size in bytes

/**
 * @brief Poly1305 context for streaming operations
 */
typedef struct {
    uint32_t r[5];              // r value (clamped)
    uint32_t h[5];              // accumulator h
    uint32_t s[4];              // s value (for final addition)
    uint8_t buffer[POLY1305_BLOCK_SIZE];  // Input buffer
    size_t buffer_len;          // Buffered bytes
    uint8_t finalized;          // Whether context is finalized
} poly1305_ctx_t;

/**
 * @brief One-shot Poly1305 MAC computation
 * 
 * @param key 32-byte one-time key
 * @param data Input data to authenticate
 * @param data_len Length of input data in bytes
 * @param tag Output 16-byte authentication tag
 * @return 0 on success, negative on error
 */
int poly1305_mac(const uint8_t key[POLY1305_KEY_SIZE],
                 const uint8_t* data, size_t data_len,
                 uint8_t tag[POLY1305_TAG_SIZE]);

/**
 * @brief Initialize Poly1305 context
 * 
 * @param ctx Context to initialize
 * @param key 32-byte one-time key
 * @return 0 on success, negative on error
 */
int poly1305_init(poly1305_ctx_t* ctx, const uint8_t key[POLY1305_KEY_SIZE]);

/**
 * @brief Update Poly1305 context with data
 * 
 * @param ctx Context to update
 * @param data Input data
 * @param data_len Length of input data
 * @return 0 on success, negative on error
 */
int poly1305_update(poly1305_ctx_t* ctx, const uint8_t* data, size_t data_len);

/**
 * @brief Finalize Poly1305 computation
 * 
 * @param ctx Context to finalize
 * @param tag Output 16-byte authentication tag
 * @return 0 on success, negative on error
 */
int poly1305_final(poly1305_ctx_t* ctx, uint8_t tag[POLY1305_TAG_SIZE]);

/**
 * @brief Clear Poly1305 context
 * 
 * @param ctx Context to clear
 */
void poly1305_clear(poly1305_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* POLY1305_H */