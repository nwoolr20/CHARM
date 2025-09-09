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
 * @file caeds_flux.h
 * @brief Entropy Flux Acquisition for CAEDS
 * 
 * This header defines the interfaces for the Entropy Flux Acquisition
 * component of the CAEDS subsystem, responsible for collecting entropy
 * from multiple sources with high fidelity and resilience.
 */

#ifndef CAEDS_FLUX_H
#define CAEDS_FLUX_H

#include "charm_status.h"
#include "charm_types.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Entropy source types
 */
typedef enum {
    CAEDS_FLUX_SOURCE_TIMESTAMP_JITTER = 0,  /**< High-resolution timestamp jitter */
    CAEDS_FLUX_SOURCE_CPU_LATENCY = 1,       /**< CPU loop latency drift */
    CAEDS_FLUX_SOURCE_THERMAL = 2,           /**< Thermal/power variance */
    CAEDS_FLUX_SOURCE_NETWORK = 3,           /**< Network jitter */
    CAEDS_FLUX_SOURCE_URANDOM = 4,           /**< /dev/urandom */
    CAEDS_FLUX_SOURCE_RDRAND = 5,            /**< RDRAND instruction */
    CAEDS_FLUX_SOURCE_COUNT = 6              /**< Number of entropy sources */
} caeds_flux_source_t;

/**
 * @brief Entropy source configuration
 */
typedef struct {
    bool enabled;                /**< Whether this source is enabled */
    uint32_t weight;             /**< Source weight (0-100) */
    uint32_t batch_size;         /**< Batch size in bytes */
    uint32_t sampling_interval;  /**< Sampling interval in microseconds */
} caeds_flux_source_config_t;

/**
 * @brief Entropy flux configuration
 */
typedef struct {
    caeds_flux_source_config_t sources[CAEDS_FLUX_SOURCE_COUNT]; /**< Source configurations */
    uint32_t buffer_size;        /**< Buffer size in bytes */
    bool non_blocking;           /**< Whether sampling should be non-blocking */
    bool fingerprint_rdrand;     /**< Whether to fingerprint RDRAND */
    uint32_t min_entropy_bits;   /**< Minimum entropy bits per byte */
} caeds_flux_config_t;

/**
 * @brief Entropy source volatility metrics
 */
typedef struct {
    double mean;                 /**< Mean value */
    double variance;             /**< Variance */
    double min;                  /**< Minimum value */
    double max;                  /**< Maximum value */
    double entropy_bits;         /**< Estimated entropy bits per byte */
    uint32_t sample_count;       /**< Number of samples */
} caeds_flux_volatility_t;

/**
 * @brief Entropy flux context
 */
typedef struct caeds_flux_context_s* caeds_flux_context_t;

/**
 * @brief Default configuration for entropy flux
 */
extern const caeds_flux_config_t CAEDS_FLUX_DEFAULT_CONFIG;

/**
 * @brief Initialize entropy flux
 * 
 * @param context Pointer to receive context handle
 * @param config Configuration, or NULL for default
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_init(caeds_flux_context_t* context, const caeds_flux_config_t* config);

/**
 * @brief Collect entropy from all enabled sources
 * 
 * @param context Context handle
 * @param buffer Buffer to receive entropy
 * @param size Buffer size
 * @param collected Pointer to receive number of bytes collected
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_collect(caeds_flux_context_t context, 
                                 uint8_t* buffer, 
                                 size_t size, 
                                 size_t* collected);

/**
 * @brief Collect entropy from a specific source
 * 
 * @param context Context handle
 * @param source Source type
 * @param buffer Buffer to receive entropy
 * @param size Buffer size
 * @param collected Pointer to receive number of bytes collected
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_collect_from_source(caeds_flux_context_t context,
                                            caeds_flux_source_t source,
                                            uint8_t* buffer,
                                            size_t size,
                                            size_t* collected);

/**
 * @brief Get volatility metrics for a specific source
 * 
 * @param context Context handle
 * @param source Source type
 * @param volatility Pointer to receive volatility metrics
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_get_volatility(caeds_flux_context_t context,
                                       caeds_flux_source_t source,
                                       caeds_flux_volatility_t* volatility);

/**
 * @brief Enable or disable a specific source
 * 
 * @param context Context handle
 * @param source Source type
 * @param enabled Whether to enable or disable
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_set_source_enabled(caeds_flux_context_t context,
                                           caeds_flux_source_t source,
                                           bool enabled);

/**
 * @brief Set weight for a specific source
 * 
 * @param context Context handle
 * @param source Source type
 * @param weight Source weight (0-100)
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_set_source_weight(caeds_flux_context_t context,
                                          caeds_flux_source_t source,
                                          uint32_t weight);

/**
 * @brief Check if a specific source is available
 * 
 * @param context Context handle
 * @param source Source type
 * @param available Pointer to receive availability
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_is_source_available(caeds_flux_context_t context,
                                            caeds_flux_source_t source,
                                            bool* available);

/**
 * @brief Get source name
 * 
 * @param source Source type
 * @return const char* Source name
 */
const char* caeds_flux_get_source_name(caeds_flux_source_t source);

/**
 * @brief Shutdown entropy flux
 * 
 * @param context Context handle
 * @return charm_status_t Status code
 */
charm_status_t caeds_flux_shutdown(caeds_flux_context_t context);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_FLUX_H */
