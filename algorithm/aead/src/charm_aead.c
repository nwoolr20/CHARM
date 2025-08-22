/**
 * @file charm_aead.c
 * @brief CHARM AEAD Implementation
 * 
 * AEAD implementation using CHARM algorithm with both regular nonce-based
 * mode and misuse-resistant SIV variant.
 */

#include "charm_aead.h"
#include "../include/charm.h"
#include "../../CHARM-B/include/charmb.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

// Internal utility functions
static void secure_clear(void* ptr, size_t len) {
    volatile uint8_t* volatile_ptr = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < len; i++) {
        volatile_ptr[i] = 0;
    }
}

static void xor_buffers(const uint8_t* a, const uint8_t* b, uint8_t* out, size_t len) {
    for (size_t i = 0; i < len; i++) {
        out[i] = a[i] ^ b[i];
    }
}

/**
 * @brief Fast keystream generation using CHARM-256 but optimized
 */
static charm_aead_status_t charm_generate_keystream_fast(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    uint32_t counter,
    uint8_t* keystream,
    size_t keystream_len
) {
    if (!key || !nonce || !keystream) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    // Build input: key(32) || nonce(16) || counter(4) = 52 bytes
    uint8_t input[52];
    memcpy(input, key, 32);
    memcpy(input + 32, nonce, 16);
    
    size_t generated = 0;
    uint32_t block_counter = counter;
    
    while (generated < keystream_len) {
        // Encode counter as little-endian
        input[48] = (uint8_t)(block_counter & 0xFF);
        input[49] = (uint8_t)((block_counter >> 8) & 0xFF);
        input[50] = (uint8_t)((block_counter >> 16) & 0xFF);
        input[51] = (uint8_t)((block_counter >> 24) & 0xFF);
        
        // Use CHARM-512 for 64-byte blocks (faster than multiple CHARM-256 calls)
        uint8_t block[64];
        int status = charm_hash(CHARM_512, input, 52, block);
        if (status != 0) return CHARM_AEAD_ERROR_NULL_POINTER;
        
        size_t copy_len = (keystream_len - generated < 64) ? (keystream_len - generated) : 64;
        memcpy(keystream + generated, block, copy_len);
        generated += copy_len;
        block_counter++;
        
        secure_clear(block, sizeof(block));
    }
    
    secure_clear(input, sizeof(input));
    return CHARM_AEAD_SUCCESS;
}

/**
 * @brief CHARM HMAC implementation using standard CHARM for compatibility
 */
charm_aead_status_t charm_hmac(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[32]
) {
    if (!key || !data || !hmac) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    // Simple but compatible HMAC using CHARM-256: CHARM(key || data)
    uint8_t padded_key[32];
    memset(padded_key, 0, 32);
    
    if (key_len <= 32) {
        memcpy(padded_key, key, key_len);
    } else {
        // Hash long keys
        int status = charm_hash(CHARM_256, key, key_len, padded_key);
        if (status != 0) return CHARM_AEAD_ERROR_NULL_POINTER;
    }
    
    // Build input: key || data
    size_t input_len = 32 + data_len;
    uint8_t* input = malloc(input_len);
    if (!input) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    memcpy(input, padded_key, 32);
    memcpy(input + 32, data, data_len);
    
    // Use CHARM hash as HMAC
    int status = charm_hash(CHARM_256, input, input_len, hmac);
    secure_clear(input, input_len);
    free(input);
    secure_clear(padded_key, sizeof(padded_key));
    
    return (status == 0) ? CHARM_AEAD_SUCCESS : CHARM_AEAD_ERROR_NULL_POINTER;
}

/**
 * @brief CHARM Key Derivation Function
 */
