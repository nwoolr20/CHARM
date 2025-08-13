#ifndef CHARM_TYPES_H
#define CHARM_TYPES_H

/**
 * @file charm_types.h
 * @brief Core type definitions for the CHARM v2.0 system
 * 
 * This header defines common types used throughout the CHARM system,
 * providing a unified type system for all subsystems.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM context structure
 * 
 * Opaque handle to CHARM subsystem context
 */
typedef struct charm_context_s* charm_context;

/**
 * @brief Entropy statistics structure
 * 
 * Contains statistical information about entropy quality and characteristics
 */
typedef struct {
    double entropy_estimate;     /**< Estimated entropy in bits per byte */
    double quality;              /**< Overall quality metric (0.0-1.0) */
    double bias;                 /**< Bias metric (0.0-1.0, lower is better) */
    double correlation;          /**< Serial correlation metric (0.0-1.0, lower is better) */
    double shannon_entropy;      /**< Shannon entropy estimate */
    double chi_squared;          /**< Chi-squared test result */
    double monte_carlo_pi;       /**< Monte Carlo PI approximation result */
    double serial_correlation;   /**< Serial correlation coefficient */
    uint64_t sample_count;       /**< Number of samples in this measurement */
    uint64_t total_bytes;        /**< Total bytes processed */
    time_t timestamp;            /**< Timestamp of this measurement */
    uint32_t flags;              /**< Status flags */
} charm_entropy_stats_t;

/**
 * @brief Entropy source information
 */
typedef struct {
    char name[32];              /**< Source name */
    double quality;             /**< Quality estimate (0.0-1.0) */
    uint64_t bytes_contributed; /**< Total bytes contributed */
    uint64_t last_update;       /**< Timestamp of last update */
    bool active;                /**< Whether source is active */
} charm_entropy_source_t;

/**
 * @brief Entropy consumer information
 */
typedef struct {
    char name[32];              /**< Consumer name */
    uint64_t bytes_consumed;    /**< Total bytes consumed */
    uint64_t last_access;       /**< Timestamp of last access */
    bool active;                /**< Whether consumer is active */
} charm_entropy_consumer_t;

/**
 * @brief Digest context for streaming operations
 */
typedef struct {
    uint32_t state[8];          /**< Hash state */
    uint8_t buffer[64];         /**< Input buffer */
    uint64_t total_bytes;       /**< Total bytes processed */
    uint8_t buffer_size;        /**< Current buffer fill level */
    bool finalized;             /**< Whether context is finalized */
} charm_digest_ctx_t;

/**
 * @brief System states
 */
typedef enum {
    CHARM_STATE_INIT,      /**< System initializing */
    CHARM_STATE_RUN,       /**< Normal operation */
    CHARM_STATE_DEGRADED,  /**< Degraded entropy quality */
    CHARM_STATE_FAILOVER   /**< Using fallback entropy sources */
} charm_system_state_t;

/**
 * @brief Entropy quality levels
 */
typedef enum {
    CHARM_ENTROPY_QUALITY_CRITICAL = 0,  /**< Critical - insufficient for security */
    CHARM_ENTROPY_QUALITY_LOW = 1,       /**< Low - minimum acceptable */
    CHARM_ENTROPY_QUALITY_MEDIUM = 2,    /**< Medium - adequate */
    CHARM_ENTROPY_QUALITY_HIGH = 3       /**< High - excellent */
} charm_entropy_quality_t;

/**
 * @brief Error codes
 */
typedef enum {
    CHARM_SUCCESS = 0,                /**< Operation successful */
    CHARM_ERROR_GENERAL = -1,         /**< General error */
    CHARM_ERROR_INVALID_PARAM = -2,   /**< Invalid parameter */
    CHARM_ERROR_NOT_INITIALIZED = -3, /**< System not initialized */
    CHARM_ERROR_ENTROPY_LOW = -4,     /**< Entropy too low */
    CHARM_ERROR_MEMORY = -5,          /**< Memory allocation error */
    CHARM_ERROR_IO = -6,              /**< I/O error */
    CHARM_ERROR_TIMEOUT = -7          /**< Operation timed out */
} charm_error_t;

#ifdef __cplusplus
}
#endif

#endif /* CHARM_TYPES_H */
