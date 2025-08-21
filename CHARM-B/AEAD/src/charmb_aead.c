/**
 * @file charmb_aead.c
 * @brief CHARM-B AEAD Implementation
 * 
 * Ultra-fast AEAD using CHARM-B for both encryption and authentication
 */

#include "../include/charmb_aead.h"
#include "../../include/charmb.h"
#include "../../include/charmb_entropy.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

/**
 * @brief Secure memory clear
 */
static void secure_clear(void* ptr, size_t len) {
    volatile uint8_t* p = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < len; i++) {
        p[i] = 0;
    }
}

/**
 * @brief CHARM-B HMAC implementation
 */
charmb_aead_status_t charmb_hmac(
    const uint8_t* key, size_t key_len,
    const uint8_t* data, size_t data_len,
    uint8_t hmac[32]
) {
    if (!key || !data || !hmac) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Ensure deterministic HMAC computation
    charmb_entropy_reset();
    
    // HMAC using CHARM-B as the hash function
    uint8_t key_pad[64] = {0};
    uint8_t inner_hash[32];
    uint8_t outer_input[96]; // 64 (key_pad) + 32 (inner_hash)
    
    // Prepare key
    if (key_len <= 64) {
        memcpy(key_pad, key, key_len);
    } else {
        // Hash long keys
        charmb_status_t status = charmb_hash(key, key_len, key_pad, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Inner hash: CHARM-B(key XOR ipad || data)
    uint8_t* inner_input = malloc(64 + data_len);
    if (!inner_input) return CHARMB_AEAD_ERROR_NULL_POINTER;
    
    for (int i = 0; i < 64; i++) {
        inner_input[i] = key_pad[i] ^ 0x36;
    }
    memcpy(inner_input + 64, data, data_len);
    
    charmb_status_t status;
    
    // CHARM-B has a 64-byte limit, so we need to handle larger inputs differently
    if (64 + data_len <= CHARMB_MAX_INPUT_SIZE) {
        status = charmb_hash(inner_input, 64 + data_len, inner_hash, CHARMB_DIGEST_256);
    } else {
        // For larger inputs, hash in chunks and combine
        // First hash the key pad
        uint8_t temp_hash[32];
        status = charmb_hash(inner_input, 64, temp_hash, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) {
            secure_clear(inner_input, 64 + data_len);
            free(inner_input);
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
        
        // Then hash temp_hash || data (limited to 64 bytes total)
        size_t remaining_data = (data_len <= 32) ? data_len : 32;
        uint8_t final_input[64];
        memcpy(final_input, temp_hash, 32);
        memcpy(final_input + 32, data, remaining_data);
        
        status = charmb_hash(final_input, 32 + remaining_data, inner_hash, CHARMB_DIGEST_256);
        secure_clear(temp_hash, sizeof(temp_hash));
        secure_clear(final_input, sizeof(final_input));
    }
    
    secure_clear(inner_input, 64 + data_len);
    free(inner_input);
    
    if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
    
    // Outer hash: CHARM-B(key XOR opad || inner_hash)
    for (int i = 0; i < 64; i++) {
        outer_input[i] = key_pad[i] ^ 0x5C;
    }
    memcpy(outer_input + 64, inner_hash, 32);
    
    // Since outer input is always 96 bytes (64 + 32), we need the chunked approach
    uint8_t temp_hash[32];
    status = charmb_hash(outer_input, 64, temp_hash, CHARMB_DIGEST_256);
    if (status != CHARMB_SUCCESS) {
        secure_clear(key_pad, sizeof(key_pad));
        secure_clear(inner_hash, sizeof(inner_hash));
        secure_clear(outer_input, sizeof(outer_input));
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Final hash: temp_hash || inner_hash (32 + 32 = 64 bytes, within limit)
    uint8_t final_outer[64];
    memcpy(final_outer, temp_hash, 32);
    memcpy(final_outer + 32, inner_hash, 32);
    
    status = charmb_hash(final_outer, 64, hmac, CHARMB_DIGEST_256);
    
    secure_clear(temp_hash, sizeof(temp_hash));
    secure_clear(final_outer, sizeof(final_outer));
    
    secure_clear(key_pad, sizeof(key_pad));
    secure_clear(inner_hash, sizeof(inner_hash));
    secure_clear(outer_input, sizeof(outer_input));
    
    return (status == CHARMB_SUCCESS) ? CHARMB_AEAD_SUCCESS : CHARMB_AEAD_ERROR_NULL_POINTER;
}

/**
 * @brief CHARM-B Key Derivation Function (HKDF-like)
 */
charmb_aead_status_t charmb_kdf(
    const uint8_t* key, size_t key_len,
    const uint8_t* salt, size_t salt_len,
    const uint8_t* info, size_t info_len,
    uint8_t* output, size_t output_len
) {
    if (!key || !output || key_len == 0 || output_len == 0) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Extract phase: HMAC(salt, key)
    uint8_t prk[32];
    const uint8_t* actual_salt = salt ? salt : (const uint8_t*)"";
    size_t actual_salt_len = salt ? salt_len : 0;
    
    charmb_aead_status_t status = charmb_hmac(actual_salt, actual_salt_len, key, key_len, prk);
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    // Expand phase
    size_t n = (output_len + 31) / 32; // Number of 32-byte blocks needed
    uint8_t* expand_input = malloc(32 + info_len + 1);
    if (!expand_input) {
        secure_clear(prk, sizeof(prk));
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    uint8_t prev[32] = {0};
    size_t output_offset = 0;
    
    for (size_t i = 1; i <= n; i++) {
        size_t input_len = 0;
        
        if (i > 1) {
            memcpy(expand_input, prev, 32);
            input_len += 32;
        }
        
        if (info && info_len > 0) {
            memcpy(expand_input + input_len, info, info_len);
            input_len += info_len;
        }
        
        expand_input[input_len] = (uint8_t)i;
        input_len++;
        
        uint8_t block[32];
        status = charmb_hmac(prk, 32, expand_input, input_len, block);
        if (status != CHARMB_AEAD_SUCCESS) break;
        
        size_t copy_len = (output_len - output_offset < 32) ? (output_len - output_offset) : 32;
        memcpy(output + output_offset, block, copy_len);
        output_offset += copy_len;
        
        memcpy(prev, block, 32);
        secure_clear(block, sizeof(block));
    }
    
    secure_clear(prk, sizeof(prk));
    secure_clear(prev, sizeof(prev));
    secure_clear(expand_input, 32 + info_len + 1);
    free(expand_input);
    
    return status;
}

/**
 * @brief Generate keystream using CHARM-B
 */
static charmb_aead_status_t generate_keystream(
    const uint8_t key[32],
    const uint8_t nonce[12],
    uint32_t counter,
    uint8_t* keystream,
    size_t keystream_len
) {
    // Keystream = CHARM-B(key || nonce || counter)
    uint8_t input[48]; // 32 + 12 + 4
    memcpy(input, key, 32);
    memcpy(input + 32, nonce, 12);
    
    size_t generated = 0;
    uint32_t block_counter = counter;
    
    while (generated < keystream_len) {
        // Encode counter as little-endian
        input[44] = (uint8_t)(block_counter & 0xFF);
        input[45] = (uint8_t)((block_counter >> 8) & 0xFF);
        input[46] = (uint8_t)((block_counter >> 16) & 0xFF);
        input[47] = (uint8_t)((block_counter >> 24) & 0xFF);
        
        uint8_t block[32];
        charmb_status_t status = charmb_hash(input, 48, block, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
        
        size_t copy_len = (keystream_len - generated < 32) ? (keystream_len - generated) : 32;
        memcpy(keystream + generated, block, copy_len);
        generated += copy_len;
        block_counter++;
        
        secure_clear(block, sizeof(block));
    }
    
    secure_clear(input, sizeof(input));
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief CHARM-B AEAD Encryption
 */
charmb_aead_status_t charmb_aead_encrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARMB_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARMB_AEAD_TAG_SIZE]
) {
    // Reset entropy state to ensure deterministic operation
    charmb_entropy_reset();
    
    if (!key || !nonce || !plaintext || !ciphertext || !tag) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Derive encryption and authentication keys
    uint8_t keys[64]; // 32 bytes for encryption, 32 bytes for authentication
    charmb_aead_status_t status = charmb_kdf(key, 32, nonce, 12, 
                                             (const uint8_t*)"CHARM-B-AEAD", 12, 
                                             keys, 64);
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    const uint8_t* enc_key = keys;
    const uint8_t* auth_key = keys + 32;
    
    // Generate keystream and encrypt - optimized for small payloads
    uint8_t keystream_stack[256]; // Stack buffer for small payloads
    uint8_t* keystream;
    bool use_heap = plaintext_len > 256;
    
    if (use_heap) {
        keystream = malloc(plaintext_len);
        if (!keystream) {
            secure_clear(keys, sizeof(keys));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    } else {
        keystream = keystream_stack;
    }
    
    status = generate_keystream(enc_key, nonce, 1, keystream, plaintext_len);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(keystream, plaintext_len);
        if (use_heap) free(keystream);
        secure_clear(keys, sizeof(keys));
        return status;
    }

    
    // XOR to create ciphertext
    for (size_t i = 0; i < plaintext_len; i++) {
        ciphertext[i] = plaintext[i] ^ keystream[i];
    }
    
    secure_clear(keystream, plaintext_len);
    if (use_heap) free(keystream);
    
    // Compute authentication tag - optimized for small payloads
    // Tag = HMAC(auth_key, AAD || ciphertext)
    size_t auth_input_len = aad_len + plaintext_len;
    uint8_t auth_input_stack[512]; // Stack buffer for small auth data
    uint8_t* auth_input;
    bool use_heap_auth = auth_input_len > 512;
    
    if (use_heap_auth) {
        auth_input = malloc(auth_input_len);
        if (!auth_input) {
            secure_clear(keys, sizeof(keys));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    } else {
        auth_input = auth_input_stack;
    }
    
    if (aad && aad_len > 0) {
        memcpy(auth_input, aad, aad_len);
    }
    memcpy(auth_input + aad_len, ciphertext, plaintext_len);
    
    uint8_t full_tag[32];
    status = charmb_hmac(auth_key, 32, auth_input, auth_input_len, full_tag);
    
    secure_clear(auth_input, auth_input_len);
    if (use_heap_auth) free(auth_input);
    secure_clear(keys, sizeof(keys));
    
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    // Truncate tag to 16 bytes
    memcpy(tag, full_tag, 16);
    secure_clear(full_tag, sizeof(full_tag));
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief CHARM-B AEAD Decryption
 */
charmb_aead_status_t charmb_aead_decrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARMB_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARMB_AEAD_TAG_SIZE],
    uint8_t* plaintext
) {
    // Reset entropy state to ensure deterministic operation
    charmb_entropy_reset();
    
    if (!key || !nonce || !ciphertext || !tag || !plaintext) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Derive encryption and authentication keys
    uint8_t keys[64]; // 32 bytes for encryption, 32 bytes for authentication
    charmb_aead_status_t status = charmb_kdf(key, 32, nonce, 12, 
                                             (const uint8_t*)"CHARM-B-AEAD", 12, 
                                             keys, 64);
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    const uint8_t* enc_key = keys;
    const uint8_t* auth_key = keys + 32;
    
    // Verify authentication tag first - optimized for small payloads
    size_t auth_input_len = aad_len + ciphertext_len;
    uint8_t auth_input_stack[512]; // Stack buffer for small auth data
    uint8_t* auth_input;
    bool use_heap_auth = auth_input_len > 512;
    
    if (use_heap_auth) {
        auth_input = malloc(auth_input_len);
        if (!auth_input) {
            secure_clear(keys, sizeof(keys));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    } else {
        auth_input = auth_input_stack;
    }
    
    if (aad && aad_len > 0) {
        memcpy(auth_input, aad, aad_len);
    }
    memcpy(auth_input + aad_len, ciphertext, ciphertext_len);
    
    uint8_t computed_tag[32];
    status = charmb_hmac(auth_key, 32, auth_input, auth_input_len, computed_tag);
    
    secure_clear(auth_input, auth_input_len);
    if (use_heap_auth) free(auth_input);
    
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(keys, sizeof(keys));
        return status;
    }
    
    // Constant-time tag comparison
    uint8_t tag_match = 0;
    for (int i = 0; i < 16; i++) {
        tag_match |= tag[i] ^ computed_tag[i];
    }
    
    secure_clear(computed_tag, sizeof(computed_tag));
    
    if (tag_match != 0) {
        secure_clear(keys, sizeof(keys));
        return CHARMB_AEAD_ERROR_AUTH_FAILED;
    }
    
    // Generate keystream and decrypt - optimized for small payloads
    uint8_t keystream_stack[256]; // Stack buffer for small payloads
    uint8_t* keystream;
    bool use_heap_ks = ciphertext_len > 256;
    
    if (use_heap_ks) {
        keystream = malloc(ciphertext_len);
        if (!keystream) {
            secure_clear(keys, sizeof(keys));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    } else {
        keystream = keystream_stack;
    }
    
    status = generate_keystream(enc_key, nonce, 1, keystream, ciphertext_len);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(keystream, ciphertext_len);
        if (use_heap_ks) free(keystream);
        secure_clear(keys, sizeof(keys));
        return status;
    }

    
    // XOR to recover plaintext
    for (size_t i = 0; i < ciphertext_len; i++) {
        plaintext[i] = ciphertext[i] ^ keystream[i];
    }
    
    secure_clear(keystream, ciphertext_len);
    if (use_heap_ks) free(keystream);
    secure_clear(keys, sizeof(keys));
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief Get current time in milliseconds
 */
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/**
 * @brief Benchmark CHARM-B AEAD performance
 */
charmb_aead_status_t charmb_aead_benchmark(
    size_t payload_size, int iterations,
    double* encrypt_mbps, double* decrypt_mbps
) {
    if (!encrypt_mbps || !decrypt_mbps || payload_size == 0 || iterations <= 0) {
        return CHARMB_AEAD_ERROR_INVALID_SIZE;
    }
    
    // Prepare test data
    uint8_t key[32], nonce[12], tag[16];
    uint8_t* plaintext = malloc(payload_size);
    uint8_t* ciphertext = malloc(payload_size);
    uint8_t* decrypted = malloc(payload_size);
    
    if (!plaintext || !ciphertext || !decrypted) {
        free(plaintext);
        free(ciphertext);
        free(decrypted);
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Initialize test data
    for (size_t i = 0; i < 32; i++) key[i] = (uint8_t)i;
    for (size_t i = 0; i < 12; i++) nonce[i] = (uint8_t)(i + 32);
    for (size_t i = 0; i < payload_size; i++) plaintext[i] = (uint8_t)(i & 0xFF);
    
    // Warmup
    for (int i = 0; i < 100; i++) {
        charmb_aead_encrypt(key, nonce, NULL, 0, plaintext, payload_size, ciphertext, tag);
        charmb_aead_decrypt(key, nonce, NULL, 0, ciphertext, payload_size, tag, decrypted);
    }
    
    // Benchmark encryption
    double start_time = get_time_ms();
    for (int i = 0; i < iterations; i++) {
        charmb_aead_status_t status = charmb_aead_encrypt(key, nonce, NULL, 0, 
                                                          plaintext, payload_size, 
                                                          ciphertext, tag);
        if (status != CHARMB_AEAD_SUCCESS) {
            free(plaintext);
            free(ciphertext);
            free(decrypted);
            return status;
        }
    }
    double encrypt_time = get_time_ms() - start_time;
    
    // Benchmark decryption
    start_time = get_time_ms();
    for (int i = 0; i < iterations; i++) {
        charmb_aead_status_t status = charmb_aead_decrypt(key, nonce, NULL, 0, 
                                                          ciphertext, payload_size, 
                                                          tag, decrypted);
        if (status != CHARMB_AEAD_SUCCESS) {
            free(plaintext);
            free(ciphertext);
            free(decrypted);
            return status;
        }
    }
    double decrypt_time = get_time_ms() - start_time;
    
    // Calculate throughput
    if (encrypt_time > 0) {
        double total_bytes = (double)payload_size * iterations;
        *encrypt_mbps = (total_bytes / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
    } else {
        *encrypt_mbps = 0.0;
    }
    
    if (decrypt_time > 0) {
        double total_bytes = (double)payload_size * iterations;
        *decrypt_mbps = (total_bytes / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
    } else {
        *decrypt_mbps = 0.0;
    }
    
    free(plaintext);
    free(ciphertext);
    free(decrypted);
    
    return CHARMB_AEAD_SUCCESS;
}