charm_aead_status_t charm_kdf(
    const uint8_t* key, size_t key_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* output, size_t output_len
) {
    if (!key || !output) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    // HKDF-like expansion using CHARM
    // For each output block: CHARM(key || salt || info || counter)
    
    size_t generated = 0;
    uint8_t counter = 1;
    
    while (generated < output_len) {
        // Build input: key || salt || info || counter
        size_t input_len = key_len + salt_len + info_len + 1;
        uint8_t* input = malloc(input_len);
        if (!input) return CHARM_AEAD_ERROR_NULL_POINTER;
        
        size_t offset = 0;
        memcpy(input + offset, key, key_len);
        offset += key_len;
        
        if (salt && salt_len > 0) {
            memcpy(input + offset, salt, salt_len);
            offset += salt_len;
        }
        
        if (info && info_len > 0) {
            memcpy(input + offset, info, info_len);
            offset += info_len;
        }
        
        input[offset] = counter;
        
        uint8_t block[32];
        int status = charm_hash(CHARM_256, input, input_len, block);
        secure_clear(input, input_len);
        free(input);
        
        if (status != 0) return CHARM_AEAD_ERROR_NULL_POINTER;
        
        size_t copy_len = (output_len - generated < 32) ? (output_len - generated) : 32;
        memcpy(output + generated, block, copy_len);
        generated += copy_len;
        counter++;
        
        secure_clear(block, sizeof(block));
    }
    
    return CHARM_AEAD_SUCCESS;
}

/**
 * @brief Regular CHARM AEAD Encryption
 */
charm_aead_status_t charm_aead_encrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAD_TAG_SIZE]
) {
    if (!key || !nonce || !plaintext || !ciphertext || !tag) {
        return CHARM_AEAD_ERROR_NULL_POINTER;
    }
    
    // Generate keystream for encryption
    uint8_t* keystream = malloc(plaintext_len);
    if (!keystream) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    charm_aead_status_t status = charm_generate_keystream_fast(
        key, nonce, 0, keystream, plaintext_len
    );
    if (status != CHARM_AEAD_SUCCESS) {
        free(keystream);
        return status;
    }
    
    // Encrypt: ciphertext = plaintext XOR keystream
    xor_buffers(plaintext, keystream, ciphertext, plaintext_len);
    secure_clear(keystream, plaintext_len);
    free(keystream);
    
    // Generate authentication tag
    // tag = HMAC(key, nonce || aad || ciphertext)
    size_t auth_input_len = CHARM_AEAD_NONCE_SIZE + aad_len + plaintext_len;
    uint8_t* auth_input = malloc(auth_input_len);
    if (!auth_input) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    size_t offset = 0;
    memcpy(auth_input + offset, nonce, CHARM_AEAD_NONCE_SIZE);
    offset += CHARM_AEAD_NONCE_SIZE;
    
    if (aad && aad_len > 0) {
        memcpy(auth_input + offset, aad, aad_len);
        offset += aad_len;
    }
    
    memcpy(auth_input + offset, ciphertext, plaintext_len);
    
    status = charm_hmac(key, CHARM_AEAD_KEY_SIZE, auth_input, auth_input_len, tag);
    secure_clear(auth_input, auth_input_len);
    free(auth_input);
    
    return status;
}

/**
 * @brief Regular CHARM AEAD Decryption
 */
