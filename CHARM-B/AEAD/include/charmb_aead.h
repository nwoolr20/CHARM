/**
 * @file charmb_aead.h
 * @brief CHARM-B AEAD (Authenticated Encryption with Associated Data)
 * 
 * Ultra-fast AEAD implementation optimized for small payloads using CHARM-B
 */

#ifndef CHARMB_AEAD_H
#define CHARMB_AEAD_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM-B AEAD Constants
 */
#define CHARMB_AEAD_KEY_SIZE    32    // 256-bit key
#define CHARMB_AEAD_NONCE_SIZE  12    // 96-bit nonce
#define CHARMB_AEAD_TAG_SIZE    16    // 128-bit authentication tag

/**
 * @brief CHARM-B AEAD Status Codes
 */
typedef enum {
    CHARMB_AEAD_SUCCESS = 0,
    CHARMB_AEAD_ERROR_NULL_POINTER = -1,
    CHARMB_AEAD_ERROR_INVALID_SIZE = -2,
    CHARMB_AEAD_ERROR_AUTH_FAILED = -3,
    CHARMB_AEAD_ERROR_BUFFER_TOO_SMALL = -4
} charmb_aead_status_t;

/**
 * @brief CHARM-B AEAD Encryption
 * 
 * @param key Encryption key (32 bytes)
 * @param nonce Nonce (12 bytes, must be unique for each encryption with same key)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag (16 bytes)
 * @return Status code
 */
charmb_aead_status_t charmb_aead_encrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARMB_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARMB_AEAD_TAG_SIZE]
);

/**
 * @brief CHARM-B AEAD Decryption
 * 
 * @param key Decryption key (32 bytes)
 * @param nonce Nonce (12 bytes)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag (16 bytes)
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code (CHARMB_AEAD_ERROR_AUTH_FAILED if authentication fails)
 */
charmb_aead_status_t charmb_aead_decrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARMB_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARMB_AEAD_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief CHARM-B HMAC for authentication
 * 
 * @param key HMAC key
 * @param key_len Key length
 * @param data Data to authenticate
 * @param data_len Data length
 * @param hmac Output HMAC (32 bytes)
 * @return Status code
 */
charmb_aead_status_t charmb_hmac(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[32]
);

/**
 * @brief CHARM-B Key Derivation Function
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
charmb_aead_status_t charmb_kdf(
    const uint8_t* key, size_t key_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* output, size_t output_len
);

/**
 * @brief CHARM-B AEAD SIV (Synthetic IV) Encryption - Misuse-Resistant
 * 
 * SIV mode provides nonce-misuse resistance by deriving a synthetic IV
 * from the key, AAD, and plaintext. This makes encryption deterministic
 * and safe even with repeated or predictable nonces.
 * 
 * @param key Encryption key (32 bytes)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag (16 bytes, contains SIV)
 * @return Status code
 */
charmb_aead_status_t charmb_aead_siv_encrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARMB_AEAD_TAG_SIZE]
);

/**
 * @brief CHARM-B AEAD SIV (Synthetic IV) Decryption - Misuse-Resistant
 * 
 * @param key Decryption key (32 bytes)
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag (16 bytes, contains SIV)
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code (CHARMB_AEAD_ERROR_AUTH_FAILED if authentication fails)
 */
charmb_aead_status_t charmb_aead_siv_decrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARMB_AEAD_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief Benchmark CHARM-B AEAD performance
 * 
 * @param payload_size Size of payload to test
 * @param iterations Number of iterations
 * @param encrypt_mbps Output encryption throughput
 * @param decrypt_mbps Output decryption throughput
 * @return Status code
 */
charmb_aead_status_t charmb_aead_benchmark(
    size_t payload_size, int iterations,
    double* encrypt_mbps, double* decrypt_mbps
);

#ifdef __cplusplus
}
#endif

#endif /* CHARMB_AEAD_H */