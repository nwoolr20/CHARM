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
 * @file hmac_charm.c
 * @brief HMAC implementation using CHARM-256
 */

#include "hmac_charm.h"
#include "util.h"
#include "../../algorithm/include/charm.h"
#include <string.h>
#include <stdlib.h>

/**
 * @brief HMAC constants
 */
#define IPAD 0x36
#define OPAD 0x5C

/**
 * @brief One-shot HMAC-CHARM computation
 */
int hmac_charm(const uint8_t* key, size_t key_len,
               const uint8_t* data, size_t data_len,
               uint8_t out[HMAC_CHARM_OUTPUT_SIZE]) {
    
    if (!key || !out || (!data && data_len > 0)) {
        return -1;
    }
    
    uint8_t key_pad[HMAC_CHARM_BLOCK_SIZE];
    uint8_t inner_hash[HMAC_CHARM_OUTPUT_SIZE];
    
    // Prepare key: if longer than block size, hash it first
    if (key_len > HMAC_CHARM_BLOCK_SIZE) {
        if (charm_hash(CHARM_256, key, key_len, key_pad) != 0) {
            return -1;
        }
        // Zero-pad the rest
        memset(key_pad + HMAC_CHARM_OUTPUT_SIZE, 0, 
               HMAC_CHARM_BLOCK_SIZE - HMAC_CHARM_OUTPUT_SIZE);
    } else {
        // Copy key and zero-pad
        memcpy(key_pad, key, key_len);
        memset(key_pad + key_len, 0, HMAC_CHARM_BLOCK_SIZE - key_len);
    }
    
    // Inner hash: CHARM(key XOR ipad || data)
    {
        // Create CHARM context for inner hash
        charm_ctx_t* ctx = malloc(charm_ctx_size());
        if (!ctx) return -1;
        
        charm_params_t params = {
            .version = 1,
            .out_bits = 256,
            .flags = 0,
            .reserved = {0}
        };
        
        if (charm_init(ctx, &params, NULL, 0, NULL, 0) != 0) {
            free(ctx);
            return -1;
        }
        
        // XOR key with ipad and update
        uint8_t ipad_key[HMAC_CHARM_BLOCK_SIZE];
        for (int i = 0; i < HMAC_CHARM_BLOCK_SIZE; i++) {
            ipad_key[i] = key_pad[i] ^ IPAD;
        }
        
        if (charm_update(ctx, ipad_key, HMAC_CHARM_BLOCK_SIZE) != 0) {
            free(ctx);
            return -1;
        }
        
        // Update with data
        if (data_len > 0) {
            if (charm_update(ctx, data, data_len) != 0) {
                free(ctx);
                return -1;
            }
        }
        
        // Finalize inner hash
        if (charm_final(ctx, inner_hash) != 0) {
            free(ctx);
            return -1;
        }
        
        free(ctx);
        util_secure_clear(ipad_key, sizeof(ipad_key));
    }
    
    // Outer hash: CHARM(key XOR opad || inner_hash)
    {
        charm_ctx_t* ctx = malloc(charm_ctx_size());
        if (!ctx) return -1;
        
        charm_params_t params = {
            .version = 1,
            .out_bits = 256,
            .flags = 0,
            .reserved = {0}
        };
        
        if (charm_init(ctx, &params, NULL, 0, NULL, 0) != 0) {
            free(ctx);
            return -1;
        }
        
        // XOR key with opad and update
        uint8_t opad_key[HMAC_CHARM_BLOCK_SIZE];
        for (int i = 0; i < HMAC_CHARM_BLOCK_SIZE; i++) {
            opad_key[i] = key_pad[i] ^ OPAD;
        }
        
        if (charm_update(ctx, opad_key, HMAC_CHARM_BLOCK_SIZE) != 0) {
            free(ctx);
            return -1;
        }
        
        // Update with inner hash
        if (charm_update(ctx, inner_hash, HMAC_CHARM_OUTPUT_SIZE) != 0) {
            free(ctx);
            return -1;
        }
        
        // Finalize outer hash
        if (charm_final(ctx, out) != 0) {
            free(ctx);
            return -1;
        }
        
        free(ctx);
        util_secure_clear(opad_key, sizeof(opad_key));
    }
    
    // Clear sensitive data
    util_secure_clear(key_pad, sizeof(key_pad));
    util_secure_clear(inner_hash, sizeof(inner_hash));
    
    return 0;
}

/**
 * @brief Initialize HMAC-CHARM context
 */
