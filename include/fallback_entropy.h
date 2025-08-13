/**
 * @file fallback_entropy.h
 * @brief Fallback entropy generation for the CHARM system
 * 
 * This header defines the fallback entropy generation mechanisms for the CHARM
 * system, providing synthetic entropy when hardware sources are unavailable or
 * insufficient.
 */

#ifndef FALLBACK_ENTROPY_H
#define FALLBACK_ENTROPY_H

#include "charm_status.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fallback entropy generation methods
 */
typedef enum {
    FALLBACK_METHOD_TIMER_JITTER = 0,    /**< Timer jitter collection */
    FALLBACK_METHOD_MEMORY_NOISE = 1,    /**< Memory access noise */
    FALLBACK_METHOD_CPU_EXECUTION = 2,   /**< CPU execution timing */
    FALLBACK_METHOD_SYSTEM_STATS = 3,    /**< System statistics */
    FALLBACK_METHOD_MIXED = 4            /**< Mixed methods */
} fallback_method_t;

/**
 * @brief Fallback entropy configuration
 */
typedef struct {
    fallback_method_t method;            /**< Generation method */
    uint32_t collection_rounds;          /**< Collection rounds */
    uint32_t mixing_iterations;          /**< Mixing iterations */
    bool continuous_collection;          /**< Continuous collection flag */
    uint32_t min_entropy_bits_per_byte;  /**< Minimum entropy bits per byte (0-8) */
} fallback_config_t;

/**
 * @brief Fallback entropy statistics
 */
typedef struct {
    uint64_t bytes_generated;            /**< Total bytes generated */
    double avg_entropy_bits_per_byte;    /**< Average entropy bits per byte */
    uint64_t collection_time_us;         /**< Collection time in microseconds */
    uint32_t rounds_completed;           /**< Rounds completed */
} fallback_stats_t;

/**
 * @brief Fallback entropy context handle
 */
typedef struct fallback_context* fallback_handle_t;

/**
 * @brief Default fallback configuration
 */
extern const fallback_config_t FALLBACK_DEFAULT_CONFIG;

/**
 * @brief Initialize fallback entropy generator
 * 
 * @param config Configuration (NULL for default)
 * @return fallback_handle_t Handle or NULL on failure
 */
fallback_handle_t fallback_init(const fallback_config_t* config);

/**
 * @brief Shutdown fallback entropy generator
 * 
 * @param handle Generator handle
 */
void fallback_shutdown(fallback_handle_t handle);

/**
 * @brief Generate fallback entropy
 * 
 * @param handle Generator handle
 * @param buffer Output buffer
 * @param size Buffer size in bytes
 * @return charm_status_t Status code
 */
charm_status_t fallback_generate(fallback_handle_t handle, uint8_t* buffer, size_t size);

/**
 * @brief Start continuous entropy collection
 * 
 * @param handle Generator handle
 * @return charm_status_t Status code
 */
charm_status_t fallback_start_continuous(fallback_handle_t handle);

/**
 * @brief Stop continuous entropy collection
 * 
 * @param handle Generator handle
 * @return charm_status_t Status code
 */
charm_status_t fallback_stop_continuous(fallback_handle_t handle);

/**
 * @brief Get fallback entropy statistics
 * 
 * @param handle Generator handle
 * @param stats Statistics structure
 * @return charm_status_t Status code
 */
charm_status_t fallback_get_stats(fallback_handle_t handle, fallback_stats_t* stats);

/**
 * @brief Set fallback generation method
 * 
 * @param handle Generator handle
 * @param method Generation method
 * @return charm_status_t Status code
 */
charm_status_t fallback_set_method(fallback_handle_t handle, fallback_method_t method);

/**
 * @brief Estimate entropy quality of a buffer
 * 
 * @param buffer Input buffer
 * @param size Buffer size in bytes
 * @return double Estimated entropy bits per byte (0.0-8.0)
 */
double fallback_estimate_entropy(const uint8_t* buffer, size_t size);

/**
 * @brief Get string representation of fallback method
 * 
 * @param method Fallback method
 * @return const char* String representation
 */
const char* fallback_method_to_string(fallback_method_t method);

#ifdef __cplusplus
}
#endif

#endif /* FALLBACK_ENTROPY_H */