charm_aead_status_t charm_aead_decrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARM_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAD_TAG_SIZE],
    uint8_t* plaintext
) {
    if (!key || !nonce || !ciphertext || !tag || !plaintext) {
        return CHARM_AEAD_ERROR_NULL_POINTER;
    }
    
    // Verify authentication tag first
    size_t auth_input_len = CHARM_AEAD_NONCE_SIZE + aad_len + ciphertext_len;
    uint8_t* auth_input = malloc(auth_input_len);
    if (!auth_input) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    size_t offset = 0;
    memcpy(auth_input + offset, nonce, CHARM_AEAD_NONCE_SIZE);
    offset += CHARM_AEAD_NONCE_SIZE;
    
    if (aad && aad_len > 0) {
        memcpy(auth_input + offset, aad, aad_len);
        offset += aad_len;
    }
    
    memcpy(auth_input + offset, ciphertext, ciphertext_len);
    
    uint8_t computed_tag[CHARM_AEAD_TAG_SIZE];
    charm_aead_status_t status = charm_hmac(
        key, CHARM_AEAD_KEY_SIZE, auth_input, auth_input_len, computed_tag
    );
    secure_clear(auth_input, auth_input_len);
    free(auth_input);
    
    if (status != CHARM_AEAD_SUCCESS) return status;
    
    // Constant-time tag comparison
    uint8_t diff = 0;
    for (int i = 0; i < CHARM_AEAD_TAG_SIZE; i++) {
        diff |= tag[i] ^ computed_tag[i];
    }
    secure_clear(computed_tag, sizeof(computed_tag));
    
    if (diff != 0) {
        return CHARM_AEAD_ERROR_AUTH_FAILED;
    }
    
    // Generate keystream for decryption
    uint8_t* keystream = malloc(ciphertext_len);
    if (!keystream) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    status = charm_generate_keystream_fast(key, nonce, 0, keystream, ciphertext_len);
    if (status != CHARM_AEAD_SUCCESS) {
        free(keystream);
        return status;
    }
    
    // Decrypt: plaintext = ciphertext XOR keystream
    xor_buffers(ciphertext, keystream, plaintext, ciphertext_len);
    secure_clear(keystream, ciphertext_len);
    free(keystream);
    
    return CHARM_AEAD_SUCCESS;
}

/**
 * @brief Generate Synthetic IV for SIV mode
 */
charm_aead_status_t charm_generate_siv(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t siv[16]
) {
    // SIV = first 16 bytes of CHARM-512(key || "SIV" || aad_len || aad || plaintext_len || plaintext)
    size_t input_len = 32 + 3 + 8 + aad_len + 8 + plaintext_len;
    uint8_t* input = malloc(input_len);
    if (!input) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    size_t offset = 0;
    memcpy(input + offset, key, 32);
    offset += 32;
    
    memcpy(input + offset, "SIV", 3);
    offset += 3;
    
    // Encode AAD length (little-endian 64-bit)
    for (int i = 0; i < 8; i++) {
        input[offset + i] = (uint8_t)((aad_len >> (i * 8)) & 0xFF);
    }
    offset += 8;
    
    if (aad && aad_len > 0) {
        memcpy(input + offset, aad, aad_len);
        offset += aad_len;
    }
    
    // Encode plaintext length (little-endian 64-bit)
    for (int i = 0; i < 8; i++) {
        input[offset + i] = (uint8_t)((plaintext_len >> (i * 8)) & 0xFF);
    }
    offset += 8;
    
    if (plaintext && plaintext_len > 0) {
        memcpy(input + offset, plaintext, plaintext_len);
    }
    
    uint8_t hash_output[64];
    int status = charm_hash(CHARM_512, input, input_len, hash_output);
    secure_clear(input, input_len);
    free(input);
    
    if (status != 0) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    // Use last 16 bytes as SIV (bytes 48-63) for better diffusion
    memcpy(siv, hash_output + 48, 16);
    secure_clear(hash_output, sizeof(hash_output));
    
    return CHARM_AEAD_SUCCESS;
}

/**
 * @brief SIV CHARM AEAD Encryption
 */
