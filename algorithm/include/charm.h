/**
 * @file charm.h
 * @brief CHARM Algorithm API - Clean, Frozen Specification
 * 
 * CHARM (Chaotic Hierarchical Adaptive Resilient Mechanism) Algorithm
 * A linear streaming hash with entropy-native design
 * 
 * This header defines the frozen algorithm specification for CHARM-n where n ∈ {256, 384, 512}
 * 
 * Version: CHARM/1
 * Mode: Linear streaming hash (NOT tree-based, NOT Merkle construction)
 * Security: Entropy-native cryptographic hash with optional keyed mode
 */

#ifndef CHARM_ALGORITHM_H
#define CHARM_ALGORITHM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM Algorithm Version
 */
#define CHARM_VERSION_MAJOR 1
#define CHARM_VERSION_MINOR 0
#define CHARM_VERSION_STRING "CHARM/1.0"

/**
 * @brief CHARM Variants - Security levels and output sizes
 */
typedef enum {
    CHARM_256 = 256,    // 256-bit output, 128-bit security level
    CHARM_384 = 384,    // 384-bit output, 192-bit security level  
    CHARM_512 = 512     // 512-bit output, 256-bit security level
} charm_variant_t;

/**
 * @brief CHARM Algorithm Parameters (frozen)
 * 
 * These parameters are fixed in the algorithm specification and cannot be changed
 * without creating a new algorithm version.
 */
#define CHARM_BLOCK_BYTES 64        // Fixed block size (like SHA-2)
#define CHARM_CHUNK_BYTES (16*1024) // Fixed chunk size for processing
#define CHARM_MAX_ROUNDS 12         // Maximum mixing rounds
#define CHARM_MIN_ROUNDS 4          // Minimum mixing rounds

/**
 * @brief CHARM Parameter Block Structure
 * 
 * Encodes all algorithm parameters for deterministic operation
 */
typedef struct {
    uint8_t version;                // Algorithm version (1 for CHARM/1)
    uint16_t out_bits;             // Output size in bits (256, 384, or 512)
    uint8_t flags;                 // Mode flags (keyed, xof, custom)
    uint8_t reserved[12];          // Reserved for future use, must be zero
} charm_params_t;

/**
 * @brief CHARM Algorithm Context
 * 
 * Opaque context structure for streaming operations
 */
typedef struct charm_ctx charm_ctx_t;

/**
 * @brief CHARM Algorithm Flags
 */
#define CHARM_FLAG_KEYED    0x01   // Keyed mode (KMAC-like)
#define CHARM_FLAG_XOF      0x02   // Extended output function mode
#define CHARM_FLAG_CUSTOM   0x04   // Custom domain separation

/**
 * @brief Initialize CHARM context
 * 
 * @param ctx Context to initialize (allocated by caller)
 * @param params Algorithm parameters
 * @param key Optional key for keyed mode (NULL for unkeyed, 0 or 32 bytes)
 * @param key_len Length of key (0 or 32)
 * @param custom Optional customization string (NULL if unused)
 * @param custom_len Length of customization string
 * @return 0 on success, negative on error
 */
int charm_init(charm_ctx_t* ctx, 
               const charm_params_t* params,
               const uint8_t* key, size_t key_len,
               const uint8_t* custom, size_t custom_len);

/**
 * @brief Update CHARM context with data
 * 
 * @param ctx Context to update
 * @param data Input data
 * @param len Length of input data
 * @return 0 on success, negative on error
 */
int charm_update(charm_ctx_t* ctx, const void* data, size_t len);

/**
 * @brief Finalize CHARM computation
 * 
 * @param ctx Context to finalize
 * @param out Output buffer (size must match params.out_bits/8)
 * @return 0 on success, negative on error
 */
int charm_final(charm_ctx_t* ctx, uint8_t* out);

/**
 * @brief One-shot CHARM hash computation
 * 
 * @param variant CHARM variant (256, 384, or 512)
 * @param data Input data
 * @param len Length of input data
 * @param out Output buffer (size must match variant/8)
 * @return 0 on success, negative on error
 */
int charm_hash(charm_variant_t variant, const void* data, size_t len, uint8_t* out);

/**
 * @brief One-shot CHARM keyed hash (KMAC-like)
 * 
 * @param variant CHARM variant (256, 384, or 512)
 * @param key Key material (32 bytes)
 * @param data Input data
 * @param len Length of input data
 * @param custom Optional customization string
 * @param custom_len Length of customization string
 * @param out Output buffer (size must match variant/8)
 * @return 0 on success, negative on error
 */
int charm_keyed_hash(charm_variant_t variant,
                     const uint8_t key[32],
                     const void* data, size_t len,
                     const uint8_t* custom, size_t custom_len,
                     uint8_t* out);

/**
 * @brief Initialize CHARM XOF context (if supported)
 * 
 * @param ctx Context to initialize
 * @param variant CHARM variant
 * @param key Optional key (NULL for unkeyed)
 * @param custom Optional customization string
 * @param custom_len Length of customization string
 * @return 0 on success, negative on error
 */
int charm_xof_init(charm_ctx_t* ctx,
                   charm_variant_t variant,
                   const uint8_t* key,
                   const uint8_t* custom, size_t custom_len);

/**
 * @brief Extract output from CHARM XOF
 * 
 * @param ctx XOF context
 * @param out Output buffer
 * @param len Number of bytes to extract
 * @return 0 on success, negative on error
 */
int charm_xof_squeeze(charm_ctx_t* ctx, uint8_t* out, size_t len);

/**
 * @brief Get context size for allocation
 * 
 * @return Size in bytes needed for charm_ctx_t
 */
size_t charm_ctx_size(void);

/**
 * @brief Get algorithm version string
 * 
 * @return Version string
 */
const char* charm_version(void);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ALGORITHM_H */