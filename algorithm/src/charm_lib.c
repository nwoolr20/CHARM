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
 * @file charm_lib.c
 * @brief CHARM Algorithm Library - Implementation Only (No Main)
 * 
 * This file contains the CHARM algorithm implementation without the main function
 * for linking with test harnesses and other applications.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "charm.h"

/**
 * @brief CHARM algorithm context structure
 */
struct charm_ctx {
    uint8_t state[128];           // Internal state (fixed size)
    uint64_t counter;             // Block counter
    uint64_t total_len;           // Total input length in bytes
    charm_params_t params;        // Algorithm parameters
    uint8_t buffer[CHARM_BLOCK_BYTES]; // Input buffer
    size_t buffer_len;            // Buffered bytes
    uint8_t finalized;            // Finalization flag
};

/**
 * @brief CHARM-256 Initialization Vector
 */
static const uint64_t CHARM256_IV[4] = {
    0x6A09E667F3BCC908ULL, 0xBB67AE8584CAA73BULL,
    0x3C6EF372FE94F82BULL, 0xA54FF53A5F1D36F1ULL
};

/**
 * @brief CHARM-384 Initialization Vector  
 */
static const uint64_t CHARM384_IV[6] = {
    0xCBBB9D5DC1059ED8ULL, 0x629A292A367CD507ULL,
    0x9159015A3070DD17ULL, 0x152FECD8F70E5939ULL,
    0x67332667FFC00B31ULL, 0x8EB44A8768581511ULL
};

/**
 * @brief CHARM-512 Initialization Vector
 */
static const uint64_t CHARM512_IV[8] = {
    0x6A09E667F3BCC908ULL, 0xBB67AE8584CAA73BULL,
    0x3C6EF372FE94F82BULL, 0xA54FF53A5F1D36F1ULL,
    0x510E527FADE682D1ULL, 0x9B05688C2B3E6C1FULL,
    0x1F83D9ABFB41BD6BULL, 0x5BE0CD19137E2179ULL
};

/**
 * @brief Round constants for mixing
 */
static const uint64_t ROUND_CONSTANTS[CHARM_MAX_ROUNDS] = {
    0x428A2F98D728AE22ULL, 0x7137449123EF65CDULL, 0xB5C0FBCFEC4D3B2FULL,
    0xE9B5DBA58189DBBCULL, 0x3956C25BF348B538ULL, 0x59F111F1B605D019ULL,
    0x923F82A4AF194F9BULL, 0xAB1C5ED5DA6D8118ULL, 0xD807AA98A3030242ULL,
    0x12835B0145706FBEULL, 0x243185BE4EE4B28CULL, 0x550C7DC3D5FFB4E2ULL
};

/**
 * @brief Load 64-bit word from byte array (little-endian)
 */
static uint64_t load_u64_le(const uint8_t* data) {
    return ((uint64_t)data[0] << 0)  | ((uint64_t)data[1] << 8)  |
           ((uint64_t)data[2] << 16) | ((uint64_t)data[3] << 24) |
           ((uint64_t)data[4] << 32) | ((uint64_t)data[5] << 40) |
           ((uint64_t)data[6] << 48) | ((uint64_t)data[7] << 56);
}

/**
 * @brief Store 64-bit word to byte array (little-endian)
 */
static void store_u64_le(uint8_t* data, uint64_t value) {
    data[0] = (uint8_t)(value >> 0);
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)(value >> 16);
    data[3] = (uint8_t)(value >> 24);
    data[4] = (uint8_t)(value >> 32);
    data[5] = (uint8_t)(value >> 40);
    data[6] = (uint8_t)(value >> 48);
    data[7] = (uint8_t)(value >> 56);
}

/**
 * @brief Rotate left 64-bit value
 */
static uint64_t rotl64(uint64_t x, int n) {
    return (x << n) | (x >> (64 - n));
}

/**
 * @brief Simple entropy estimation for CAEDS
 */
static double estimate_entropy(const uint8_t* block, size_t len) {
    int freq[256] = {0};
    
    // Count byte frequencies
    for (size_t i = 0; i < len; i++) {
        freq[block[i]]++;
    }
    
    // Calculate Shannon entropy
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            double p = (double)freq[i] / len;
            entropy -= p * log2(p);
        }
    }
    
    return entropy / 8.0; // Normalize to [0,1]
}

/**
 * @brief CAEDS round scheduling (deterministic)
 */