int hmac_charm_init(hmac_charm_ctx_t* ctx, const uint8_t* key, size_t key_len) {
    if (!ctx || !key) return -1;
    
    // Clear context
    memset(ctx, 0, sizeof(hmac_charm_ctx_t));
    
    // Prepare key: if longer than block size, hash it first
    if (key_len > HMAC_CHARM_BLOCK_SIZE) {
        if (charm_hash(CHARM_256, key, key_len, ctx->key_pad) != 0) {
            return -1;
        }
        // Zero-pad the rest
        memset(ctx->key_pad + HMAC_CHARM_OUTPUT_SIZE, 0, 
               HMAC_CHARM_BLOCK_SIZE - HMAC_CHARM_OUTPUT_SIZE);
    } else {
        // Copy key and zero-pad
        memcpy(ctx->key_pad, key, key_len);
        memset(ctx->key_pad + key_len, 0, HMAC_CHARM_BLOCK_SIZE - key_len);
    }
    
    // Allocate and initialize CHARM context for inner hash
    ctx->charm_ctx = malloc(charm_ctx_size());
    if (!ctx->charm_ctx) return -1;
    
    charm_params_t params = {
        .version = 1,
        .out_bits = 256,
        .flags = 0,
        .reserved = {0}
    };
    
    if (charm_init((charm_ctx_t*)ctx->charm_ctx, &params, NULL, 0, NULL, 0) != 0) {
        free(ctx->charm_ctx);
        ctx->charm_ctx = NULL;
        return -1;
    }
    
    // XOR key with ipad and update
    uint8_t ipad_key[HMAC_CHARM_BLOCK_SIZE];
    for (int i = 0; i < HMAC_CHARM_BLOCK_SIZE; i++) {
        ipad_key[i] = ctx->key_pad[i] ^ IPAD;
    }
    
    if (charm_update((charm_ctx_t*)ctx->charm_ctx, ipad_key, HMAC_CHARM_BLOCK_SIZE) != 0) {
        free(ctx->charm_ctx);
        ctx->charm_ctx = NULL;
        util_secure_clear(ipad_key, sizeof(ipad_key));
        return -1;
    }
    
    util_secure_clear(ipad_key, sizeof(ipad_key));
    return 0;
}

/**
 * @brief Update HMAC-CHARM context with data
 */
int hmac_charm_update(hmac_charm_ctx_t* ctx, const uint8_t* data, size_t data_len) {
    if (!ctx || !ctx->charm_ctx || ctx->finalized) return -1;
    if (!data && data_len > 0) return -1;
    
    if (data_len > 0) {
        return charm_update((charm_ctx_t*)ctx->charm_ctx, data, data_len);
    }
    
    return 0;
}

/**
 * @brief Finalize HMAC-CHARM computation
 */
int hmac_charm_final(hmac_charm_ctx_t* ctx, uint8_t out[HMAC_CHARM_OUTPUT_SIZE]) {
    if (!ctx || !ctx->charm_ctx || !out || ctx->finalized) return -1;
    
    uint8_t inner_hash[HMAC_CHARM_OUTPUT_SIZE];
    
    // Finalize inner hash
    if (charm_final((charm_ctx_t*)ctx->charm_ctx, inner_hash) != 0) {
        return -1;
    }
    
    // Create new context for outer hash
    charm_ctx_t* outer_ctx = malloc(charm_ctx_size());
    if (!outer_ctx) return -1;
    
    charm_params_t params = {
        .version = 1,
        .out_bits = 256,
        .flags = 0,
        .reserved = {0}
    };
    
    if (charm_init(outer_ctx, &params, NULL, 0, NULL, 0) != 0) {
        free(outer_ctx);
        return -1;
    }
    
    // XOR key with opad and update
    uint8_t opad_key[HMAC_CHARM_BLOCK_SIZE];
    for (int i = 0; i < HMAC_CHARM_BLOCK_SIZE; i++) {
        opad_key[i] = ctx->key_pad[i] ^ OPAD;
    }
    
    if (charm_update(outer_ctx, opad_key, HMAC_CHARM_BLOCK_SIZE) != 0) {
        free(outer_ctx);
        util_secure_clear(opad_key, sizeof(opad_key));
        return -1;
    }
    
    // Update with inner hash
    if (charm_update(outer_ctx, inner_hash, HMAC_CHARM_OUTPUT_SIZE) != 0) {
        free(outer_ctx);
        util_secure_clear(opad_key, sizeof(opad_key));
        return -1;
    }
    
    // Finalize outer hash
    if (charm_final(outer_ctx, out) != 0) {
        free(outer_ctx);
        util_secure_clear(opad_key, sizeof(opad_key));
        return -1;
    }
    
    free(outer_ctx);
    util_secure_clear(opad_key, sizeof(opad_key));
    util_secure_clear(inner_hash, sizeof(inner_hash));
    
    ctx->finalized = 1;
    return 0;
}

/**
 * @brief Clear HMAC-CHARM context
 */
void hmac_charm_clear(hmac_charm_ctx_t* ctx) {
    if (!ctx) return;
    
    if (ctx->charm_ctx) {
        free(ctx->charm_ctx);
        ctx->charm_ctx = NULL;
    }
    
    util_secure_clear(ctx->key_pad, sizeof(ctx->key_pad));
    ctx->finalized = 0;
}