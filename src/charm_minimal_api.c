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


// SPDX-License-Identifier: LicenseRef-CHARM-2025
// SPDX-FileCopyrightText: Copyright (c) 2025 Nicholas Woolridge & NOCTRL™ (Nô)

/**
 * @file charm_minimal_api.c
 * @brief Minimal CHARM API Implementation for Security Suite
 */

#include "charm_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

// Simple state
static int g_charm_initialized = 0;
static charm_system_state_t g_charm_state = CHARM_STATE_INIT;

// Initialize CHARM system
int charm_init(void) {
    if (g_charm_initialized) {
        return 0;
    }
    
    g_charm_state = CHARM_STATE_RUN;
    g_charm_initialized = 1;
    return 0;
}

// Shutdown CHARM system
int charm_shutdown(void) {
    if (!g_charm_initialized) {
        return 0;
    }
    
    g_charm_state = CHARM_STATE_INIT;
    g_charm_initialized = 0;
    return 0;
}

// Get current system state
charm_system_state_t charm_get_state(void) {
    return g_charm_state;
}

// Simple hash function using a basic algorithm
static void simple_hash(const uint8_t* data, size_t len, uint8_t* output) {
    // Very simple hash - not cryptographically secure, just for demo
    uint32_t hash = 0x811c9dc5; // FNV offset basis
    
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x01000193; // FNV prime
    }
    
    // Expand hash to 32 bytes
    for (int i = 0; i < 32; i += 4) {
        output[i] = (hash >> 24) & 0xFF;
        output[i+1] = (hash >> 16) & 0xFF;
        output[i+2] = (hash >> 8) & 0xFF;
        output[i+3] = hash & 0xFF;
        hash = hash * 0x01000193 + i; // Mix for next bytes
    }
}

// Compute digest of data
int charm_digest_compute(const void* data, size_t len, uint8_t* digest) {
    if (!data || !digest) {
        return -1;
    }
    
    simple_hash((const uint8_t*)data, len, digest);
    return 0;
}

// Convert binary digest to hexadecimal string
void charm_digest_to_hex(const uint8_t* digest, char* hex_str) {
    if (!digest || !hex_str) {
        return;
    }
    
    for (int i = 0; i < 32; i++) {
        snprintf(hex_str + (i * 2), 3, "%02x", digest[i]);
    }
    hex_str[64] = '\0';
}

// Get current entropy quality (dummy implementation)
double charm_get_entropy_quality(void) {
    // Return a random quality between 0.7 and 1.0
    return 0.7 + (rand() % 100) / 333.0;
}

// Feed external entropy into the system (dummy implementation)
int charm_feed_entropy(const void* data, size_t len) {
    // Mix provided entropy data into the random state
    if (data && len > 0) {
        const uint8_t* bytes = (const uint8_t*)data;
        unsigned int mixed_seed = (unsigned int)time(NULL);
        
        // Mix in the provided data bytes
        for (size_t i = 0; i < len; i++) {
            mixed_seed ^= (bytes[i] << ((i % 4) * 8));
            mixed_seed = mixed_seed * 1103515245 + 12345; // LCG mixing
        }
        
        srand(mixed_seed);
    } else {
        // Fallback to time-based seeding
        srand(time(NULL));
    }
    return 0;
}

// Get library version
const char* charm_get_version(void) {
    return "CHARM Security Suite Demo v1.0.0";
}

// Convert system state to string
const char* charm_state_to_string(charm_system_state_t state) {
    switch (state) {
        case CHARM_STATE_INIT: return "INIT";
        case CHARM_STATE_RUN: return "RUN";
        case CHARM_STATE_DEGRADED: return "DEGRADED";
        case CHARM_STATE_FAILOVER: return "FAILOVER";
        default: return "UNKNOWN";
    }
}