charm_aead_status_t charm_aead_siv_encrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_AEAD_TAG_SIZE]
) {
    if (!key || !plaintext || !ciphertext || !tag) {
        return CHARM_AEAD_ERROR_NULL_POINTER;
    }
    
    // Generate Synthetic IV
    uint8_t siv[16];
    charm_aead_status_t status = charm_generate_siv(
        key, aad, aad_len, plaintext, plaintext_len, siv
    );
    if (status != CHARM_AEAD_SUCCESS) return status;
    
    // Use SIV as nonce for encryption (pad to 16 bytes)
    uint8_t synthetic_nonce[16];
    memcpy(synthetic_nonce, siv, 16);
    
    // Generate keystream using synthetic nonce
    uint8_t* keystream = malloc(plaintext_len);
    if (!keystream) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    status = charm_generate_keystream_fast(key, synthetic_nonce, 0, keystream, plaintext_len);
    if (status != CHARM_AEAD_SUCCESS) {
        free(keystream);
        return status;
    }
    
    // Encrypt: ciphertext = plaintext XOR keystream
    xor_buffers(plaintext, keystream, ciphertext, plaintext_len);
    secure_clear(keystream, plaintext_len);
    free(keystream);
    
    // Generate authentication tag with SIV embedded
    // tag = HMAC(key, "SIV" || siv || aad || ciphertext)
    size_t auth_input_len = 3 + 16 + aad_len + plaintext_len;
    uint8_t* auth_input = malloc(auth_input_len);
    if (!auth_input) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    size_t offset = 0;
    memcpy(auth_input + offset, "SIV", 3);
    offset += 3;
    
    memcpy(auth_input + offset, siv, 16);
    offset += 16;
    
    if (aad && aad_len > 0) {
        memcpy(auth_input + offset, aad, aad_len);
        offset += aad_len;
    }
    
    memcpy(auth_input + offset, ciphertext, plaintext_len);
    
    // Generate tag using HMAC
    uint8_t hmac_result[32];
    status = charm_hmac(key, CHARM_AEAD_KEY_SIZE, auth_input, auth_input_len, hmac_result);
    secure_clear(auth_input, auth_input_len);
    free(auth_input);
    
    if (status != CHARM_AEAD_SUCCESS) return status;
    
    // Combine SIV and HMAC result into the tag
    // First 16 bytes: SIV, Last 16 bytes: First 16 bytes of HMAC
    memcpy(tag, siv, 16);
    memcpy(tag + 16, hmac_result, 16);
    
    secure_clear(synthetic_nonce, sizeof(synthetic_nonce));
    secure_clear(hmac_result, sizeof(hmac_result));
    return CHARM_AEAD_SUCCESS;
}

/**
 * @brief SIV CHARM AEAD Decryption
 */
charm_aead_status_t charm_aead_siv_decrypt(
    const uint8_t key[CHARM_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_AEAD_TAG_SIZE],
    uint8_t* plaintext
) {
    if (!key || !ciphertext || !tag || !plaintext) {
        return CHARM_AEAD_ERROR_NULL_POINTER;
    }
    
    // Extract SIV from first 16 bytes of tag
    uint8_t siv[16];
    memcpy(siv, tag, 16);
    
    // Generate keystream using the SIV as nonce
    uint8_t* keystream = malloc(ciphertext_len);
    if (!keystream) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    charm_aead_status_t status = charm_generate_keystream_fast(key, siv, 0, keystream, ciphertext_len);
    if (status != CHARM_AEAD_SUCCESS) {
        free(keystream);
        return status;
    }
    
    // Decrypt: plaintext = ciphertext XOR keystream
    xor_buffers(ciphertext, keystream, plaintext, ciphertext_len);
    secure_clear(keystream, ciphertext_len);
    free(keystream);
    
    // Verify that the SIV is correctly derived from the plaintext
    uint8_t computed_siv[16];
    status = charm_generate_siv(key, aad, aad_len, plaintext, ciphertext_len, computed_siv);
    if (status != CHARM_AEAD_SUCCESS) {
        return status;
    }
    
    // Check if SIV matches
    if (memcmp(siv, computed_siv, 16) != 0) {
        secure_clear(plaintext, ciphertext_len);
        secure_clear(computed_siv, sizeof(computed_siv));
        return CHARM_AEAD_ERROR_AUTH_FAILED;
    }
    
    // Verify authentication tag
    size_t auth_input_len = 3 + 16 + aad_len + ciphertext_len;
    uint8_t* auth_input = malloc(auth_input_len);
    if (!auth_input) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    size_t offset = 0;
    memcpy(auth_input + offset, "SIV", 3);
    offset += 3;
    
    memcpy(auth_input + offset, siv, 16);
    offset += 16;
    
    if (aad && aad_len > 0) {
        memcpy(auth_input + offset, aad, aad_len);
        offset += aad_len;
    }
    
    memcpy(auth_input + offset, ciphertext, ciphertext_len);
    
    uint8_t computed_hmac[32];
    status = charm_hmac(key, CHARM_AEAD_KEY_SIZE, auth_input, auth_input_len, computed_hmac);
    secure_clear(auth_input, auth_input_len);
    free(auth_input);
    
    if (status != CHARM_AEAD_SUCCESS) {
        secure_clear(computed_siv, sizeof(computed_siv));
        return status;
    }
    
    // Compare only the first 16 bytes of HMAC with last 16 bytes of tag
    uint8_t diff = 0;
    for (int i = 0; i < 16; i++) {
        diff |= tag[16 + i] ^ computed_hmac[i];
    }
    secure_clear(computed_hmac, sizeof(computed_hmac));
    secure_clear(computed_siv, sizeof(computed_siv));
    
    if (diff != 0) {
        // Clear plaintext on authentication failure
        secure_clear(plaintext, ciphertext_len);
        return CHARM_AEAD_ERROR_AUTH_FAILED;
    }
    
    return CHARM_AEAD_SUCCESS;
}