static int caeds_schedule_rounds(const uint8_t* block, size_t len, uint64_t position) {
    double entropy = estimate_entropy(block, len);
    
    // Position-based modulation (prime modulus for distribution)
    double position_factor = (double)(position % 251) / 251.0;
    
    // Adaptive round calculation
    double entropy_factor = 1.0 - entropy; // More rounds for low entropy
    int bonus_rounds = (int)(entropy_factor * (CHARM_MAX_ROUNDS - CHARM_MIN_ROUNDS));
    
    int total_rounds = CHARM_MIN_ROUNDS + bonus_rounds;
    
    // Position modulation (±1 round variation)
    if (position_factor > 0.5) {
        total_rounds = (total_rounds < CHARM_MAX_ROUNDS) ? total_rounds + 1 : total_rounds;
    }
    
    return (total_rounds > CHARM_MAX_ROUNDS) ? CHARM_MAX_ROUNDS : total_rounds;
}

/**
 * @brief CEE mixing round
 */
static void cee_mix_round(uint8_t* state, int round_num) {
    // ARX operations on 64-bit words
    for (int i = 0; i < 16; i += 2) { // Process pairs of 64-bit words
        uint64_t a = load_u64_le(state + i * 8);
        uint64_t b = load_u64_le(state + (i + 1) * 8);
        
        // Add with round constant
        a = a + b + ROUND_CONSTANTS[round_num % CHARM_MAX_ROUNDS];
        
        // Rotate (varying per round and position)
        int rot = 13 + (round_num * 2) + (i % 7);
        a = rotl64(a, rot % 64);
        
        // XOR with position-dependent value
        a ^= (uint64_t)i * ROUND_CONSTANTS[(round_num + i) % CHARM_MAX_ROUNDS];
        
        store_u64_le(state + i * 8, a);
    }
    
    // Cross-lane shuffle for diffusion
    uint8_t temp[8];
    for (int i = 0; i < 8; i++) {
        int src_idx = (i * 17 + round_num) % 16; // Prime-based shuffle
        memcpy(temp, state + i * 8, 8);
        memcpy(state + i * 8, state + src_idx * 8, 8);
        memcpy(state + src_idx * 8, temp, 8);
    }
}

/**
 * @brief ECE entropic collapse 
 */
static void ece_collapse(uint8_t* state, double entropy_level) {
    int collapse_rounds = (int)((1.0 - entropy_level) * 4) + 2; // 2-6 rounds
    
    for (int round = 0; round < collapse_rounds; round++) {
        for (int i = 0; i < 16; i++) {
            uint64_t word = load_u64_le(state + i * 8);
            
            // Maximum avalanche transformation
            word = word ^ rotl64(word, 13) ^ rotl64(word, 35);
            word = word * ROUND_CONSTANTS[round % CHARM_MAX_ROUNDS];
            word = word ^ (word >> 17) ^ (word >> 31);
            
            store_u64_le(state + i * 8, word);
        }
        
        // Full permutation
        for (int i = 0; i < 8; i++) {
            int j = (i + round + 1) % 16;
            uint64_t temp = load_u64_le(state + i * 8);
            store_u64_le(state + i * 8, load_u64_le(state + j * 8));
            store_u64_le(state + j * 8, temp);
        }
    }
}

/**
 * @brief Process a single block through CAEDS/CEE/ECE
 */
static void process_block(charm_ctx_t* ctx, const uint8_t* block) {
    // CAEDS: Adaptive entropy detection and round scheduling
    int rounds = caeds_schedule_rounds(block, CHARM_BLOCK_BYTES, ctx->counter);
    double entropy = estimate_entropy(block, CHARM_BLOCK_BYTES);
    
    // Absorb block into state (XOR)
    for (int i = 0; i < CHARM_BLOCK_BYTES && i < 128; i++) {
        ctx->state[i] ^= block[i];
    }
    
    // CEE: Controlled entropy evolution
    for (int round = 0; round < rounds; round++) {
        cee_mix_round(ctx->state, round);
    }
    
    // ECE: Periodic entropic collapse (every 4 blocks or low entropy)
    if ((ctx->counter % 4 == 3) || (entropy < 0.5)) {
        ece_collapse(ctx->state, entropy);
    }
    
    ctx->counter++;
}

/**
 * @brief Initialize CHARM context
 */
