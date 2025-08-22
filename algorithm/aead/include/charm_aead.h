/**
 * @file charm_aead.h
 * @brief CHARM AEAD (Authenticated Encryption with Associated Data)
 * 
 * AEAD implementation using the main CHARM algorithm with both regular 
 * nonce-based mode and misuse-resistant SIV (Synthetic IV) variant.
 */

#ifndef CHARM_AEAD_H
#define CHARM_AEAD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM AEAD Constants
 */
#define CHARM_AEAD_KEY_SIZE     32    // 256-bit key
#define CHARM_AEAD_NONCE_SIZE   16    // 128-bit nonce (larger than CHARM-B for security)
#define CHARM_AEAD_TAG_SIZE     32    // 256-bit authentication tag (larger for security)

/**
 * @brief CHARM AEAD Status Codes
 */
typedef enum {
    CHARM_AEAD_SUCCESS = 0,
    CHARM_AEAD_ERROR_NULL_POINTER = -1,
    CHARM_AEAD_ERROR_INVALID_SIZE = -2,
    CHARM_AEAD_ERROR_AUTH_FAILED = -3,
    CHARM_AEAD_ERROR_BUFFER_TOO_SMALL = -4
} charm_aead_status_t;

/**
 * @brief CHARM AEAD Encryption
 * 
 * Regular nonce-based AEAD mode. Nonce must be unique for each encryption
 * with the same key to ensure security.
 * 
 * @param key Encryption key (32 bytes)
 * @param nonce Nonce (16 bytes, must be unique for each encryption with same key)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag (32 bytes)
 * @return Status code
 */
charm_aead_status_t charm_aead_encrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAD_TAG_SIZE]
);

/**
 * @brief CHARM AEAD Decryption
 * 
 * @param key Decryption key (32 bytes)
 * @param nonce Nonce (16 bytes)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag (32 bytes)
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code (CHARM_AEAD_ERROR_AUTH_FAILED if authentication fails)
 */
charm_aead_status_t charm_aead_decrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAD_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief CHARM AEAD SIV (Synthetic IV) Encryption - Misuse-Resistant
 * 
 * SIV mode provides nonce-misuse resistance by deriving a synthetic IV
 * from the key, AAD, and plaintext using CHARM's entropy-native properties.
 * This makes encryption deterministic and safe even with repeated or
 * predictable nonces.
 * 
 * Key advantages over regular mode:
 * - No nonce required (internally generated from inputs)
 * - Deterministic: same inputs always produce same outputs
 * - Misuse-resistant: safe even with repeated nonces
 * - Uses CHARM-512 for enhanced security margin
 * 
 * @param key Encryption key (32 bytes)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag (32 bytes, contains SIV)
 * @return Status code
 */
charm_aead_status_t charm_aead_siv_encrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAD_TAG_SIZE]
);

/**
 * @brief CHARM AEAD SIV (Synthetic IV) Decryption - Misuse-Resistant
 * 
 * @param key Decryption key (32 bytes)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag (32 bytes, contains SIV)
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code (CHARM_AEAD_ERROR_AUTH_FAILED if authentication fails)
 */
charm_aead_status_t charm_aead_siv_decrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAD_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief CHARM HMAC for authentication
 * 
 * @param key HMAC key
 * @param key_len Key length
 * @param data Data to authenticate
 * @param data_len Data length
 * @param hmac Output HMAC (32 bytes)
 * @return Status code
 */
charm_aead_status_t charm_hmac(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[32]
);

/**
 * @brief CHARM Key Derivation Function
 * 
 * @param key Input key material
 * @param key_len Input key length
 * @param salt Optional salt (can be NULL)
 * @param salt_len Salt length
 * @param info Context information (can be NULL)
 * @param info_len Info length
 * @param output Output key material
 * @param output_len Desired output length
 * @return Status code
 */
charm_aead_status_t charm_kdf(
    const uint8_t* key, size_t key_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* output, size_t output_len
);

/**
 * @brief Benchmark CHARM AEAD performance
 * 
 * @param mode_siv True for SIV mode, false for regular mode
 * @param payload_size Size of payload to test
 * @param iterations Number of iterations
 * @param encrypt_mbps Output encryption throughput
 * @param decrypt_mbps Output decryption throughput
 * @return Status code
 */
/**
 * @brief Benchmark CHARM AEAD performance
 * 
 * @param mode_siv True for SIV mode, false for regular mode
 * @param payload_size Size of payload to test
 * @param iterations Number of iterations
 * @param encrypt_mbps Output encryption throughput
 * @param decrypt_mbps Output decryption throughput
 * @return Status code
 */
charm_aead_status_t charm_aead_benchmark(
    bool mode_siv, size_t payload_size, int iterations,
    double* encrypt_mbps, double* decrypt_mbps
);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_AEAD_H */