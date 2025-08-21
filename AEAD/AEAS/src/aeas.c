/**
 * @file aeas.c
 * @brief CHARM-AEAS (Authenticated Encryption with Adaptive State) implementation
 * 
 * Encrypt-then-MAC AEAD construction using CHARM-derived keystream and Poly1305 authentication.
 */

#include "aeas.h"
#include "hmac_charm.h"
#include "hkdf_charm.h"
#include "poly1305.h"
#include "util.h"
#include "../../algorithm/include/charm.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief Generate keystream using HMAC-CHARM
 */
static int aeas_generate_keystream(const uint8_t k_enc[32],
                                   const uint8_t nonce[AEAS_NONCE_SIZE],
                                   uint64_t counter,
                                   uint32_t block_idx,
                                   uint8_t keystream[32]) {
    
    // Keystream block: HMAC-CHARM(k_enc, nonce || LE64(counter) || LE32(block_idx))
    uint8_t input[AEAS_NONCE_SIZE + 8 + 4];
    
    // Copy nonce
    memcpy(input, nonce, AEAS_NONCE_SIZE);
    
    // Append counter as little-endian 64-bit
    util_le64_encode(counter, input + AEAS_NONCE_SIZE);
    
    // Append block index as little-endian 32-bit
    util_le32_encode(block_idx, input + AEAS_NONCE_SIZE + 8);
    
    int result = hmac_charm(k_enc, 32, input, sizeof(input), keystream);
    
    // Clear sensitive input
    util_secure_clear(input, sizeof(input));
    
    return result;
}

/**
 * @brief Fast HMAC-CHARM for keystream generation with pre-allocated context
 * 
 * This optimization avoids malloc/free in the hot path by reusing a context.
 * For small data encryption, this can provide significant performance gains.
 */
static int aeas_hmac_keystream_fast(const uint8_t k_enc[32],
                                   const uint8_t* input, size_t input_len,
                                   uint8_t output[32]) {
    
    // Use stack allocation for better performance with small data
    uint8_t ctx_buffer[512]; // Reasonable buffer for CHARM context
    charm_ctx_t* ctx = (charm_ctx_t*)ctx_buffer;
    
    uint8_t key_pad[64];
    uint8_t inner_hash[32];
    
    // Prepare key (32-byte keys don't need hashing)
    memcpy(key_pad, k_enc, 32);
    memset(key_pad + 32, 0, 64 - 32);
    
    // Inner hash: CHARM(key XOR ipad || input)
    {
        charm_params_t params = {
            .version = 1,
            .out_bits = 256,
            .flags = 0,
            .reserved = {0}
        };
        
        if (charm_init(ctx, &params, NULL, 0, NULL, 0) != 0) {
            return -1;
        }
        
        // XOR key with ipad and update
        uint8_t ipad_key[64];
        for (int i = 0; i < 64; i++) {
            ipad_key[i] = key_pad[i] ^ 0x36;
        }
        
        if (charm_update(ctx, ipad_key, 64) != 0 ||
            charm_update(ctx, input, input_len) != 0 ||
            charm_final(ctx, inner_hash) != 0) {
            util_secure_clear(ipad_key, sizeof(ipad_key));
            return -1;
        }
        
        util_secure_clear(ipad_key, sizeof(ipad_key));
    }
    
    // Outer hash: CHARM(key XOR opad || inner_hash)
    {
        charm_params_t params = {
            .version = 1,
            .out_bits = 256,
            .flags = 0,
            .reserved = {0}
        };
        
        if (charm_init(ctx, &params, NULL, 0, NULL, 0) != 0) {
            return -1;
        }
        
        // XOR key with opad and update
        uint8_t opad_key[64];
        for (int i = 0; i < 64; i++) {
            opad_key[i] = key_pad[i] ^ 0x5C;
        }
        
        if (charm_update(ctx, opad_key, 64) != 0 ||
            charm_update(ctx, inner_hash, 32) != 0 ||
            charm_final(ctx, output) != 0) {
            util_secure_clear(opad_key, sizeof(opad_key));
            return -1;
        }
        
        util_secure_clear(opad_key, sizeof(opad_key));
    }
    
    util_secure_clear(key_pad, sizeof(key_pad));
    util_secure_clear(inner_hash, sizeof(inner_hash));
    
    return 0;
}

/**
 * @brief Generate multiple keystream blocks efficiently for small data
 * 
 * For small data sizes (up to 4 blocks = 128 bytes), this function optimizes
 * keystream generation by using the fast HMAC implementation.
 */
