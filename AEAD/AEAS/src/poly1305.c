/*
 * CHARM – High-Performance Entropy-Native Cryptographic Framework
 * Copyright (c) 2025 Nicholas Woolridge & NOCTRL™ (Nô)
 *
 * This software is licensed under the CHARM License 2025.
 * Use, modification, and distribution are permitted only with
 * verified, real-world test results demonstrating correct
 * functionality, performance, and security.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 * See LICENSE in the repository root for full license details.
 */

/**
 * @file poly1305.c
 * @brief Poly1305 one-time authenticator implementation (RFC 7539)
 * 
 * Constant-time implementation of Poly1305 MAC.
 */

#include "poly1305.h"
#include "util.h"
#include <string.h>

/**
 * @brief Poly1305 prime modulus: 2^130 - 5
 */
static const uint32_t POLY1305_PRIME[5] = {
    0xFFFFFFFB, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x3
};

/**
 * @brief Clamp the r value according to Poly1305 specification
 */
static void poly1305_clamp_r(uint32_t r[5]) {
    r[0] &= 0x0FFFFFFF;
    r[1] &= 0x0FFFFFFC;
    r[2] &= 0x0FFFFFFC;
    r[3] &= 0x0FFFFFFC;
    r[4] = 0;
}

/**
 * @brief Add two 130-bit numbers modulo 2^130 - 5
 */
static void poly1305_add(uint32_t a[5], const uint32_t b[5]) {
    uint64_t carry = 0;
    
    for (int i = 0; i < 5; i++) {
        carry += (uint64_t)a[i] + b[i];
        a[i] = (uint32_t)carry;
        carry >>= 32;
    }
    
    // Reduce modulo 2^130 - 5
    if (carry || a[4] >= 4) {
        uint64_t borrow = 5 * (carry + (a[4] >> 2));
        a[4] &= 3;
        
        for (int i = 0; i < 4; i++) {
            borrow += a[i];
            a[i] = (uint32_t)borrow;
            borrow >>= 32;
        }
        
        if (borrow) {
            a[4] += (uint32_t)borrow;
        }
    }
}

/**
 * @brief Multiply by r modulo 2^130 - 5
 */
static void poly1305_mul_r(uint32_t h[5], const uint32_t r[5]) {
    uint64_t hr[9] = {0};
    
    // Multiply h * r
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            hr[i + j] += (uint64_t)h[i] * r[j];
        }
    }
    
    // Reduce modulo 2^130 - 5
    // High parts (>= 2^130) are multiplied by 5 and added to low parts
    for (int i = 5; i < 9; i++) {
        hr[i - 5] += 5 * hr[i];
    }
    
    // Propagate carries
    for (int i = 0; i < 4; i++) {
        hr[i + 1] += hr[i] >> 32;
        h[i] = (uint32_t)hr[i];
    }
    h[4] = (uint32_t)hr[4];
    
    // Final reduction if needed
    if (h[4] >= 4) {
        uint64_t borrow = 5 * (h[4] >> 2);
        h[4] &= 3;
        
        for (int i = 0; i < 4; i++) {
            borrow += h[i];
            h[i] = (uint32_t)borrow;
            borrow >>= 32;
        }
        
        if (borrow) {
            h[4] += (uint32_t)borrow;
        }
    }
}

/**
 * @brief Process a 16-byte block
 */
static void poly1305_process_block(poly1305_ctx_t* ctx, const uint8_t block[16], int final) {
    uint32_t block_words[5] = {0};
    
    // Convert block to little-endian 32-bit words
    for (int i = 0; i < 4; i++) {
        block_words[i] = util_le32_decode(block + i * 4);
    }
    
    // Add high bit unless this is the final block
    if (!final) {
        block_words[4] = 1;
    }
    
    // h = (h + block) * r mod (2^130 - 5)
    poly1305_add(ctx->h, block_words);
    poly1305_mul_r(ctx->h, ctx->r);
}

/**
 * @brief One-shot Poly1305 MAC computation
 */
