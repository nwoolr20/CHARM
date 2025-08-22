/**
 * @file charmb.h
 * @brief CHARM-B (CHARMbit) Algorithm API - Ultra-Small Input Optimized Hash
 * 
 * CHARM-B (CHARMbit) - Specialized variant optimized for ≤64B inputs
 * Revolutionary Performance on Ultra-Small Inputs
 * 
 * Key Features:
 * - Bit-level optimization for maximum efficiency
 * - Specialized algorithms for exact sizes (8B, 16B, 32B, 64B)
 * - SIMD acceleration using AVX2 instructions
 * - Zero overhead design with thread-local static state
 * - Optional 128-bit digests for micro-hashing applications
 * 
 * Version: CHARM-B/1.0
 * Mode: Bit-optimized ultra-fast hash for micro-payloads
 * Security: Entropy-native cryptographic hash with quantum resistance
 */

#ifndef CHARMB_H
#define CHARMB_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM-B Algorithm Version
 */
#define CHARMB_VERSION_MAJOR 1
#define CHARMB_VERSION_MINOR 0
#define CHARMB_VERSION_STRING "CHARM-B/1.0"

/**
 * @brief CHARM-B Constants
 */
#define CHARMB_MAX_INPUT_SIZE 4096  // Extended input size for AEAD support
#define CHARMB_DIGEST_256_SIZE 32   // 256-bit digest size
#define CHARMB_DIGEST_128_SIZE 16   // 128-bit digest size for micro-hashing

/**
 * @brief CHARM-B Status Codes
 */
typedef enum {
    CHARMB_SUCCESS = 0,
    CHARMB_ERROR_NULL_POINTER = -1,
    CHARMB_ERROR_INVALID_SIZE = -2,
    CHARMB_ERROR_INVALID_DIGEST_SIZE = -3,
    CHARMB_ERROR_UNSUPPORTED_SIZE = -4
} charmb_status_t;

/**
 * @brief CHARM-B Digest Size Options
 */
typedef enum {
    CHARMB_DIGEST_128 = 128,
    CHARMB_DIGEST_256 = 256
} charmb_digest_size_t;

/**
 * @brief Main CHARM-B hash function with automatic size optimization
 * 
 * @param data Input data (up to 64 bytes)
 * @param size Input size in bytes (1-64)
 * @param digest Output digest buffer
 * @param digest_size Digest size (128 or 256 bits)
 * @return Status code
 */
charmb_status_t charmb_hash(const uint8_t* data, size_t size, 
                            uint8_t* digest, charmb_digest_size_t digest_size);

/**
 * @brief Ultra-optimized 8-byte hash
 * 
 * @param data Input data (exactly 8 bytes)
 * @param digest Output digest buffer
 * @param digest_size Digest size (128 or 256 bits)
 * @return Status code
 */
charmb_status_t charmb_hash_8b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size);

/**
 * @brief Ultra-optimized 16-byte hash
 * 
 * @param data Input data (exactly 16 bytes)
 * @param digest Output digest buffer
 * @param digest_size Digest size (128 or 256 bits)
 * @return Status code
 */
charmb_status_t charmb_hash_16b(const uint8_t* data, uint8_t* digest, 
                                charmb_digest_size_t digest_size);

/**
 * @brief Ultra-optimized 32-byte hash
 * 
 * @param data Input data (exactly 32 bytes)
 * @param digest Output digest buffer
 * @param digest_size Digest size (128 or 256 bits)
 * @return Status code
 */
charmb_status_t charmb_hash_32b(const uint8_t* data, uint8_t* digest, 
                                charmb_digest_size_t digest_size);

/**
 * @brief Ultra-optimized 64-byte hash
 * 
 * @param data Input data (exactly 64 bytes)
 * @param digest Output digest buffer
 * @param digest_size Digest size (128 or 256 bits)
 * @return Status code
 */
charmb_status_t charmb_hash_64b(const uint8_t* data, uint8_t* digest, 
                                charmb_digest_size_t digest_size);

/**
 * @brief Check if SIMD acceleration is available
 * 
 * @return true if AVX2 is available and enabled
 */
bool charmb_simd_available(void);

/**
 * @brief Get CHARM-B algorithm information
 * 
 * @param version_string Output buffer for version string (minimum 32 bytes)
 * @param features Output buffer for features string (minimum 128 bytes)
 * @return Status code
 */
charmb_status_t charmb_get_info(char* version_string, char* features);

/**
 * @brief CHARM-B performance benchmark function
 * 
 * @param size Input size to benchmark
 * @param iterations Number of iterations
 * @param throughput_mbps Output throughput in MB/s
 * @return Status code
 */
charmb_status_t charmb_benchmark(size_t size, int iterations, double* throughput_mbps);

#ifdef __cplusplus
}
#endif

#endif /* CHARMB_H */