/**
 * @brief Benchmark CHARM AEAD performance
 */
charm_aead_status_t charm_aead_benchmark(
    bool mode_siv, size_t payload_size, int iterations,
    double* encrypt_mbps, double* decrypt_mbps
) {
    if (!encrypt_mbps || !decrypt_mbps) return CHARM_AEAD_ERROR_NULL_POINTER;
    
    // Prepare test data
    uint8_t key[CHARM_AEAD_KEY_SIZE] = {0};
    uint8_t nonce[CHARM_AEAD_NONCE_SIZE] = {0};
    uint8_t* plaintext = malloc(payload_size);
    uint8_t* ciphertext = malloc(payload_size);
    uint8_t tag[CHARM_AEAD_TAG_SIZE];
    uint8_t* recovered = malloc(payload_size);
    
    if (!plaintext || !ciphertext || !recovered) {
        free(plaintext);
        free(ciphertext);
        free(recovered);
        return CHARM_AEAD_ERROR_NULL_POINTER;
    }
    
    // Initialize test data
    for (size_t i = 0; i < payload_size; i++) {
        plaintext[i] = (uint8_t)(i & 0xFF);
    }
    
    // Benchmark encryption
    clock_t start = clock();
    for (int i = 0; i < iterations; i++) {
        charm_aead_status_t status;
        if (mode_siv) {
            status = charm_aead_siv_encrypt(key, NULL, 0, plaintext, payload_size, ciphertext, tag);
        } else {
            status = charm_aead_encrypt(key, nonce, NULL, 0, plaintext, payload_size, ciphertext, tag);
        }
        if (status != CHARM_AEAD_SUCCESS) {
            free(plaintext);
            free(ciphertext);
            free(recovered);
            return status;
        }
    }
    clock_t encrypt_time = clock() - start;
    
    // Benchmark decryption
    start = clock();
    for (int i = 0; i < iterations; i++) {
        charm_aead_status_t status;
        if (mode_siv) {
            status = charm_aead_siv_decrypt(key, NULL, 0, ciphertext, payload_size, tag, recovered);
        } else {
            status = charm_aead_decrypt(key, nonce, NULL, 0, ciphertext, payload_size, tag, recovered);
        }
        if (status != CHARM_AEAD_SUCCESS) {
            free(plaintext);
            free(ciphertext);
            free(recovered);
            return status;
        }
    }
    clock_t decrypt_time = clock() - start;
    
    // Calculate throughput
    double total_bytes = (double)payload_size * iterations;
    double encrypt_seconds = (double)encrypt_time / CLOCKS_PER_SEC;
    double decrypt_seconds = (double)decrypt_time / CLOCKS_PER_SEC;
    
    *encrypt_mbps = (total_bytes / (1024 * 1024)) / encrypt_seconds;
    *decrypt_mbps = (total_bytes / (1024 * 1024)) / decrypt_seconds;
    
    free(plaintext);
    free(ciphertext);
    free(recovered);
    
    return CHARM_AEAD_SUCCESS;
}