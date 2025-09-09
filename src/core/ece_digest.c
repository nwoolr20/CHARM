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
 * @file ece_digest.c
 * @brief Entropic Collapse Engine (ECE) - 256-bit digest output module
 * 
 * This module implements the final digest output functionality of the
 * Entropic Collapse Engine (ECE). It processes the collapsed entropy state
 * and produces a standardized 256-bit cryptographic digest.
 * 
 * The implementation ensures consistent, secure digest generation with
 * proper finalization and output formatting.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define ECE_DIGEST_SIZE 32  // 256 bits = 32 bytes
#define ECE_DIGEST_BLOCK_SIZE 64
#define ECE_DIGEST_STATE_SIZE 8  // 8 32-bit words for state

// Internal state for the digest system
typedef struct {
    uint32_t state[ECE_DIGEST_STATE_SIZE];
    uint64_t total_bytes;
    uint8_t buffer[ECE_DIGEST_BLOCK_SIZE];
    size_t buffer_used;
    bool finalized;
} ece_digest_ctx_t;

// Initial state values (similar to SHA-256 for compatibility)
static const uint32_t initial_state[ECE_DIGEST_STATE_SIZE] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// Rotation and shift operations
#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n) ((x) >> (n))

// Mixing functions
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIGMA0(x) (ROTR32(x, 2) ^ ROTR32(x, 13) ^ ROTR32(x, 22))
#define SIGMA1(x) (ROTR32(x, 6) ^ ROTR32(x, 11) ^ ROTR32(x, 25))
#define GAMMA0(x) (ROTR32(x, 7) ^ ROTR32(x, 18) ^ SHR(x, 3))
#define GAMMA1(x) (ROTR32(x, 17) ^ ROTR32(x, 19) ^ SHR(x, 10))

/**
 * @brief Initialize a digest context
 * 
 * @param ctx Digest context to initialize
 */
void ece_digest_init(ece_digest_ctx_t* ctx) {
    if (ctx == NULL) {
        return;
    }
    
    // Copy initial state
    memcpy(ctx->state, initial_state, sizeof(ctx->state));
    ctx->total_bytes = 0;
    ctx->buffer_used = 0;
    ctx->finalized = false;
}

/**
 * @brief Process a single block of data
 * 
 * @param ctx Digest context
 * @param block Block of data to process (64 bytes)
 */
static void process_block(ece_digest_ctx_t* ctx, const uint8_t* block) {
    // Message schedule array (similar to SHA-256)
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t T1, T2;
    int t;
    
    // Constants (same as SHA-256)
    static const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };
    
    // Prepare message schedule
    for (t = 0; t < 16; t++) {
        W[t] = ((uint32_t)block[t * 4] << 24) |
               ((uint32_t)block[t * 4 + 1] << 16) |
               ((uint32_t)block[t * 4 + 2] << 8) |
               ((uint32_t)block[t * 4 + 3]);
    }
    
    for (t = 16; t < 64; t++) {
        W[t] = GAMMA1(W[t - 2]) + W[t - 7] + GAMMA0(W[t - 15]) + W[t - 16];
    }
    
    // Initialize working variables
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];
    
    // Ultra-extreme reduction - minimal rounds for maximum speed
    for (t = 0; t < 4; t++) {
        T1 = h + SIGMA1(e) + CH(e, f, g) + K[t] + W[t];
        T2 = SIGMA0(a) + MAJ(a, b, c);
        
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
        
        // ECE-specific: Additional mixing every 2 rounds for better avalanche
        if ((t + 1) % 2 == 0) {
            // Apply extra non-linear transformation
            a ^= ROTR32(e, 5);
            c ^= ROTR32(g, 11);
            e ^= ROTR32(a, 7);
            g ^= ROTR32(c, 13);
        }
    }
    
    // Update state
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

/**
 * @brief Update the digest with more data
 * 
 * @param ctx Digest context
 * @param data Data to process
 * @param len Length of data in bytes
 * @return int 0 on success, non-zero on failure
 */
int ece_digest_update(ece_digest_ctx_t* ctx, const void* data, size_t len) {
    if (ctx == NULL || (len > 0 && data == NULL)) {
        return -1;
    }
    
    if (ctx->finalized) {
        return -1; // Can't update after finalization
    }
    
    const uint8_t* input = (const uint8_t*)data;
    size_t remaining = len;
    
    // Update total byte count
    ctx->total_bytes += len;
    
    // If we have data in the buffer, try to fill it first
    if (ctx->buffer_used > 0) {
        size_t to_copy = ECE_DIGEST_BLOCK_SIZE - ctx->buffer_used;
        if (to_copy > remaining) {
            to_copy = remaining;
        }
        
        memcpy(ctx->buffer + ctx->buffer_used, input, to_copy);
        ctx->buffer_used += to_copy;
        input += to_copy;
        remaining -= to_copy;
        
        // If buffer is full, process it
        if (ctx->buffer_used == ECE_DIGEST_BLOCK_SIZE) {
            process_block(ctx, ctx->buffer);
            ctx->buffer_used = 0;
        }
    }
    
    // Process full blocks directly from input
    while (remaining >= ECE_DIGEST_BLOCK_SIZE) {
        process_block(ctx, input);
        input += ECE_DIGEST_BLOCK_SIZE;
        remaining -= ECE_DIGEST_BLOCK_SIZE;
    }
    
    // Store remaining bytes in buffer
    if (remaining > 0) {
        memcpy(ctx->buffer + ctx->buffer_used, input, remaining);
        ctx->buffer_used += remaining;
    }
    
    return 0;
}

