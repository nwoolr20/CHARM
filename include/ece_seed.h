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
 * @file ece_seed.h
 * @brief Seed generation and management for the Entropic Collapse Engine
 * 
 * This header defines the seed generation and management functionality for the ECE
 * (Entropic Collapse Engine) subsystem, providing mechanisms for creating and
 * managing high-quality entropy seeds for the collapse function.
 */

#ifndef ECE_SEED_H
#define ECE_SEED_H

#include "ece_core.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECE seed handle
 */
typedef struct ece_seed* ece_seed_handle_t;

/**
 * @brief Seed source types
 */
typedef enum {
    ECE_SEED_SOURCE_SYSTEM = 0,    /**< System entropy source (/dev/urandom) */
    ECE_SEED_SOURCE_HARDWARE = 1,  /**< Hardware entropy source (RDRAND) */
    ECE_SEED_SOURCE_TIMER = 2,     /**< High-resolution timer jitter */
    ECE_SEED_SOURCE_CUSTOM = 3,    /**< Custom entropy source */
    ECE_SEED_SOURCE_MIXED = 4      /**< Mixed entropy sources */
} ece_seed_source_t;

/**
 * @brief Seed configuration structure
 */
typedef struct {
    ece_seed_source_t source;      /**< Seed source type */
    size_t seed_size;              /**< Seed size in bytes (0 for default) */
    uint32_t mixing_rounds;        /**< Number of mixing rounds (0 for default) */
    bool persistent;               /**< Whether to persist seed across sessions */
    const char* custom_source;     /**< Custom source path (for ECE_SEED_SOURCE_CUSTOM) */
} ece_seed_config_t;

/**
 * @brief Seed quality metrics
 */
typedef struct {
    double entropy_bits_per_byte;  /**< Estimated entropy bits per byte (0.0-8.0) */
    double uniformity;             /**< Uniformity score (0.0-1.0) */
    double independence;           /**< Independence score (0.0-1.0) */
    uint32_t effective_size;       /**< Effective seed size in bits */
} ece_seed_quality_t;

/**
 * @brief Initialize a seed context
 * 
 * @param config Configuration structure (NULL for defaults)
 * @return ece_seed_handle_t Seed handle or NULL on failure
 */
ece_seed_handle_t ece_seed_init(const ece_seed_config_t* config);

/**
 * @brief Shutdown the seed context and free resources
 * 
 * @param handle Seed handle
 */
void ece_seed_shutdown(ece_seed_handle_t handle);

/**
 * @brief Generate a new seed
 * 
 * @param handle Seed handle
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_generate(ece_seed_handle_t handle);

/**
 * @brief Get the current seed
 * 
 * @param handle Seed handle
 * @param seed Output buffer for seed
 * @param size Size of seed buffer in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_get(ece_seed_handle_t handle, uint8_t* seed, size_t size);

/**
 * @brief Set a custom seed
 * 
 * @param handle Seed handle
 * @param seed Input seed data
 * @param size Size of seed data in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_set(ece_seed_handle_t handle, const uint8_t* seed, size_t size);

/**
 * @brief Mix additional entropy into the seed
 * 
 * @param handle Seed handle
 * @param data Additional entropy data
 * @param size Size of data in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_mix(ece_seed_handle_t handle, const uint8_t* data, size_t size);

/**
 * @brief Evaluate seed quality
 * 
 * @param handle Seed handle
 * @param quality Quality metrics structure to fill
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_evaluate(ece_seed_handle_t handle, ece_seed_quality_t* quality);

/**
 * @brief Save seed to a file
 * 
 * @param handle Seed handle
 * @param filename Output filename
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_save(ece_seed_handle_t handle, const char* filename);

/**
 * @brief Load seed from a file
 * 
 * @param handle Seed handle
 * @param filename Input filename
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_load(ece_seed_handle_t handle, const char* filename);

/**
 * @brief Apply seed to an ECE context
 * 
 * @param handle Seed handle
 * @param ece_handle ECE context handle
 * @return ece_status_t Status code
 */
ece_status_t ece_seed_apply(ece_seed_handle_t handle, ece_handle_t ece_handle);

/**
 * @brief Get string representation of seed source
 * 
 * @param source Seed source type
 * @return const char* String representation
 */
const char* ece_seed_source_to_string(ece_seed_source_t source);

#ifdef __cplusplus
}
#endif

#endif /* ECE_SEED_H */