int poly1305_mac(const uint8_t key[POLY1305_KEY_SIZE],
                 const uint8_t* data, size_t data_len,
                 uint8_t tag[POLY1305_TAG_SIZE]) {
    
    if (!key || !tag || (!data && data_len > 0)) {
        return -1;
    }
    
    poly1305_ctx_t ctx;
    
    if (poly1305_init(&ctx, key) != 0) {
        return -1;
    }
    
    if (data_len > 0) {
        if (poly1305_update(&ctx, data, data_len) != 0) {
            poly1305_clear(&ctx);
            return -1;
        }
    }
    
    int result = poly1305_final(&ctx, tag);
    poly1305_clear(&ctx);
    
    return result;
}

/**
 * @brief Initialize Poly1305 context
 */
int poly1305_init(poly1305_ctx_t* ctx, const uint8_t key[POLY1305_KEY_SIZE]) {
    if (!ctx || !key) return -1;
    
    // Clear context
    memset(ctx, 0, sizeof(poly1305_ctx_t));
    
    // Extract r from first 16 bytes of key
    for (int i = 0; i < 4; i++) {
        ctx->r[i] = util_le32_decode(key + i * 4);
    }
    ctx->r[4] = 0;
    
    // Clamp r
    poly1305_clamp_r(ctx->r);
    
    // Extract s from last 16 bytes of key
    for (int i = 0; i < 4; i++) {
        ctx->s[i] = util_le32_decode(key + 16 + i * 4);
    }
    
    // Initialize accumulator h to 0
    memset(ctx->h, 0, sizeof(ctx->h));
    
    return 0;
}

/**
 * @brief Update Poly1305 context with data
 */
int poly1305_update(poly1305_ctx_t* ctx, const uint8_t* data, size_t data_len) {
    if (!ctx || ctx->finalized || (!data && data_len > 0)) {
        return -1;
    }
    
    const uint8_t* input = data;
    size_t remaining = data_len;
    
    // Process any buffered data first
    if (ctx->buffer_len > 0) {
        size_t needed = POLY1305_BLOCK_SIZE - ctx->buffer_len;
        size_t to_copy = (remaining < needed) ? remaining : needed;
        
        memcpy(ctx->buffer + ctx->buffer_len, input, to_copy);
        ctx->buffer_len += to_copy;
        input += to_copy;
        remaining -= to_copy;
        
        if (ctx->buffer_len == POLY1305_BLOCK_SIZE) {
            poly1305_process_block(ctx, ctx->buffer, 0);
            ctx->buffer_len = 0;
        }
    }
    
    // Process complete 16-byte blocks
    while (remaining >= POLY1305_BLOCK_SIZE) {
        poly1305_process_block(ctx, input, 0);
        input += POLY1305_BLOCK_SIZE;
        remaining -= POLY1305_BLOCK_SIZE;
    }
    
    // Buffer any remaining data
    if (remaining > 0) {
        memcpy(ctx->buffer, input, remaining);
        ctx->buffer_len = remaining;
    }
    
    return 0;
}

/**
 * @brief Finalize Poly1305 computation
 */
int poly1305_final(poly1305_ctx_t* ctx, uint8_t tag[POLY1305_TAG_SIZE]) {
    if (!ctx || !tag || ctx->finalized) {
        return -1;
    }
    
    // Process final block if any data is buffered
    if (ctx->buffer_len > 0) {
        uint8_t final_block[POLY1305_BLOCK_SIZE] = {0};
        memcpy(final_block, ctx->buffer, ctx->buffer_len);
        final_block[ctx->buffer_len] = 1;  // Add padding bit
        
        poly1305_process_block(ctx, final_block, 1);
        util_secure_clear(final_block, sizeof(final_block));
    }
    
    // Add s to h
    uint64_t carry = 0;
    for (int i = 0; i < 4; i++) {
        carry += (uint64_t)ctx->h[i] + ctx->s[i];
        ctx->h[i] = (uint32_t)carry;
        carry >>= 32;
    }
    
    // Output h as little-endian
    for (int i = 0; i < 4; i++) {
        util_le32_encode(ctx->h[i], tag + i * 4);
    }
    
    ctx->finalized = 1;
    return 0;
}

/**
 * @brief Clear Poly1305 context
 */
void poly1305_clear(poly1305_ctx_t* ctx) {
    if (!ctx) return;
    
    util_secure_clear(ctx, sizeof(poly1305_ctx_t));
}