static int aeas_generate_keystream_small(const uint8_t k_enc[32],
                                        const uint8_t nonce[AEAS_NONCE_SIZE],
                                        uint64_t counter,
                                        uint32_t num_blocks,
                                        uint8_t keystream_out[]) {
    if (num_blocks == 0 || num_blocks > 4) {
        return -1;
    }
    
    // Generate each block using optimized HMAC
    for (uint32_t i = 0; i < num_blocks; i++) {
        uint8_t input[AEAS_NONCE_SIZE + 8 + 4];
        
        // Build input: nonce || counter || block_idx
        memcpy(input, nonce, AEAS_NONCE_SIZE);
        util_le64_encode(counter, input + AEAS_NONCE_SIZE);
        util_le32_encode(i, input + AEAS_NONCE_SIZE + 8);
        
        if (aeas_hmac_keystream_fast(k_enc, input, sizeof(input), 
                                    keystream_out + (i * 32)) != 0) {
            util_secure_clear(input, sizeof(input));
            return -1;
        }
        
        util_secure_clear(input, sizeof(input));
    }
    
    return 0;
}

/**
 * @brief XOR data with keystream
 */
static void aeas_xor_keystream(const uint8_t* input, size_t input_len,
                               const uint8_t k_enc[32],
                               const uint8_t nonce[AEAS_NONCE_SIZE],
                               uint64_t counter,
                               uint8_t* output) {
    
    // Optimize for small data sizes (up to 4 blocks = 128 bytes)
    if (input_len <= 128) {
        uint32_t num_blocks = (uint32_t)((input_len + 31) / 32);
        uint8_t keystream_batch[128]; // Up to 4 blocks
        
        if (aeas_generate_keystream_small(k_enc, nonce, counter, num_blocks, keystream_batch) == 0) {
            // Fast XOR for small data using 64-bit operations where possible
            size_t i = 0;
            
            // XOR 8 bytes at a time for better performance
            while (i + 8 <= input_len) {
                uint64_t input_val, keystream_val;
                memcpy(&input_val, input + i, 8);
                memcpy(&keystream_val, keystream_batch + i, 8);
                uint64_t result = input_val ^ keystream_val;
                memcpy(output + i, &result, 8);
                i += 8;
            }
            
            // Handle remaining bytes
            while (i < input_len) {
                output[i] = input[i] ^ keystream_batch[i];
                i++;
            }
            
            util_secure_clear(keystream_batch, sizeof(keystream_batch));
            return;
        }
        // Fall back to regular method on error
        util_secure_clear(keystream_batch, sizeof(keystream_batch));
    }
    
    // Standard method for larger data or fallback
    size_t offset = 0;
    uint32_t block_idx = 0;
    
    while (offset < input_len) {
        uint8_t keystream[32];
        
        // Generate keystream block
        if (aeas_generate_keystream(k_enc, nonce, counter, block_idx, keystream) != 0) {
            // On error, clear what we can and return
            util_secure_clear(keystream, sizeof(keystream));
            return;
        }
        
        // XOR with keystream
        size_t chunk_len = input_len - offset;
        if (chunk_len > 32) chunk_len = 32;
        
        for (size_t i = 0; i < chunk_len; i++) {
            output[offset + i] = input[offset + i] ^ keystream[i];
        }
        
        util_secure_clear(keystream, sizeof(keystream));
        offset += chunk_len;
        block_idx++;
    }
}

/**
 * @brief Compute Poly1305 authentication tag
 */
