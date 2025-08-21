/**
 * @file charm_crypto_suite.h
 * @brief Unified API for CHARM Cryptographic Suite
 * 
 * This header provides a unified interface to all CHARM cryptographic primitives,
 * enabling easy algorithm selection and consistent error handling.
 */

#ifndef CHARM_CRYPTO_SUITE_H
#define CHARM_CRYPTO_SUITE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Unified status codes for all CHARM crypto operations
 */
typedef enum {
    CHARM_CRYPTO_SUCCESS = 0,
    CHARM_CRYPTO_ERROR_NULL_POINTER = -1,
    CHARM_CRYPTO_ERROR_INVALID_ALGORITHM = -2,
    CHARM_CRYPTO_ERROR_INVALID_SIZE = -3,
    CHARM_CRYPTO_ERROR_BUFFER_TOO_SMALL = -4,
    CHARM_CRYPTO_ERROR_AUTH_FAILED = -5,
    CHARM_CRYPTO_ERROR_NOT_IMPLEMENTED = -6,
    CHARM_CRYPTO_ERROR_INTERNAL = -7
} charm_crypto_status_t;

/**
 * @brief Available hash algorithms
 */
typedef enum {
    CHARM_HASH_256 = 256,
    CHARM_HASH_384 = 384,
    CHARM_HASH_512 = 512,
    CHARM_HASH_B_128 = 128,    // CHARM-B optimized for small inputs
    CHARM_HASH_B_256 = 1256    // CHARM-B 256-bit
} charm_hash_algorithm_t;

/**
 * @brief Available AEAD algorithms
 */
typedef enum {
    CHARM_AEAD_CHARMB,         // CHARM-B native AEAD (optimized for ≤64 bytes)
    CHARM_AEAD_AES_GCM,        // AES-256-GCM wrapper
    CHARM_AEAD_CHACHA20_POLY1305 // ChaCha20-Poly1305 wrapper
} charm_aead_algorithm_t;

/**
 * @brief Available MAC algorithms
 */
typedef enum {
    CHARM_MAC_HMAC_CHARM,      // HMAC using CHARM hash
    CHARM_MAC_NATIVE,          // CHARM-native MAC construction
    CHARM_MAC_HMAC_SHA256      // HMAC-SHA256 for interop
} charm_mac_algorithm_t;

/**
 * @brief Available KDF algorithms
 */
typedef enum {
    CHARM_KDF_HKDF_CHARM,      // HKDF using CHARM hash
    CHARM_KDF_NATIVE,          // CHARM-native key expansion
    CHARM_KDF_PBKDF2           // PBKDF2 for password-based derivation
} charm_kdf_algorithm_t;

// =============================================================================
// HASH FUNCTIONS
// =============================================================================

/**
 * @brief Unified hash function interface
 * 
 * @param algorithm Hash algorithm to use
 * @param data Input data
 * @param data_len Length of input data
 * @param digest Output digest buffer
 * @param digest_len Size of digest buffer (must match algorithm output size)
 * @return Status code
 */
charm_crypto_status_t charm_hash(
    charm_hash_algorithm_t algorithm,
    const uint8_t* data, size_t data_len,
    uint8_t* digest, size_t digest_len
);

/**
 * @brief Get digest size for hash algorithm
 * 
 * @param algorithm Hash algorithm
 * @return Digest size in bytes, or 0 if algorithm invalid
 */
size_t charm_hash_digest_size(charm_hash_algorithm_t algorithm);

// =============================================================================
// AUTHENTICATED ENCRYPTION (AEAD)
// =============================================================================

/**
 * @brief Unified AEAD encryption interface
 * 
 * @param algorithm AEAD algorithm to use
 * @param key Encryption key
 * @param key_len Key length
 * @param nonce Nonce/IV
 * @param nonce_len Nonce length
 * @param aad Additional authenticated data
 * @param aad_len AAD length
 * @param plaintext Input plaintext
 * @param plaintext_len Plaintext length
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag
 * @param tag_len Tag buffer length
 * @return Status code
 */
charm_crypto_status_t charm_aead_encrypt(
    charm_aead_algorithm_t algorithm,
    const uint8_t* key, size_t key_len,
    const uint8_t* nonce, size_t nonce_len,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t* tag, size_t tag_len
);

/**
 * @brief Unified AEAD decryption interface
 * 
 * @param algorithm AEAD algorithm to use
 * @param key Decryption key
 * @param key_len Key length
 * @param nonce Nonce/IV
 * @param nonce_len Nonce length
 * @param aad Additional authenticated data
 * @param aad_len AAD length
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Ciphertext length
 * @param tag Authentication tag
 * @param tag_len Tag length
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code (CHARM_CRYPTO_ERROR_AUTH_FAILED if authentication fails)
 */
charm_crypto_status_t charm_aead_decrypt(
    charm_aead_algorithm_t algorithm,
    const uint8_t* key, size_t key_len,
    const uint8_t* nonce, size_t nonce_len,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t* tag, size_t tag_len,
    uint8_t* plaintext
);

/**
 * @brief Get key, nonce, and tag sizes for AEAD algorithm
 */
charm_crypto_status_t charm_aead_get_sizes(
    charm_aead_algorithm_t algorithm,
    size_t* key_size, size_t* nonce_size, size_t* tag_size
);

// =============================================================================
// MESSAGE AUTHENTICATION CODES (MAC)
// =============================================================================

/**
 * @brief Unified MAC computation interface
 * 
 * @param algorithm MAC algorithm to use
 * @param key MAC key
 * @param key_len Key length
 * @param data Message data
 * @param data_len Message length
 * @param mac Output MAC
 * @param mac_len MAC buffer length
 * @return Status code
 */
charm_crypto_status_t charm_mac(
    charm_mac_algorithm_t algorithm,
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t* mac, size_t mac_len
);

/**
 * @brief Constant-time MAC verification
 * 
 * @param algorithm MAC algorithm used
 * @param key MAC key
 * @param key_len Key length
 * @param data Message data
 * @param data_len Message length
 * @param mac MAC to verify
 * @param mac_len MAC length
 * @return CHARM_CRYPTO_SUCCESS if MAC is valid, CHARM_CRYPTO_ERROR_AUTH_FAILED if invalid
 */
charm_crypto_status_t charm_mac_verify(
    charm_mac_algorithm_t algorithm,
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    const uint8_t* mac, size_t mac_len
);

// =============================================================================
// KEY DERIVATION FUNCTIONS (KDF)
// =============================================================================

/**
 * @brief Unified key derivation interface
 * 
 * @param algorithm KDF algorithm to use
 * @param input_key Input key material
 * @param input_key_len Input key length
 * @param salt Optional salt
 * @param salt_len Salt length
 * @param info Optional context information
 * @param info_len Info length
 * @param output_key Output key material
 * @param output_key_len Desired output key length
 * @return Status code
 */
charm_crypto_status_t charm_kdf(
    charm_kdf_algorithm_t algorithm,
    const uint8_t* input_key, size_t input_key_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* output_key, size_t output_key_len
);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Get human-readable algorithm name
 */
const char* charm_crypto_algorithm_name(int algorithm_type, int algorithm);

/**
 * @brief Get library version information
 */
const char* charm_crypto_version(void);

/**
 * @brief Check if algorithm is available in this build
 */
int charm_crypto_algorithm_available(int algorithm_type, int algorithm);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_CRYPTO_SUITE_H */