int charm_init(charm_ctx_t* ctx, 
               const charm_params_t* params,
               const uint8_t* key, size_t key_len,
               const uint8_t* custom, size_t custom_len) {
    if (!ctx || !params) return -1;
    
    // Validate parameters
    if (params->version != 1) return -1;
    if (params->out_bits != 256 && params->out_bits != 384 && params->out_bits != 512) return -1;
    if (key_len > 0 && key_len != 32) return -1;
    
    // Clear context
    memset(ctx, 0, sizeof(charm_ctx_t));
    
    // Copy parameters
    ctx->params = *params;
    
    // Initialize state with appropriate IV
    const uint64_t* iv;
    size_t iv_words;
    
    switch (params->out_bits) {
        case 256:
            iv = CHARM256_IV;
            iv_words = 4;
            break;
        case 384:
            iv = CHARM384_IV;
            iv_words = 6;
            break;
        case 512:
            iv = CHARM512_IV;
            iv_words = 8;
            break;
        default:
            return -1;
    }
    
    // Copy IV to state
    for (size_t i = 0; i < iv_words; i++) {
        store_u64_le(ctx->state + i * 8, iv[i]);
    }
    
    // Process key if provided (keyed mode)
    if (key && key_len > 0) {
        uint8_t key_block[CHARM_BLOCK_BYTES] = {0};
        memcpy(key_block, key, key_len);
        key_block[key_len] = 0x01; // Domain separation
        process_block(ctx, key_block);
    }
    
    // Process customization if provided
    if (custom && custom_len > 0) {
        // For simplicity, hash customization string into a block
        uint8_t custom_block[CHARM_BLOCK_BYTES] = {0};
        size_t copy_len = (custom_len < CHARM_BLOCK_BYTES - 1) ? custom_len : CHARM_BLOCK_BYTES - 1;
        memcpy(custom_block, custom, copy_len);
        custom_block[CHARM_BLOCK_BYTES - 1] = 0x02; // Domain separation
        process_block(ctx, custom_block);
    }
    
    return 0;
}

/**
 * @brief Update CHARM context with data
 */
int charm_update(charm_ctx_t* ctx, const void* data, size_t len) {
    if (!ctx || ctx->finalized) return -1;
    if (!data && len > 0) return -1;
    
    const uint8_t* input = (const uint8_t*)data;
    ctx->total_len += len;
    
    while (len > 0) {
        size_t available = CHARM_BLOCK_BYTES - ctx->buffer_len;
        size_t to_copy = (len < available) ? len : available;
        
        memcpy(ctx->buffer + ctx->buffer_len, input, to_copy);
        ctx->buffer_len += to_copy;
        input += to_copy;
        len -= to_copy;
        
        if (ctx->buffer_len == CHARM_BLOCK_BYTES) {
            process_block(ctx, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
    
    return 0;
}

/**
 * @brief Finalize CHARM computation
 */
int charm_final(charm_ctx_t* ctx, uint8_t* out) {
    if (!ctx || !out || ctx->finalized) return -1;
    
    // Merkle-Damgård style padding
    ctx->buffer[ctx->buffer_len] = 0x80; // Append '1' bit
    ctx->buffer_len++;
    
    // Pad with zeros
    if (ctx->buffer_len > CHARM_BLOCK_BYTES - 8) {
        // Need extra block for length
        memset(ctx->buffer + ctx->buffer_len, 0, CHARM_BLOCK_BYTES - ctx->buffer_len);
        process_block(ctx, ctx->buffer);
        memset(ctx->buffer, 0, CHARM_BLOCK_BYTES);
        ctx->buffer_len = 0;
    }
    
    // Pad to leave room for length
    memset(ctx->buffer + ctx->buffer_len, 0, CHARM_BLOCK_BYTES - 8 - ctx->buffer_len);
    
    // Append length in bits (big-endian)
    uint64_t bit_len = ctx->total_len * 8;
    for (int i = 7; i >= 0; i--) {
        ctx->buffer[CHARM_BLOCK_BYTES - 8 + i] = (uint8_t)(bit_len >> (i * 8));
    }
    
    // Process final block
    process_block(ctx, ctx->buffer);
    
    // Final collapse for output generation
    ece_collapse(ctx->state, 1.0); // Maximum collapse for finalization
    
    // Extract output
    size_t output_bytes = ctx->params.out_bits / 8;
    memcpy(out, ctx->state, output_bytes);
    
    ctx->finalized = 1;
    return 0;
}

/**
 * @brief One-shot CHARM hash computation
 */
int charm_hash(charm_variant_t variant, const void* data, size_t len, uint8_t* out) {
    charm_params_t params = {
        .version = 1,
        .out_bits = (uint16_t)variant,
        .flags = 0,
        .reserved = {0}
    };
    
    charm_ctx_t ctx;
    int ret = charm_init(&ctx, &params, NULL, 0, NULL, 0);
    if (ret != 0) return ret;
    
    ret = charm_update(&ctx, data, len);
    if (ret != 0) return ret;
    
    return charm_final(&ctx, out);
}

/**
 * @brief Get context size for allocation
 */
size_t charm_ctx_size(void) {
    return sizeof(charm_ctx_t);
}

/**
 * @brief Get algorithm version string
 */
const char* charm_version(void) {
    return CHARM_VERSION_STRING;
}