static int aeas_compute_tag(const uint8_t k_mac[32],
                            const uint8_t nonce[AEAS_NONCE_SIZE],
                            const uint8_t* aad, size_t aad_len,
                            const uint8_t* ciphertext, size_t ct_len,
                            uint8_t tag[AEAS_TAG_SIZE]) {
    
    // Derive one-time Poly1305 key: HMAC-CHARM(k_mac, nonce)
    uint8_t poly_key[32];
    if (hmac_charm(k_mac, 32, nonce, AEAS_NONCE_SIZE, poly_key) != 0) {
        util_secure_clear(poly_key, sizeof(poly_key));
        return -1;
    }
    
    // Compute Poly1305 over: AAD || pad16 || ciphertext || pad16 || LE64(len(AAD)) || LE64(len(ciphertext))
    poly1305_ctx_t ctx;
    if (poly1305_init(&ctx, poly_key) != 0) {
        util_secure_clear(poly_key, sizeof(poly_key));
        return -1;
    }
    
    // Add AAD
    if (aad_len > 0) {
        if (poly1305_update(&ctx, aad, aad_len) != 0) {
            poly1305_clear(&ctx);
            util_secure_clear(poly_key, sizeof(poly_key));
            return -1;
        }
    }
    
    // Add AAD padding to 16-byte boundary
    size_t aad_pad_len = (16 - (aad_len % 16)) % 16;
    if (aad_pad_len > 0) {
        uint8_t padding[16] = {0};
        if (poly1305_update(&ctx, padding, aad_pad_len) != 0) {
            poly1305_clear(&ctx);
            util_secure_clear(poly_key, sizeof(poly_key));
            return -1;
        }
    }
    
    // Add ciphertext
    if (ct_len > 0) {
        if (poly1305_update(&ctx, ciphertext, ct_len) != 0) {
            poly1305_clear(&ctx);
            util_secure_clear(poly_key, sizeof(poly_key));
            return -1;
        }
    }
    
    // Add ciphertext padding to 16-byte boundary
    size_t ct_pad_len = (16 - (ct_len % 16)) % 16;
    if (ct_pad_len > 0) {
        uint8_t padding[16] = {0};
        if (poly1305_update(&ctx, padding, ct_pad_len) != 0) {
            poly1305_clear(&ctx);
            util_secure_clear(poly_key, sizeof(poly_key));
            return -1;
        }
    }
    
    // Add lengths
    uint8_t lengths[16];
    util_le64_encode(aad_len, lengths);
    util_le64_encode(ct_len, lengths + 8);
    
    if (poly1305_update(&ctx, lengths, 16) != 0) {
        poly1305_clear(&ctx);
        util_secure_clear(poly_key, sizeof(poly_key));
        return -1;
    }
    
    // Finalize
    int result = poly1305_final(&ctx, tag);
    
    poly1305_clear(&ctx);
    util_secure_clear(poly_key, sizeof(poly_key));
    
    return result;
}

/**
 * @brief Encrypt and authenticate data using CHARM-AEAS
 */
int aeas_encrypt(const uint8_t key[AEAS_KEY_SIZE],
                 const uint8_t nonce[AEAS_NONCE_SIZE],
                 const uint8_t* aad, size_t aad_len,
                 const uint8_t* plaintext, size_t pt_len,
                 uint8_t* ciphertext,
                 uint8_t tag[AEAS_TAG_SIZE]) {
    
    if (!key || !nonce || !tag) return AEAS_ERROR_INVALID;
    if (!plaintext && pt_len > 0) return AEAS_ERROR_INVALID;
    if (!aad && aad_len > 0) return AEAS_ERROR_INVALID;
    if (!ciphertext && pt_len > 0) return AEAS_ERROR_INVALID;
    
    // Derive subkeys using HKDF-CHARM
    uint8_t k_enc[32], k_mac[32];
    
    // k_enc = HKDF-CHARM(key, salt=nonce, info="AEAS-enc", L=32)
    const char* enc_info = "AEAS-enc";
    if (hkdf_charm(nonce, AEAS_NONCE_SIZE, key, AEAS_KEY_SIZE,
                   (const uint8_t*)enc_info, strlen(enc_info),
                   k_enc, 32) != 0) {
        util_secure_clear(k_enc, sizeof(k_enc));
        util_secure_clear(k_mac, sizeof(k_mac));
        return AEAS_ERROR_INVALID;
    }
    
    // k_mac = HKDF-CHARM(key, salt=nonce, info="AEAS-mac", L=32)
    const char* mac_info = "AEAS-mac";
    if (hkdf_charm(nonce, AEAS_NONCE_SIZE, key, AEAS_KEY_SIZE,
                   (const uint8_t*)mac_info, strlen(mac_info),
                   k_mac, 32) != 0) {
        util_secure_clear(k_enc, sizeof(k_enc));
        util_secure_clear(k_mac, sizeof(k_mac));
        return AEAS_ERROR_INVALID;
    }
    
    // Encrypt: XOR plaintext with keystream
    if (pt_len > 0) {
        aeas_xor_keystream(plaintext, pt_len, k_enc, nonce, 0, ciphertext);
    }
    
    // Authenticate: Compute Poly1305 MAC
    int result = aeas_compute_tag(k_mac, nonce, aad, aad_len, ciphertext, pt_len, tag);
    
    // Clear sensitive keys
    util_secure_clear(k_enc, sizeof(k_enc));
    util_secure_clear(k_mac, sizeof(k_mac));
    
    return (result == 0) ? AEAS_SUCCESS : AEAS_ERROR_INVALID;
}

