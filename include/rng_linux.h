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
 * @file rng_linux.h
 * @brief Linux-specific random number generation for the CHARM system
 * 
 * This header defines the Linux-specific random number generation interfaces
 * for the CHARM system, providing access to hardware and kernel entropy sources.
 */

#ifndef RNG_LINUX_H
#define RNG_LINUX_H

#include "charm_status.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Linux RNG source types
 */
typedef enum {
    RNG_LINUX_SOURCE_URANDOM = 0,    /**< /dev/urandom */
    RNG_LINUX_SOURCE_RANDOM = 1,     /**< /dev/random */
    RNG_LINUX_SOURCE_HWRNG = 2,      /**< Hardware RNG (/dev/hwrng) */
    RNG_LINUX_SOURCE_RDRAND = 3,     /**< Intel RDRAND instruction */
    RNG_LINUX_SOURCE_RDSEED = 4,     /**< Intel RDSEED instruction */
    RNG_LINUX_SOURCE_JITTER = 5,     /**< CPU jitter entropy */
    RNG_LINUX_SOURCE_COUNT = 6       /**< Number of sources */
} rng_linux_source_t;

/**
 * @brief Linux RNG source information
 */
typedef struct {
    bool available;                  /**< Source availability */
    bool active;                     /**< Source active status */
    double quality;                  /**< Entropy quality (0.0-1.0) */
    uint64_t bytes_read;             /**< Total bytes read */
    uint64_t error_count;            /**< Error count */
} rng_linux_source_info_t;

/**
 * @brief Linux RNG configuration
 */
typedef struct {
    bool use_urandom;                /**< Use /dev/urandom */
    bool use_random;                 /**< Use /dev/random */
    bool use_hwrng;                  /**< Use hardware RNG */
    bool use_rdrand;                 /**< Use RDRAND instruction */
    bool use_rdseed;                 /**< Use RDSEED instruction */
    bool use_jitter;                 /**< Use CPU jitter entropy */
    bool block_on_insufficient;      /**< Block when insufficient entropy */
    uint32_t timeout_ms;             /**< Operation timeout in milliseconds */
} rng_linux_config_t;

/**
 * @brief Linux RNG statistics
 */
typedef struct {
    uint64_t bytes_read;             /**< Total bytes read */
    uint64_t urandom_bytes;          /**< Bytes from /dev/urandom */
    uint64_t random_bytes;           /**< Bytes from /dev/random */
    uint64_t hwrng_bytes;            /**< Bytes from hardware RNG */
    uint64_t rdrand_bytes;           /**< Bytes from RDRAND */
    uint64_t rdseed_bytes;           /**< Bytes from RDSEED */
    uint64_t jitter_bytes;           /**< Bytes from CPU jitter */
    uint64_t error_count;            /**< Error count */
} rng_linux_stats_t;

/**
 * @brief Linux RNG context handle
 */
typedef struct rng_linux_context* rng_linux_handle_t;

/**
 * @brief Default Linux RNG configuration
 */
extern const rng_linux_config_t RNG_LINUX_DEFAULT_CONFIG;

/**
 * @brief Initialize Linux RNG
 * 
 * @param config Configuration (NULL for default)
 * @return rng_linux_handle_t Handle or NULL on failure
 */
rng_linux_handle_t rng_linux_init(const rng_linux_config_t* config);

/**
 * @brief Shutdown Linux RNG
 * 
 * @param handle RNG handle
 */
void rng_linux_shutdown(rng_linux_handle_t handle);

/**
 * @brief Get random bytes from Linux RNG
 * 
 * @param handle RNG handle
 * @param buffer Output buffer
 * @param size Buffer size in bytes
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_get_bytes(rng_linux_handle_t handle, uint8_t* buffer, size_t size);

/**
 * @brief Get random bytes from specific source
 * 
 * @param handle RNG handle
 * @param source Source type
 * @param buffer Output buffer
 * @param size Buffer size in bytes
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_get_bytes_from_source(rng_linux_handle_t handle, rng_linux_source_t source, uint8_t* buffer, size_t size);

/**
 * @brief Get source information
 * 
 * @param handle RNG handle
 * @param source Source type
 * @param info Source information structure
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_get_source_info(rng_linux_handle_t handle, rng_linux_source_t source, rng_linux_source_info_t* info);

/**
 * @brief Activate source
 * 
 * @param handle RNG handle
 * @param source Source type
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_activate_source(rng_linux_handle_t handle, rng_linux_source_t source);

/**
 * @brief Deactivate source
 * 
 * @param handle RNG handle
 * @param source Source type
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_deactivate_source(rng_linux_handle_t handle, rng_linux_source_t source);

/**
 * @brief Get RNG statistics
 * 
 * @param handle RNG handle
 * @param stats Statistics structure
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_get_stats(rng_linux_handle_t handle, rng_linux_stats_t* stats);

/**
 * @brief Check if entropy is available
 * 
 * @param handle RNG handle
 * @param available Pointer to store availability
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_is_entropy_available(rng_linux_handle_t handle, bool* available);

/**
 * @brief Get entropy pool size
 * 
 * @param handle RNG handle
 * @param size Pointer to store size
 * @return charm_status_t Status code
 */
charm_status_t rng_linux_get_entropy_pool_size(rng_linux_handle_t handle, uint32_t* size);

/**
 * @brief Get string representation of source
 * 
 * @param source Source type
 * @return const char* String representation
 */
const char* rng_linux_source_to_string(rng_linux_source_t source);

/**
 * @brief Check if RDRAND is supported
 * 
 * @return bool True if supported
 */
bool rng_linux_is_rdrand_supported(void);

/**
 * @brief Check if RDSEED is supported
 * 
 * @return bool True if supported
 */
bool rng_linux_is_rdseed_supported(void);

#ifdef __cplusplus
}
#endif

#endif /* RNG_LINUX_H */