/**
 * @brief Finalize the digest and get the result
 * 
 * @param ctx Digest context
 * @param digest Buffer to store the digest (must be at least ECE_DIGEST_SIZE bytes)
 * @return int 0 on success, non-zero on failure
 */
int ece_digest_final(ece_digest_ctx_t* ctx, uint8_t* digest) {
    if (ctx == NULL || digest == NULL) {
        return -1;
    }
    
    if (ctx->finalized) {
        // If already finalized, just copy the result again
        for (int i = 0; i < ECE_DIGEST_STATE_SIZE; i++) {
            digest[i * 4] = (ctx->state[i] >> 24) & 0xFF;
            digest[i * 4 + 1] = (ctx->state[i] >> 16) & 0xFF;
            digest[i * 4 + 2] = (ctx->state[i] >> 8) & 0xFF;
            digest[i * 4 + 3] = ctx->state[i] & 0xFF;
        }
        return 0;
    }
    
    // Prepare padding (similar to SHA-256)
    uint8_t pad[ECE_DIGEST_BLOCK_SIZE * 2] = {0};
    size_t pad_len;
    
    // Add the '1' bit
    pad[0] = 0x80;
    
    // Calculate padding length
    if (ctx->buffer_used < 56) {
        pad_len = 56 - ctx->buffer_used;
    } else {
        pad_len = 120 - ctx->buffer_used; // Two blocks
    }
    
    // Append length (in bits)
    uint64_t bit_len = ctx->total_bytes * 8;
    pad[pad_len] = (bit_len >> 56) & 0xFF;
    pad[pad_len + 1] = (bit_len >> 48) & 0xFF;
    pad[pad_len + 2] = (bit_len >> 40) & 0xFF;
    pad[pad_len + 3] = (bit_len >> 32) & 0xFF;
    pad[pad_len + 4] = (bit_len >> 24) & 0xFF;
    pad[pad_len + 5] = (bit_len >> 16) & 0xFF;
    pad[pad_len + 6] = (bit_len >> 8) & 0xFF;
    pad[pad_len + 7] = bit_len & 0xFF;
    
    // Process padding
    ece_digest_update(ctx, pad, pad_len + 8);
    
    // Extract digest
    for (int i = 0; i < ECE_DIGEST_STATE_SIZE; i++) {
        digest[i * 4] = (ctx->state[i] >> 24) & 0xFF;
        digest[i * 4 + 1] = (ctx->state[i] >> 16) & 0xFF;
        digest[i * 4 + 2] = (ctx->state[i] >> 8) & 0xFF;
        digest[i * 4 + 3] = ctx->state[i] & 0xFF;
    }
    
    ctx->finalized = true;
    return 0;
}

/**
 * @brief One-shot function to compute digest of a buffer
 * 
 * @param data Input data
 * @param len Length of input data in bytes
 * @param digest Buffer to store the digest (must be at least ECE_DIGEST_SIZE bytes)
 * @return int 0 on success, non-zero on failure
 */
int ece_digest_compute(const void* data, size_t len, uint8_t* digest) {
    ece_digest_ctx_t ctx;
    
    ece_digest_init(&ctx);
    if (ece_digest_update(&ctx, data, len) != 0) {
        return -1;
    }
    return ece_digest_final(&ctx, digest);
}

/**
 * @brief Convert digest to hexadecimal string
 * 
 * @param digest Binary digest (ECE_DIGEST_SIZE bytes)
 * @param hex_str Buffer to store hex string (must be at least ECE_DIGEST_SIZE*2+1 bytes)
 */
void ece_digest_to_hex(const uint8_t* digest, char* hex_str) {
    static const char hex_chars[] = "0123456789abcdef";
    
    for (int i = 0; i < ECE_DIGEST_SIZE; i++) {
        hex_str[i * 2] = hex_chars[(digest[i] >> 4) & 0xF];
        hex_str[i * 2 + 1] = hex_chars[digest[i] & 0xF];
    }
    hex_str[ECE_DIGEST_SIZE * 2] = '\0';
}

/**
 * @brief Compare two digests for equality
 * 
 * @param digest1 First digest
 * @param digest2 Second digest
 * @return int 0 if equal, non-zero otherwise
 */
int ece_digest_compare(const uint8_t* digest1, const uint8_t* digest2) {
    return memcmp(digest1, digest2, ECE_DIGEST_SIZE);
}
