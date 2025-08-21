/**
 * @file openssl_helpers.h
 * @brief Thin wrappers around OpenSSL EVP for benchmarking
 */

#ifndef OPENSSL_HELPERS_H
#define OPENSSL_HELPERS_H

#include <stdint.h>
#include <stddef.h>

#ifdef WITH_OPENSSL

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief AES-256-GCM encryption
 */
int openssl_aes_gcm_encrypt(const uint8_t key[32],
                            const uint8_t nonce[12],
                            const uint8_t* aad, size_t aad_len,
                            const uint8_t* plaintext, size_t pt_len,
                            uint8_t* ciphertext,
                            uint8_t tag[16]);

/**
 * @brief AES-256-GCM decryption
 */
int openssl_aes_gcm_decrypt(const uint8_t key[32],
                            const uint8_t nonce[12],
                            const uint8_t* aad, size_t aad_len,
                            const uint8_t* ciphertext, size_t ct_len,
                            const uint8_t tag[16],
                            uint8_t* plaintext);

/**
 * @brief ChaCha20-Poly1305 encryption
 */
int openssl_chacha20_poly1305_encrypt(const uint8_t key[32],
                                       const uint8_t nonce[12],
                                       const uint8_t* aad, size_t aad_len,
                                       const uint8_t* plaintext, size_t pt_len,
                                       uint8_t* ciphertext,
                                       uint8_t tag[16]);

/**
 * @brief ChaCha20-Poly1305 decryption
 */
int openssl_chacha20_poly1305_decrypt(const uint8_t key[32],
                                       const uint8_t nonce[12],
                                       const uint8_t* aad, size_t aad_len,
                                       const uint8_t* ciphertext, size_t ct_len,
                                       const uint8_t tag[16],
                                       uint8_t* plaintext);

#ifdef __cplusplus
}
#endif

#endif /* WITH_OPENSSL */

#endif /* OPENSSL_HELPERS_H */