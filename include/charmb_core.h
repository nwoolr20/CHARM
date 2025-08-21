/**
 * @file charmb_core.h
 * @brief CHARM-B (CHARMbit) - Ultra-optimized variant for small inputs ≤64B
 * 
 * CHARM-B is a specialized variant of CHARM optimized specifically for very
 * small inputs (≤64 bytes). It uses bit-level operations, specialized algorithms,
 * and minimal overhead to achieve maximum performance on tiny data packets.
 * 
 * Key features:
 * - Bit-level processing for micro-optimizations
 * - Specialized paths for 8B, 16B, 32B, 64B
 * - Zero malloc/context overhead 
 * - SIMD-optimized for small data patterns
 * - Ultra-fast entropy mixing for tiny inputs
 */

#ifndef CHARMB_CORE_H
#define CHARMB_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM-B status codes
 */
typedef enum {
    CHARMB_STATUS_OK = 0,           /**< Operation successful */
    CHARMB_STATUS_ERROR = -1,       /**< General error */
    CHARMB_STATUS_INVALID_ARG = -2, /**< Invalid argument */
    CHARMB_STATUS_TOO_LARGE = -3    /**< Input too large for CHARM-B (>64B) */
} charmb_status_t;

/**
 * @brief CHARM-B digest sizes - optimized for small inputs
 */
typedef enum {
    CHARMB_DIGEST_128 = 16,    /**< 128-bit digest for micro inputs */
    CHARMB_DIGEST_256 = 32     /**< 256-bit digest for compatibility */
} charmb_digest_size_t;

/**
 * @brief Ultra-fast CHARM-B hash for micro inputs (≤64B)
 * 
 * This is the primary CHARM-B function optimized for maximum performance
 * on very small inputs. Uses bit-level operations and specialized algorithms.
 * 
 * @param data Input data (must be ≤ 64 bytes)
 * @param data_size Size of input data in bytes
 * @param digest Output buffer for digest
 * @param digest_size Size of digest (16 or 32 bytes)
 * @return charmb_status_t Status code
 */
charmb_status_t charmb_hash(const uint8_t* data, size_t data_size, 
                           uint8_t* digest, charmb_digest_size_t digest_size);

/**
 * @brief Ultra-fast 8-byte specialized hash
 * 
 * Hyper-optimized for exactly 8 bytes (common for IDs, timestamps, etc.)
 * 
 * @param data Input data (exactly 8 bytes)
 * @param digest Output buffer (16 or 32 bytes)
 * @param digest_size Size of digest
 * @return charmb_status_t Status code
 */
charmb_status_t charmb_hash_8b(const uint8_t* data, uint8_t* digest, 
                              charmb_digest_size_t digest_size);

/**
 * @brief Ultra-fast 16-byte specialized hash
 * 
 * Hyper-optimized for exactly 16 bytes (common for UUIDs, small keys, etc.)
 * 
 * @param data Input data (exactly 16 bytes)
 * @param digest Output buffer (16 or 32 bytes)
 * @param digest_size Size of digest
 * @return charmb_status_t Status code
 */
charmb_status_t charmb_hash_16b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size);

/**
 * @brief Ultra-fast 32-byte specialized hash
 * 
 * Hyper-optimized for exactly 32 bytes (common for hash outputs, keys, etc.)
 * 
 * @param data Input data (exactly 32 bytes)
 * @param digest Output buffer (16 or 32 bytes)
 * @param digest_size Size of digest
 * @return charmb_status_t Status code
 */
charmb_status_t charmb_hash_32b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size);

/**
 * @brief Ultra-fast 64-byte specialized hash
 * 
 * Hyper-optimized for exactly 64 bytes (common for crypto blocks, headers, etc.)
 * 
 * @param data Input data (exactly 64 bytes)
 * @param digest Output buffer (16 or 32 bytes)
 * @param digest_size Size of digest
 * @return charmb_status_t Status code
 */
charmb_status_t charmb_hash_64b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size);

/**
 * @brief Bit-level CHARM-B for arbitrary small inputs
 * 
 * Uses bit-level processing for maximum efficiency on inputs that
 * don't fit exact size specializations.
 * 
 * @param data Input data (must be ≤ 64 bytes)
 * @param data_size Size of input data in bytes
 * @param digest Output buffer for digest
 * @param digest_size Size of digest (16 or 32 bytes)
 * @return charmb_status_t Status code
 */
charmb_status_t charmb_hash_bitlevel(const uint8_t* data, size_t data_size,
                                    uint8_t* digest, charmb_digest_size_t digest_size);

/**
 * @brief Get CHARM-B version and features
 * 
 * @return const char* Version string with feature flags
 */
const char* charmb_get_version(void);

/**
 * @brief Check if current CPU supports CHARM-B optimizations
 * 
 * @return bool True if optimizations are available
 */
bool charmb_cpu_support(void);

/**
 * @brief Set test mode for deterministic behavior
 * 
 * When enabled, CHARM-B will use deterministic entropy for consistent
 * results across multiple calls. Only use for testing purposes.
 * 
 * @param enable True to enable test mode, false to disable
 */
void charmb_set_test_mode(bool enable);

#ifdef __cplusplus
}
#endif

#endif /* CHARMB_CORE_H */