/**
 * @file charm_digest.h
 * @brief Digest-specific exports for the CHARM Entropic Collapse Engine
 * 
 * This header defines the public interface for using CHARM as a
 * cryptographic hashing engine.
 */

#ifndef CHARM_DIGEST_H
#define CHARM_DIGEST_H

#include "charm_api.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Available digest sizes
 */
typedef enum {
    CHARM_DIGEST_256 = 256,  /**< 256-bit (32-byte) digest */
    CHARM_DIGEST_512 = 512   /**< 512-bit (64-byte) digest */
} charm_digest_size_t;

/**
 * @brief Context for incremental digest operations
 */
typedef struct charm_digest_ctx_t* charm_digest_ctx;

/**
 * @brief Initialize a new digest context
 * 
 * @param ctx Pointer to receive the initialized digest context
 * @param size Desired digest size
 * @return Status code indicating success or failure
 */
charm_status_t charm_digest_init(charm_digest_ctx* ctx, charm_digest_size_t size);

/**
 * @brief Update the digest with additional data
 * 
 * @param ctx Digest context
 * @param data Pointer to input data
 * @param length Length of input data in bytes
 * @return Status code indicating success or failure
 */
charm_status_t charm_digest_update(charm_digest_ctx ctx, const void* data, size_t length);

/**
 * @brief Finalize the digest and output the result
 * 
 * @param ctx Digest context
 * @param output Buffer to receive the digest
 * @param output_size Size of the output buffer in bytes
 * @return Status code indicating success or failure
 */
charm_status_t charm_digest_final(charm_digest_ctx ctx, void* output, size_t output_size);

/**
 * @brief Destroy a digest context and release resources
 * 
 * @param ctx Digest context to destroy
 */
void charm_digest_destroy(charm_digest_ctx ctx);

/**
 * @brief Compute a digest in a single operation
 * 
 * @param size Desired digest size
 * @param data Pointer to input data
 * @param length Length of input data in bytes
 * @param output Buffer to receive the digest
 * @param output_size Size of the output buffer in bytes
 * @return Status code indicating success or failure
 */
charm_status_t charm_digest(charm_digest_size_t size, const void* data, size_t length,
                           void* output, size_t output_size);

/**
 * @brief Get the entropy quality score for the last digest operation
 * 
 * @param ctx Digest context
 * @return Entropy quality score (0-100)
 */
uint32_t charm_digest_entropy_quality(charm_digest_ctx ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_DIGEST_H */
