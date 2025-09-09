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

#ifndef CHARM_API_H
#define CHARM_API_H

/**
 * @file charm_api.h
 * @brief Main API header for the CHARM v2.0 system
 * 
 * This header defines the public API for the CHARM v2.0 system,
 * providing access to all major subsystems: CAEDS, CEE, and ECE.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System states
 */
typedef enum {
    CHARM_STATE_INIT,      // System initializing
    CHARM_STATE_RUN,       // Normal operation
    CHARM_STATE_DEGRADED,  // Degraded entropy quality
    CHARM_STATE_FAILOVER   // Using fallback entropy sources
} charm_system_state_t;

/**
 * @brief Initialize the CHARM system
 * 
 * @return int 0 on success, non-zero on failure
 */
int charm_init(void);

/**
 * @brief Shutdown the CHARM system
 * 
 * @return int 0 on success, non-zero on failure
 */
int charm_shutdown(void);

/**
 * @brief Get the current system state
 * 
 * @return charm_system_state_t Current system state
 */
charm_system_state_t charm_get_state(void);

/**
 * @brief Convert system state to string
 * 
 * @param state System state
 * @return const char* String representation of state
 */
const char* charm_state_to_string(charm_system_state_t state);

/**
 * @brief Compute digest of data
 * 
 * @param data Input data
 * @param len Length of input data in bytes
 * @param digest Buffer to store the digest (must be at least 32 bytes)
 * @return int 0 on success, non-zero on failure
 */
int charm_digest_compute(const void* data, size_t len, uint8_t* digest);

/**
 * @brief Convert binary digest to hexadecimal string
 * 
 * @param digest Binary digest (32 bytes)
 * @param hex_str Buffer to store hex string (must be at least 65 bytes)
 */
void charm_digest_to_hex(const uint8_t* digest, char* hex_str);

/**
 * @brief Get current entropy quality
 * 
 * @return double Quality value between 0.0 (poor) and 1.0 (excellent)
 */
double charm_get_entropy_quality(void);

/**
 * @brief Feed external entropy into the system
 * 
 * @param data External entropy data
 * @param len Length of data in bytes
 * @return int 0 on success, non-zero on failure
 */
int charm_feed_entropy(const void* data, size_t len);

/**
 * @brief Start entropy tracing
 * 
 * @return int 0 on success, non-zero on failure
 */
int charm_trace_start(void);

/**
 * @brief Stop entropy tracing
 * 
 * @return int 0 on success, non-zero on failure
 */
int charm_trace_stop(void);

/**
 * @brief Force system state (for testing)
 * 
 * @param state State to force
 * @return int 0 on success, non-zero on failure
 */
int charm_force_state(charm_system_state_t state);

/**
 * @brief Get library version
 * 
 * @return const char* Version string
 */
const char* charm_get_version(void);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_API_H */
