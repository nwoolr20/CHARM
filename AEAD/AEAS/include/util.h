/**
 * @file util.h
 * @brief Utility functions for AEAS implementation
 * 
 * Provides constant-time utilities, CPU feature detection,
 * and nonce management helpers.
 */

#ifndef AEAS_UTIL_H
#define AEAS_UTIL_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Constant-time memory comparison
 * 
 * Compares two memory regions in constant time to prevent timing attacks.
 * 
 * @param a First memory region
 * @param b Second memory region
 * @param len Number of bytes to compare
 * @return 0 if equal, non-zero if different
 */
int util_ct_memcmp(const void* a, const void* b, size_t len);

/**
 * @brief Secure memory clear
 * 
 * Clears memory in a way that prevents compiler optimization.
 * 
 * @param ptr Memory to clear
 * @param len Number of bytes to clear
 */
void util_secure_clear(void* ptr, size_t len);

/**
 * @brief Little-endian 32-bit integer encoding
 * 
 * @param value 32-bit value to encode
 * @param out Output buffer (4 bytes)
 */
void util_le32_encode(uint32_t value, uint8_t out[4]);

/**
 * @brief Little-endian 64-bit integer encoding
 * 
 * @param value 64-bit value to encode
 * @param out Output buffer (8 bytes)
 */
void util_le64_encode(uint64_t value, uint8_t out[8]);

/**
 * @brief Little-endian 32-bit integer decoding
 * 
 * @param in Input buffer (4 bytes)
 * @return Decoded 32-bit value
 */
uint32_t util_le32_decode(const uint8_t in[4]);

/**
 * @brief Little-endian 64-bit integer decoding
 * 
 * @param in Input buffer (8 bytes)
 * @return Decoded 64-bit value
 */
uint64_t util_le64_decode(const uint8_t in[8]);

/**
 * @brief CPU feature flags
 */
#define UTIL_CPU_AVX2    0x01  // AVX2 support
#define UTIL_CPU_AES_NI  0x02  // AES-NI support

/**
 * @brief Detect CPU features
 * 
 * @return Bitmask of supported CPU features
 */
uint32_t util_cpu_features(void);

/**
 * @brief Check if AVX2 is supported
 * 
 * @return 1 if AVX2 is supported, 0 otherwise
 */
int util_has_avx2(void);

/**
 * @brief Validate nonce parameters
 * 
 * @param nonce Nonce to validate (must be 12 bytes)
 * @return 0 if valid, negative on error
 */
int util_validate_nonce(const uint8_t nonce[12]);

/**
 * @brief Convert bytes to hex string (for debugging)
 * 
 * @param data Input data
 * @param data_len Length of input data
 * @param hex_out Output hex string (must be at least 2*data_len+1 bytes)
 */
void util_bytes_to_hex(const uint8_t* data, size_t data_len, char* hex_out);

#ifdef __cplusplus
}
#endif

#endif /* AEAS_UTIL_H */