/**
 * @brief Decrypt and verify data using CHARM-AEAS
 */
int aeas_decrypt(const uint8_t key[AEAS_KEY_SIZE],
                 const uint8_t nonce[AEAS_NONCE_SIZE],
                 const uint8_t* aad, size_t aad_len,
                 const uint8_t* ciphertext, size_t ct_len,
                 const uint8_t tag[AEAS_TAG_SIZE],
                 uint8_t* plaintext) {
    
    if (!key || !nonce || !tag) return AEAS_ERROR_INVALID;
    if (!ciphertext && ct_len > 0) return AEAS_ERROR_INVALID;
    if (!aad && aad_len > 0) return AEAS_ERROR_INVALID;
    if (!plaintext && ct_len > 0) return AEAS_ERROR_INVALID;
    
    // Derive subkeys using HKDF-CHARM
    uint8_t k_enc[32], k_mac[32];
    
    // k_enc = HKDF-CHARM(key, salt=nonce, info="AEAS-enc", L=32)
    const char* enc_info = "AEAS-enc";
    if (hkdf_charm(nonce, AEAS_NONCE_SIZE, key, AEAS_KEY_SIZE,
                   (const uint8_t*)enc_info, strlen(enc_info),
                   k_enc, 32) != 0) {
        util_secure_clear(k_enc, sizeof(k_enc));
        util_secure_clear(k_mac, sizeof(k_mac));
        return AEAS_ERROR_INVALID;
    }
    
    // k_mac = HKDF-CHARM(key, salt=nonce, info="AEAS-mac", L=32)
    const char* mac_info = "AEAS-mac";
    if (hkdf_charm(nonce, AEAS_NONCE_SIZE, key, AEAS_KEY_SIZE,
                   (const uint8_t*)mac_info, strlen(mac_info),
                   k_mac, 32) != 0) {
        util_secure_clear(k_enc, sizeof(k_enc));
        util_secure_clear(k_mac, sizeof(k_mac));
        return AEAS_ERROR_INVALID;
    }
    
    // Verify authentication tag (constant-time)
    uint8_t expected_tag[AEAS_TAG_SIZE];
    if (aeas_compute_tag(k_mac, nonce, aad, aad_len, ciphertext, ct_len, expected_tag) != 0) {
        util_secure_clear(k_enc, sizeof(k_enc));
        util_secure_clear(k_mac, sizeof(k_mac));
        util_secure_clear(expected_tag, sizeof(expected_tag));
        return AEAS_ERROR_INVALID;
    }
    
    // Constant-time tag comparison
    if (util_ct_memcmp(tag, expected_tag, AEAS_TAG_SIZE) != 0) {
        util_secure_clear(k_enc, sizeof(k_enc));
        util_secure_clear(k_mac, sizeof(k_mac));
        util_secure_clear(expected_tag, sizeof(expected_tag));
        return AEAS_ERROR_AUTH;
    }
    
    util_secure_clear(expected_tag, sizeof(expected_tag));
    
    // Decrypt: XOR ciphertext with keystream
    if (ct_len > 0) {
        aeas_xor_keystream(ciphertext, ct_len, k_enc, nonce, 0, plaintext);
    }
    
    // Clear sensitive keys
    util_secure_clear(k_enc, sizeof(k_enc));
    util_secure_clear(k_mac, sizeof(k_mac));
    
    return AEAS_SUCCESS;
}

/**
 * @brief Initialize nonce context for counter-based nonces
 */
int aeas_nonce_init(aeas_nonce_ctx_t* ctx, const uint8_t prefix[4]) {
    if (!ctx || !prefix) return AEAS_ERROR_INVALID;
    
    memcpy(ctx->prefix, prefix, 4);
    ctx->counter = 0;
    
    return AEAS_SUCCESS;
}

/**
 * @brief Generate next nonce from context
 */
int aeas_nonce_next(aeas_nonce_ctx_t* ctx, uint8_t nonce[AEAS_NONCE_SIZE]) {
    if (!ctx || !nonce) return AEAS_ERROR_INVALID;
    
    // Check for counter overflow
    if (ctx->counter == UINT64_MAX) {
        return AEAS_ERROR_INVALID;
    }
    
    // Build nonce: prefix (4 bytes) || counter (8 bytes)
    memcpy(nonce, ctx->prefix, 4);
    util_le64_encode(ctx->counter, nonce + 4);
    
    ctx->counter++;
    
    return AEAS_SUCCESS;
}