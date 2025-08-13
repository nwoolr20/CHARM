/**
 * @file caeds_efm.h
 * @brief Entropy Flux Monitor for CAEDS
 * 
 * This header defines the interfaces for the Entropy Flux Monitor (EFM)
 * component of the CAEDS subsystem, responsible for real-time monitoring
 * of entropy sources and quality assessment.
 */

#ifndef CAEDS_EFM_H
#define CAEDS_EFM_H

#include "core/charm.h"
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief EFM status codes
 */
typedef enum {
    CHARM_EFM_SUCCESS = 0,
    CHARM_EFM_ERROR_INITIALIZATION = -1,
    CHARM_EFM_ERROR_INVALID_PARAMETER = -2,
    CHARM_EFM_ERROR_HARDWARE_FAILURE = -3,
    CHARM_EFM_ERROR_INSUFFICIENT_ENTROPY = -4,
    CHARM_EFM_ERROR_NOT_INITIALIZED = -5
} charm_efm_status_t;

/**
 * @brief Entropy source details
 */
typedef struct {
    charm_entropy_source_t type;      /**< Source type */
    uint32_t quality;                 /**< Current quality (0-100) */
    uint64_t bytes_collected;         /**< Total bytes collected */
    uint64_t entropy_bits;            /**< Estimated entropy bits */
    time_t last_sample_time;          /**< Last sample timestamp */
    bool available;                   /**< Whether source is available */
    char name[32];                    /**< Source name */
    char description[128];            /**< Source description */
} charm_entropy_source_info_t;

/**
 * @brief EFM configuration options
 */
typedef struct {
    uint32_t sampling_interval_ms;    /**< Sampling interval in milliseconds */
    uint32_t sample_size;             /**< Sample size in bytes */
    bool enable_hardware_rng;         /**< Enable hardware RNG sources */
    bool enable_system_entropy;       /**< Enable system entropy pool */
    bool enable_jitter_entropy;       /**< Enable CPU jitter entropy */
    bool enable_continuous_sampling;  /**< Enable continuous background sampling */
    uint32_t min_quality_threshold;   /**< Minimum acceptable quality (0-100) */
    uint32_t entropy_pool_size;       /**< Size of entropy pool in bytes */
} charm_efm_config_t;

/**
 * @brief Default configuration for the EFM
 */
extern const charm_efm_config_t CHARM_EFM_DEFAULT_CONFIG;

/**
 * @brief Initialize the Entropy Flux Monitor
 * 
 * This function initializes the EFM with the specified configuration.
 * 
 * @param ctx CHARM context
 * @param config Pointer to configuration structure
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_init(charm_context ctx, const charm_efm_config_t* config);

/**
 * @brief Start continuous entropy sampling
 * 
 * This function starts the continuous entropy sampling process.
 * 
 * @param ctx CHARM context
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_start_sampling(charm_context ctx);

/**
 * @brief Stop continuous entropy sampling
 * 
 * This function stops the continuous entropy sampling process.
 * 
 * @param ctx CHARM context
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_stop_sampling(charm_context ctx);

/**
 * @brief Collect entropy from a specific source
 * 
 * This function collects entropy from the specified source.
 * 
 * @param ctx CHARM context
 * @param source Entropy source type
 * @param buffer Buffer to receive entropy
 * @param length Number of bytes to collect
 * @param stats Pointer to receive entropy statistics
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_collect(charm_context ctx,
                                    charm_entropy_source_t source,
                                    void* buffer, size_t length,
                                    charm_entropy_stats_t* stats);

/**
 * @brief Get information about an entropy source
 * 
 * This function returns information about the specified entropy source.
 * 
 * @param ctx CHARM context
 * @param source Entropy source type
 * @param info Pointer to receive source information
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_get_source_info(charm_context ctx,
                                           charm_entropy_source_t source,
                                           charm_entropy_source_info_t* info);

/**
 * @brief Get the current entropy quality
 * 
 * This function returns the current overall entropy quality.
 * 
 * @param ctx CHARM context
 * @param quality Pointer to receive quality value (0-100)
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_get_quality(charm_context ctx, uint32_t* quality);

/**
 * @brief Register a callback for entropy quality changes
 * 
 * This function registers a callback to be called when the entropy
 * quality changes significantly.
 * 
 * @param ctx CHARM context
 * @param callback Function to call on quality changes
 * @param user_data User data to pass to callback
 * @return Status code indicating success or failure
 */
typedef void (*charm_efm_quality_callback)(void* user_data, 
                                          uint32_t old_quality,
                                          uint32_t new_quality);

charm_efm_status_t charm_efm_set_quality_callback(charm_context ctx,
                                                charm_efm_quality_callback callback,
                                                void* user_data);

/**
 * @brief Get entropy from the pool
 * 
 * This function retrieves entropy from the EFM's entropy pool.
 * 
 * @param ctx CHARM context
 * @param buffer Buffer to receive entropy
 * @param length Number of bytes to retrieve
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_get_entropy(charm_context ctx,
                                        void* buffer, size_t length);

/**
 * @brief Add entropy to the pool
 * 
 * This function adds entropy to the EFM's entropy pool.
 * 
 * @param ctx CHARM context
 * @param buffer Buffer containing entropy
 * @param length Number of bytes to add
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_add_entropy(charm_context ctx,
                                        const void* buffer, size_t length);

/**
 * @brief Shutdown the EFM
 * 
 * This function shuts down the EFM, releasing all allocated resources.
 * 
 * @param ctx CHARM context
 * @return Status code indicating success or failure
 */
charm_efm_status_t charm_efm_shutdown(charm_context ctx);

/**
 * @brief Get a string description of an EFM status code
 * 
 * @param status Status code to describe
 * @return String description of the status code
 */
const char* charm_efm_status_string(charm_efm_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_EFM_H */
