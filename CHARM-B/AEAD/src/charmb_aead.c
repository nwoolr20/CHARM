/**
 * @file charmb_aead.c
 * @brief CHARM-B AEAD Implementation
 * 
 * Ultra-fast AEAD using CHARM-B for both encryption and authentication
 * Optimized for maximum performance to outperform AES-128-GCM
 */

#include "../include/charmb_aead.h"
#include "../../include/charmb.h"
#include "../../include/charmb_entropy.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#ifdef __AVX512F__
#include <immintrin.h>
#define SIMD_AVX512 1
#elif defined(__AVX2__)
#include <immintrin.h>
#define SIMD_AVX2 1
#elif defined(__SSE2__)
#include <emmintrin.h>
#define SIMD_SSE2 1
#endif

// Ultra-fast constants for optimized processing
#define CHARMB_FAST_ROUNDS 8
#define CHARMB_FAST_MIX_CONST 0x9E3779B97F4A7C15ULL

/**
 * @brief Ultra-fast SIMD-optimized XOR operation for encryption/decryption
 */
static inline void xor_buffers_optimized(const uint8_t* src1, const uint8_t* src2, uint8_t* dst, size_t len) {
    size_t i = 0;
    
#ifdef SIMD_AVX512
    // AVX512 optimization for 64-byte chunks - ultimate performance
    for (; i + 64 <= len; i += 64) {
        __m512i a = _mm512_loadu_si512((__m512i*)(src1 + i));
        __m512i b = _mm512_loadu_si512((__m512i*)(src2 + i));
        __m512i result = _mm512_xor_si512(a, b);
        _mm512_storeu_si512((__m512i*)(dst + i), result);
    }
#endif
    
#if defined(SIMD_AVX2) || defined(SIMD_AVX512)
    // AVX2 optimization for 32-byte chunks
    for (; i + 32 <= len; i += 32) {
        __m256i a = _mm256_loadu_si256((__m256i*)(src1 + i));
        __m256i b = _mm256_loadu_si256((__m256i*)(src2 + i));
        __m256i result = _mm256_xor_si256(a, b);
        _mm256_storeu_si256((__m256i*)(dst + i), result);
    }
#endif
    
#if defined(SIMD_SSE2) || defined(SIMD_AVX2) || defined(SIMD_AVX512)
    // SSE2 optimization for 16-byte chunks
    for (; i + 16 <= len; i += 16) {
        __m128i a = _mm_loadu_si128((__m128i*)(src1 + i));
        __m128i b = _mm_loadu_si128((__m128i*)(src2 + i));
        __m128i result = _mm_xor_si128(a, b);
        _mm_storeu_si128((__m128i*)(dst + i), result);
    }
#endif
    
    // Handle remaining bytes with 64-bit operations when possible
    for (; i + 8 <= len; i += 8) {
        *((uint64_t*)(dst + i)) = *((uint64_t*)(src1 + i)) ^ *((uint64_t*)(src2 + i));
    }
    
    // Handle final bytes
    for (; i < len; i++) {
        dst[i] = src1[i] ^ src2[i];
    }
}

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
 * @brief Ultra-fast CHARM-B HMAC implementation optimized for small inputs
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
    
    // Prepare key (optimized path for typical key sizes)
    uint8_t key_pad[32];
    memset(key_pad, 0, 32);
    
    if (key_len <= 32) {
        memcpy(key_pad, key, key_len);
    } else {
        // Hash long keys directly into key_pad
        charmb_status_t status = charmb_hash(key, key_len, key_pad, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // For data larger than what CHARM-B can handle, pre-hash it
    uint8_t data_hash[32];
    const uint8_t* actual_data;
    size_t actual_data_len;
    
    if (data_len > 32) { // Conservative limit to ensure combined input fits
        charmb_status_t status = charmb_hash(data, data_len, data_hash, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
        actual_data = data_hash;
        actual_data_len = 32;
    } else {
        actual_data = data;
        actual_data_len = data_len;
    }
    
    // Stack-allocated buffers for maximum speed
    uint8_t inner_input[64]; // 32 (key) + 32 (data) max
    uint8_t inner_hash[32];
    
    // Inner hash: CHARM-B((key XOR ipad) || data) - ensure size limits
    for (int i = 0; i < 32; i++) {
        inner_input[i] = key_pad[i] ^ 0x36;
    }
    
    memcpy(inner_input + 32, actual_data, actual_data_len);
    charmb_status_t status = charmb_hash(inner_input, 32 + actual_data_len, inner_hash, CHARMB_DIGEST_256);
    if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
    
    // Outer hash: CHARM-B((key XOR opad) || inner_hash)
    uint8_t outer_input[64]; // 32 (key) + 32 (inner_hash)
    for (int i = 0; i < 32; i++) {
        outer_input[i] = key_pad[i] ^ 0x5C;
    }
    memcpy(outer_input + 32, inner_hash, 32);
    
    // Final hash call
    status = charmb_hash(outer_input, 64, hmac, CHARMB_DIGEST_256);
    
    return (status == CHARMB_SUCCESS) ? CHARMB_AEAD_SUCCESS : CHARMB_AEAD_ERROR_NULL_POINTER;
}

/**
 * @brief Ultra-fast keystream generation optimized for small payloads
 */
static inline charmb_aead_status_t generate_fast_keystream(
    const uint8_t key[32],
    const uint8_t nonce[12], 
    uint32_t counter,
    uint8_t* keystream,
    size_t keystream_len
) {
    if (!key || !nonce || !keystream || keystream_len == 0) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Ultra-fast path for small keystreams (≤32 bytes) - single hash
    if (keystream_len <= 32) {
        uint8_t input[48]; // 32 + 12 + 4
        memcpy(input, key, 32);
        memcpy(input + 32, nonce, 12);
        
        // Encode counter as little-endian
        input[44] = (uint8_t)(counter & 0xFF);
        input[45] = (uint8_t)((counter >> 8) & 0xFF);
        input[46] = (uint8_t)((counter >> 16) & 0xFF);
        input[47] = (uint8_t)((counter >> 24) & 0xFF);
        
        uint8_t block[32];
        charmb_status_t status = charmb_hash(input, 48, block, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return CHARMB_AEAD_ERROR_NULL_POINTER;
        
        memcpy(keystream, block, keystream_len);
        return CHARMB_AEAD_SUCCESS;
    }
    
    // Optimized path for larger keystreams using streaming
    uint8_t input[48];
    memcpy(input, key, 32);
    memcpy(input + 32, nonce, 12);
    
    size_t generated = 0;
    uint32_t block_counter = counter;
    
    while (generated < keystream_len) {
        // Encode counter
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
    }
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief CHARM-B Key Derivation Function (HKDF-like) - optimized version
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
 * @brief Ultra-fast CHARM-B AEAD Encryption for small payloads (≤64 bytes)
 * Uses single-hash combined key/keystream generation for maximum performance
 */
static charmb_aead_status_t charmb_aead_encrypt_fast(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARMB_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARMB_AEAD_TAG_SIZE]
) {
    // ULTRA-FAST: Single hash operation generates both keystream and auth key
    // Input: key || nonce || payload_len || "AEAD"
    uint8_t combined_input[52]; // 32 + 12 + 4 + 4  
    memcpy(combined_input, key, 32);
    memcpy(combined_input + 32, nonce, 12);
    
    // Encode payload length for domain separation
    combined_input[44] = (uint8_t)(plaintext_len & 0xFF);
    combined_input[45] = (uint8_t)((plaintext_len >> 8) & 0xFF);
    combined_input[46] = (uint8_t)(aad_len & 0xFF);
    combined_input[47] = (uint8_t)((aad_len >> 8) & 0xFF);
    
    // Domain separator
    memcpy(combined_input + 48, "AEAD", 4);
    
    // Single hash operation produces 32-byte output used for both keystream and auth
    uint8_t master_output[32];
    charmb_status_t status = charmb_hash(combined_input, 52, master_output, CHARMB_DIGEST_256);
    if (status != CHARMB_SUCCESS) {
        secure_clear(combined_input, sizeof(combined_input));
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // For payloads > 32 bytes, we need one more hash for extended keystream
    uint8_t extended_keystream[32];
    bool need_extended = plaintext_len > 32;
    
    if (need_extended) {
        // Use master output as key for second keystream block
        uint8_t extend_input[48];
        memcpy(extend_input, master_output, 32);
        memcpy(extend_input + 32, nonce, 12);
        // Use counter = 1 for second block
        extend_input[44] = 0x01;
        extend_input[45] = 0x00;
        extend_input[46] = 0x00;
        extend_input[47] = 0x00;
        
        status = charmb_hash(extend_input, 48, extended_keystream, CHARMB_DIGEST_256);
        secure_clear(extend_input, sizeof(extend_input));
        if (status != CHARMB_SUCCESS) {
            secure_clear(combined_input, sizeof(combined_input));
            secure_clear(master_output, sizeof(master_output));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    }
    
    // Encrypt with direct keystream from master output
    if (plaintext_len <= 32) {
        // Single block encryption (SIMD-optimized)
        xor_buffers_optimized(plaintext, master_output, ciphertext, plaintext_len);
    } else {
        // Dual block encryption  
        xor_buffers_optimized(plaintext, master_output, ciphertext, 32);
        xor_buffers_optimized(plaintext + 32, extended_keystream, ciphertext + 32, plaintext_len - 32);
    }
    
    // Ultra-fast authentication: Simplified tag = CHARM-B(auth_key || aad || ciphertext)
    // Use last 16 bytes of master output as auth key for efficiency
    const uint8_t* auth_key = master_output + 16;
    
    // Build compact authentication input
    uint8_t auth_input[112]; // 16 (auth_key) + 32 (aad_max) + 64 (ciphertext_max) 
    size_t auth_len = 0;
    
    // Add 16-byte auth key
    memcpy(auth_input + auth_len, auth_key, 16);
    auth_len += 16;
    
    // Add AAD if present
    if (aad && aad_len > 0) {
        memcpy(auth_input + auth_len, aad, aad_len);
        auth_len += aad_len;
    }
    
    // Add ciphertext
    memcpy(auth_input + auth_len, ciphertext, plaintext_len);
    auth_len += plaintext_len;
    
    // Add length encoding for domain separation
    auth_input[auth_len++] = (uint8_t)(aad_len & 0xFF);
    auth_input[auth_len++] = (uint8_t)(plaintext_len & 0xFF);
    
    // Single hash for authentication tag
    uint8_t full_tag[32];
    if (auth_len <= 64) {
        // Optimal case: single hash operation
        status = charmb_hash(auth_input, auth_len, full_tag, CHARMB_DIGEST_256);
    } else {
        // Fallback: two-stage hash for larger auth data
        uint8_t temp_hash[32];
        status = charmb_hash(auth_input, 64, temp_hash, CHARMB_DIGEST_256);
        if (status == CHARMB_SUCCESS) {
            uint8_t final_input[64];
            memcpy(final_input, temp_hash, 32);
            size_t remaining = (auth_len - 64 > 32) ? 32 : (auth_len - 64);
            memcpy(final_input + 32, auth_input + 64, remaining);
            status = charmb_hash(final_input, 32 + remaining, full_tag, CHARMB_DIGEST_256);
            secure_clear(final_input, sizeof(final_input));
        }
        secure_clear(temp_hash, sizeof(temp_hash));
    }
    
    if (status != CHARMB_SUCCESS) {
        secure_clear(combined_input, sizeof(combined_input));
        secure_clear(master_output, sizeof(master_output));
        if (need_extended) secure_clear(extended_keystream, sizeof(extended_keystream));
        secure_clear(auth_input, auth_len);
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Truncate tag to 16 bytes
    memcpy(tag, full_tag, 16);
    
    // Clear sensitive data
    secure_clear(combined_input, sizeof(combined_input));
    secure_clear(master_output, sizeof(master_output));
    if (need_extended) secure_clear(extended_keystream, sizeof(extended_keystream));
    secure_clear(auth_input, auth_len);
    secure_clear(full_tag, sizeof(full_tag));
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief CHARM-B AEAD Encryption - Ultra-optimized version
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
    
    // Ultra-fast path for small payloads using optimized algorithm (within CHARM-B limits)
    if (plaintext_len <= 64 && (aad_len == 0 || aad_len <= 32)) {
        return charmb_aead_encrypt_fast(key, nonce, aad, aad_len, plaintext, plaintext_len, ciphertext, tag);
    }
    
    // Use fast keystream generation for larger payloads
    charmb_aead_status_t status = generate_fast_keystream(key, nonce, 0, ciphertext, plaintext_len);
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    // Encrypt using optimized XOR
    xor_buffers_optimized(plaintext, ciphertext, ciphertext, plaintext_len);
    
    // Generate authentication tag using fast HMAC
    // Calculate required buffer size
    size_t expected_data_len = (plaintext_len <= 256) ? plaintext_len : 32;
    size_t required_auth_len = aad_len + expected_data_len + 4; // +4 for length encoding
    
    uint8_t auth_input_stack[512];
    uint8_t* auth_input;
    bool use_heap_auth = required_auth_len > 512;
    
    if (use_heap_auth) {
        auth_input = malloc(required_auth_len);
        if (!auth_input) return CHARMB_AEAD_ERROR_NULL_POINTER;
    } else {
        auth_input = auth_input_stack;
    }
    
    size_t auth_len = 0;
    
    // Build auth input: aad || ciphertext || lengths
    if (aad && aad_len > 0) {
        memcpy(auth_input + auth_len, aad, aad_len);
        auth_len += aad_len;
    }
    
    // For large ciphertext, use hash approach
    if (plaintext_len <= 256) {
        memcpy(auth_input + auth_len, ciphertext, plaintext_len);
        auth_len += plaintext_len;
    } else {
        // Hash large ciphertext first
        uint8_t ct_hash[32];
        charmb_aead_status_t hash_status = charmb_hmac(key, 32, ciphertext, plaintext_len, ct_hash);
        if (hash_status != CHARMB_AEAD_SUCCESS) {
            if (use_heap_auth) free(auth_input);
            return hash_status;
        }
        
        memcpy(auth_input + auth_len, ct_hash, 32);
        auth_len += 32;
    }
    
    // Add length encoding
    auth_input[auth_len++] = (uint8_t)(aad_len & 0xFF);
    auth_input[auth_len++] = (uint8_t)((aad_len >> 8) & 0xFF);
    auth_input[auth_len++] = (uint8_t)(plaintext_len & 0xFF);
    auth_input[auth_len++] = (uint8_t)((plaintext_len >> 8) & 0xFF);
    
    // Generate tag using fast HMAC
    uint8_t full_tag[32];
    status = charmb_hmac(key, 32, auth_input, auth_len, full_tag);
    
    if (use_heap_auth) free(auth_input);
    
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    // Truncate to 16 bytes
    memcpy(tag, full_tag, 16);
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief Ultra-fast CHARM-B AEAD Decryption for small payloads (≤64 bytes)
 * Uses single-hash combined key/keystream generation matching encryption
 */
static charmb_aead_status_t charmb_aead_decrypt_fast(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t nonce[CHARMB_AEAD_NONCE_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARMB_AEAD_TAG_SIZE],
    uint8_t* plaintext
) {
    // ULTRA-FAST: Single hash operation generates both keystream and auth key (same as encrypt)
    uint8_t combined_input[52]; // 32 + 12 + 4 + 4  
    memcpy(combined_input, key, 32);
    memcpy(combined_input + 32, nonce, 12);
    
    // Encode payload length for domain separation (same as encrypt)
    combined_input[44] = (uint8_t)(ciphertext_len & 0xFF);
    combined_input[45] = (uint8_t)((ciphertext_len >> 8) & 0xFF);
    combined_input[46] = (uint8_t)(aad_len & 0xFF);
    combined_input[47] = (uint8_t)((aad_len >> 8) & 0xFF);
    
    // Domain separator
    memcpy(combined_input + 48, "AEAD", 4);
    
    // Single hash operation produces 32-byte output used for both keystream and auth
    uint8_t master_output[32];
    charmb_status_t status = charmb_hash(combined_input, 52, master_output, CHARMB_DIGEST_256);
    if (status != CHARMB_SUCCESS) {
        secure_clear(combined_input, sizeof(combined_input));
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // For payloads > 32 bytes, we need one more hash for extended keystream
    uint8_t extended_keystream[32];
    bool need_extended = ciphertext_len > 32;
    
    if (need_extended) {
        uint8_t extend_input[48];
        memcpy(extend_input, master_output, 32);
        memcpy(extend_input + 32, nonce, 12);
        extend_input[44] = 0x01;
        extend_input[45] = 0x00;
        extend_input[46] = 0x00;
        extend_input[47] = 0x00;
        
        status = charmb_hash(extend_input, 48, extended_keystream, CHARMB_DIGEST_256);
        secure_clear(extend_input, sizeof(extend_input));
        if (status != CHARMB_SUCCESS) {
            secure_clear(combined_input, sizeof(combined_input));
            secure_clear(master_output, sizeof(master_output));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    }
    
    // Verify authentication tag FIRST (authenticate before decrypt)
    const uint8_t* auth_key = master_output + 16;
    uint8_t auth_input[112]; // 16 + 32 + 64 max
    size_t auth_len = 0;
    
    // Add 16-byte auth key
    memcpy(auth_input + auth_len, auth_key, 16);
    auth_len += 16;
    
    // Add AAD if present
    if (aad && aad_len > 0) {
        memcpy(auth_input + auth_len, aad, aad_len);
        auth_len += aad_len;
    }
    
    // Add ciphertext
    memcpy(auth_input + auth_len, ciphertext, ciphertext_len);
    auth_len += ciphertext_len;
    
    // Add length encoding for domain separation
    auth_input[auth_len++] = (uint8_t)(aad_len & 0xFF);
    auth_input[auth_len++] = (uint8_t)(ciphertext_len & 0xFF);
    
    // Compute authentication tag
    uint8_t computed_tag[32];
    if (auth_len <= 64) {
        status = charmb_hash(auth_input, auth_len, computed_tag, CHARMB_DIGEST_256);
    } else {
        uint8_t temp_hash[32];
        status = charmb_hash(auth_input, 64, temp_hash, CHARMB_DIGEST_256);
        if (status == CHARMB_SUCCESS) {
            uint8_t final_input[64];
            memcpy(final_input, temp_hash, 32);
            size_t remaining = (auth_len - 64 > 32) ? 32 : (auth_len - 64);
            memcpy(final_input + 32, auth_input + 64, remaining);
            status = charmb_hash(final_input, 32 + remaining, computed_tag, CHARMB_DIGEST_256);
            secure_clear(final_input, sizeof(final_input));
        }
        secure_clear(temp_hash, sizeof(temp_hash));
    }
    
    if (status != CHARMB_SUCCESS) {
        secure_clear(combined_input, sizeof(combined_input));
        secure_clear(master_output, sizeof(master_output));
        if (need_extended) secure_clear(extended_keystream, sizeof(extended_keystream));
        secure_clear(auth_input, auth_len);
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    // Constant-time tag comparison (first 16 bytes)
    uint8_t tag_match = 0;
    for (int i = 0; i < 16; i++) {
        tag_match |= tag[i] ^ computed_tag[i];
    }
    
    if (tag_match != 0) {
        // Authentication failure - clear everything and return error
        secure_clear(combined_input, sizeof(combined_input));
        secure_clear(master_output, sizeof(master_output));
        if (need_extended) secure_clear(extended_keystream, sizeof(extended_keystream));
        secure_clear(auth_input, auth_len);
        secure_clear(computed_tag, sizeof(computed_tag));
        return CHARMB_AEAD_ERROR_AUTH_FAILED;
    }
    
    // Authentication successful - decrypt the ciphertext
    if (ciphertext_len <= 32) {
        // Single block decryption (SIMD-optimized)
        xor_buffers_optimized(ciphertext, master_output, plaintext, ciphertext_len);
    } else {
        // Dual block decryption
        xor_buffers_optimized(ciphertext, master_output, plaintext, 32);
        xor_buffers_optimized(ciphertext + 32, extended_keystream, plaintext + 32, ciphertext_len - 32);
    }
    
    // Clear sensitive data
    secure_clear(combined_input, sizeof(combined_input));
    secure_clear(master_output, sizeof(master_output));
    if (need_extended) secure_clear(extended_keystream, sizeof(extended_keystream));
    secure_clear(auth_input, auth_len);
    secure_clear(computed_tag, sizeof(computed_tag));
    
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
    
    // Ultra-fast path for small payloads (within CHARM-B limits)
    if (ciphertext_len <= 64 && (aad_len == 0 || aad_len <= 32)) {
        return charmb_aead_decrypt_fast(key, nonce, aad, aad_len, ciphertext, ciphertext_len, tag, plaintext);
    }
    
    // Fast authentication verification first
    // Calculate required buffer size
    size_t expected_data_len = (ciphertext_len <= 256) ? ciphertext_len : 32;
    size_t required_auth_len = aad_len + expected_data_len + 4; // +4 for length encoding
    
    uint8_t auth_input_stack[512];
    uint8_t* auth_input;
    bool use_heap_auth = required_auth_len > 512;
    
    if (use_heap_auth) {
        auth_input = malloc(required_auth_len);
        if (!auth_input) return CHARMB_AEAD_ERROR_NULL_POINTER;
    } else {
        auth_input = auth_input_stack;
    }
    
    size_t auth_len = 0;
    
    // Build auth input: aad || ciphertext || lengths
    if (aad && aad_len > 0) {
        memcpy(auth_input + auth_len, aad, aad_len);
        auth_len += aad_len;
    }
    
    // For large ciphertext, use hash approach
    if (ciphertext_len <= 256) {
        memcpy(auth_input + auth_len, ciphertext, ciphertext_len);
        auth_len += ciphertext_len;
    } else {
        // Hash large ciphertext
        uint8_t ct_hash[32];
        charmb_aead_status_t status = charmb_hmac(key, 32, ciphertext, ciphertext_len, ct_hash);
        if (status != CHARMB_AEAD_SUCCESS) {
            if (use_heap_auth) free(auth_input);
            return status;
        }
        
        memcpy(auth_input + auth_len, ct_hash, 32);
        auth_len += 32;
    }
    
    // Add length encoding
    auth_input[auth_len++] = (uint8_t)(aad_len & 0xFF);
    auth_input[auth_len++] = (uint8_t)((aad_len >> 8) & 0xFF);
    auth_input[auth_len++] = (uint8_t)(ciphertext_len & 0xFF);
    auth_input[auth_len++] = (uint8_t)((ciphertext_len >> 8) & 0xFF);
    
    // Verify authentication tag
    uint8_t computed_tag[32];
    charmb_aead_status_t status = charmb_hmac(key, 32, auth_input, auth_len, computed_tag);
    
    if (use_heap_auth) free(auth_input);
    
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    // Constant-time tag comparison
    uint8_t tag_match = 0;
    for (int i = 0; i < 16; i++) {
        tag_match |= tag[i] ^ computed_tag[i];
    }
    
    if (tag_match != 0) {
        return CHARMB_AEAD_ERROR_AUTH_FAILED;
    }
    
    // Generate keystream and decrypt
    status = generate_fast_keystream(key, nonce, 0, plaintext, ciphertext_len);
    if (status != CHARMB_AEAD_SUCCESS) return status;
    
    // Decrypt using optimized XOR
    xor_buffers_optimized(ciphertext, plaintext, plaintext, ciphertext_len);
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief Generate Synthetic IV (SIV) from key, AAD, and plaintext
 */
static charmb_aead_status_t generate_siv(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t siv[CHARMB_AEAD_NONCE_SIZE]
) {
    // For SIV generation, use a more robust approach with KDF
    // Create a unique salt from plaintext and AAD for better differentiation
    
    // Use first part of plaintext as salt if available
    const uint8_t* salt = plaintext;
    size_t salt_len = (plaintext_len > 16) ? 16 : plaintext_len;
    
    // Create info string that includes lengths for domain separation
    uint8_t info[64]; // "SIV-MODE" + lengths + partial AAD + partial plaintext
    size_t info_len = 0;
    
    memcpy(info + info_len, "SIV-MODE", 8);
    info_len += 8;
    
    // Add length information for proper domain separation
    info[info_len++] = (uint8_t)(aad_len & 0xFF);
    info[info_len++] = (uint8_t)((aad_len >> 8) & 0xFF);
    info[info_len++] = (uint8_t)(plaintext_len & 0xFF);
    info[info_len++] = (uint8_t)((plaintext_len >> 8) & 0xFF);
    
    // Add partial AAD if present
    if (aad && aad_len > 0) {
        size_t aad_copy = (aad_len > 16) ? 16 : aad_len;
        memcpy(info + info_len, aad, aad_copy);
        info_len += aad_copy;
    }
    
    // Add remaining plaintext (after salt)
    if (plaintext_len > salt_len) {
        size_t remaining_pt_len = plaintext_len - salt_len;
        size_t pt_copy = (remaining_pt_len > 32) ? 32 : remaining_pt_len;
        memcpy(info + info_len, plaintext + salt_len, pt_copy);
        info_len += pt_copy;
    }
    
    // Use KDF to generate SIV
    uint8_t siv_output[32];
    charmb_aead_status_t status = charmb_kdf(key, CHARMB_AEAD_KEY_SIZE, salt, salt_len, info, info_len, siv_output, 32);
    
    if (status == CHARMB_AEAD_SUCCESS) {
        // Truncate to 12 bytes for SIV
        memcpy(siv, siv_output, CHARMB_AEAD_NONCE_SIZE);
    }
    
    secure_clear(siv_output, sizeof(siv_output));
    secure_clear(info, info_len);
    
    return status;
}

/**
 * @brief CHARM-B AEAD SIV Encryption - Misuse-Resistant
 */
charmb_aead_status_t charmb_aead_siv_encrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARMB_AEAD_TAG_SIZE]
) {
    // Reset entropy state to ensure deterministic operation
    charmb_entropy_reset();
    
    if (!key || !plaintext || !ciphertext || !tag) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    if (plaintext_len == 0) {
        return CHARMB_AEAD_ERROR_INVALID_SIZE;
    }
    
    // Generate synthetic IV from key, AAD, and plaintext
    uint8_t siv[CHARMB_AEAD_NONCE_SIZE];
    charmb_aead_status_t status = generate_siv(key, aad, aad_len, plaintext, plaintext_len, siv);
    if (status != CHARMB_AEAD_SUCCESS) {
        return status;
    }
    
    // Generate keystream using the SIV as nonce
    uint8_t keystream_stack[256];
    uint8_t* keystream;
    bool use_heap = plaintext_len > 256;
    
    if (use_heap) {
        keystream = malloc(plaintext_len);
        if (!keystream) {
            secure_clear(siv, sizeof(siv));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    } else {
        keystream = keystream_stack;
    }
    
    // Derive encryption key from main key using SIV
    uint8_t enc_key[32];
    status = charmb_kdf(key, 32, siv, 12, (const uint8_t*)"SIV-ENC", 7, enc_key, 32);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        if (use_heap) free(keystream);
        return status;
    }
    
    status = generate_keystream(enc_key, siv, 0, keystream, plaintext_len);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        secure_clear(enc_key, sizeof(enc_key));
        secure_clear(keystream, plaintext_len);
        if (use_heap) free(keystream);
        return status;
    }
    
    // XOR to produce ciphertext
    xor_buffers_optimized(plaintext, keystream, ciphertext, plaintext_len);
    
    // The tag is just the SIV (first 12 bytes) plus a 4-byte auth code
    memcpy(tag, siv, CHARMB_AEAD_NONCE_SIZE);
    
    // Generate authentication tag for the last 4 bytes using HMAC
    // Use HMAC to handle arbitrary input sizes safely
    size_t auth_input_len = aad_len + plaintext_len;
    uint8_t* auth_input = malloc(auth_input_len);
    if (!auth_input) {
        secure_clear(siv, sizeof(siv));
        secure_clear(enc_key, sizeof(enc_key));
        secure_clear(keystream, plaintext_len);
        if (use_heap) free(keystream);
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    size_t offset = 0;
    if (aad && aad_len > 0) {
        memcpy(auth_input + offset, aad, aad_len);
        offset += aad_len;
    }
    memcpy(auth_input + offset, ciphertext, plaintext_len);
    
    uint8_t auth_tag[32];
    status = charmb_hmac(key, 32, auth_input, auth_input_len, auth_tag);
    
    secure_clear(auth_input, auth_input_len);
    free(auth_input);
    
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        secure_clear(enc_key, sizeof(enc_key));
        secure_clear(keystream, plaintext_len);
        if (use_heap) free(keystream);
        return status;
    }
    
    memcpy(tag + CHARMB_AEAD_NONCE_SIZE, auth_tag, 4);
    
    secure_clear(siv, sizeof(siv));
    secure_clear(enc_key, sizeof(enc_key));
    secure_clear(keystream, plaintext_len);
    secure_clear(auth_tag, sizeof(auth_tag));
    if (use_heap) free(keystream);
    
    return CHARMB_AEAD_SUCCESS;
}

/**
 * @brief CHARM-B AEAD SIV Decryption - Misuse-Resistant
 */
charmb_aead_status_t charmb_aead_siv_decrypt(
    const uint8_t key[CHARMB_AEAD_KEY_SIZE],
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARMB_AEAD_TAG_SIZE],
    uint8_t* plaintext
) {
    // Reset entropy state to ensure deterministic operation
    charmb_entropy_reset();
    
    if (!key || !ciphertext || !tag || !plaintext) {
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    if (ciphertext_len == 0) {
        return CHARMB_AEAD_ERROR_INVALID_SIZE;
    }
    
    // Extract SIV from the first 12 bytes of tag
    uint8_t siv[CHARMB_AEAD_NONCE_SIZE];
    memcpy(siv, tag, CHARMB_AEAD_NONCE_SIZE);
    
    // Verify the authentication tag (last 4 bytes) using HMAC
    size_t auth_input_len = aad_len + ciphertext_len;
    uint8_t* auth_input = malloc(auth_input_len);
    if (!auth_input) {
        secure_clear(siv, sizeof(siv));
        return CHARMB_AEAD_ERROR_NULL_POINTER;
    }
    
    size_t offset = 0;
    if (aad && aad_len > 0) {
        memcpy(auth_input + offset, aad, aad_len);
        offset += aad_len;
    }
    memcpy(auth_input + offset, ciphertext, ciphertext_len);
    
    uint8_t computed_auth_tag[32];
    charmb_aead_status_t status = charmb_hmac(key, 32, auth_input, auth_input_len, computed_auth_tag);
    
    secure_clear(auth_input, auth_input_len);
    free(auth_input);
    
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        return status;
    }
    
    // Constant-time comparison of auth tag
    uint8_t auth_match = 0;
    for (int i = 0; i < 4; i++) {
        auth_match |= tag[CHARMB_AEAD_NONCE_SIZE + i] ^ computed_auth_tag[i];
    }
    
    secure_clear(computed_auth_tag, sizeof(computed_auth_tag));
    
    if (auth_match != 0) {
        secure_clear(siv, sizeof(siv));
        return CHARMB_AEAD_ERROR_AUTH_FAILED;
    }
    
    // Generate keystream using the SIV as nonce
    uint8_t keystream_stack[256];
    uint8_t* keystream;
    bool use_heap = ciphertext_len > 256;
    
    if (use_heap) {
        keystream = malloc(ciphertext_len);
        if (!keystream) {
            secure_clear(siv, sizeof(siv));
            return CHARMB_AEAD_ERROR_NULL_POINTER;
        }
    } else {
        keystream = keystream_stack;
    }
    
    // Derive same encryption key from main key using SIV
    uint8_t enc_key[32];
    status = charmb_kdf(key, 32, siv, 12, (const uint8_t*)"SIV-ENC", 7, enc_key, 32);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        if (use_heap) free(keystream);
        return status;
    }
    
    status = generate_keystream(enc_key, siv, 0, keystream, ciphertext_len);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        secure_clear(enc_key, sizeof(enc_key));
        secure_clear(keystream, ciphertext_len);
        if (use_heap) free(keystream);
        return status;
    }
    
    // XOR to recover plaintext
    xor_buffers_optimized(ciphertext, keystream, plaintext, ciphertext_len);
    
    // Verify that the decrypted plaintext produces the same SIV
    uint8_t computed_siv[CHARMB_AEAD_NONCE_SIZE];
    status = generate_siv(key, aad, aad_len, plaintext, ciphertext_len, computed_siv);
    if (status != CHARMB_AEAD_SUCCESS) {
        secure_clear(siv, sizeof(siv));
        secure_clear(enc_key, sizeof(enc_key));
        secure_clear(keystream, ciphertext_len);
        secure_clear(plaintext, ciphertext_len);
        if (use_heap) free(keystream);
        return status;
    }
    
    // Constant-time SIV comparison
    uint8_t siv_match = 0;
    for (int i = 0; i < CHARMB_AEAD_NONCE_SIZE; i++) {
        siv_match |= siv[i] ^ computed_siv[i];
    }
    
    secure_clear(siv, sizeof(siv));
    secure_clear(enc_key, sizeof(enc_key));
    secure_clear(keystream, ciphertext_len);
    secure_clear(computed_siv, sizeof(computed_siv));
    if (use_heap) free(keystream);
    
    if (siv_match != 0) {
        secure_clear(plaintext, ciphertext_len);
        return CHARMB_AEAD_ERROR_AUTH_FAILED;
    }
    
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