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

#ifndef CHARM_TYPES_UNIFIED_H
#define CHARM_TYPES_UNIFIED_H

/**
 * @file charm_types_unified.h
 * @brief Unified type definitions for the CHARM v2.0 system
 * 
 * This header provides consolidated type definitions for the CHARM system,
 * eliminating duplications and ensuring consistent usage across all subsystems.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status codes for CHARM operations
 */
typedef enum {
    CHARM_STATUS_SUCCESS = 0,                /**< Operation successful */
    CHARM_STATUS_ERROR_GENERAL = -1,         /**< General error */
    CHARM_STATUS_ERROR_INVALID_PARAM = -2,   /**< Invalid parameter */
    CHARM_STATUS_ERROR_NOT_INITIALIZED = -3, /**< System not initialized */
    CHARM_STATUS_ERROR_ENTROPY_LOW = -4,     /**< Entropy too low */
    CHARM_STATUS_ERROR_MEMORY = -5,          /**< Memory allocation error */
    CHARM_STATUS_ERROR_IO = -6,              /**< I/O error */
    CHARM_STATUS_ERROR_TIMEOUT = -7,         /**< Operation timed out */
    CHARM_STATUS_ERROR_NOT_SUPPORTED = -8,   /**< Operation not supported */
    CHARM_STATUS_ERROR_BUSY = -9,            /**< System busy */
    CHARM_STATUS_ERROR_WOULD_BLOCK = -10,    /**< Operation would block */
    CHARM_STATUS_ERROR_OVERFLOW = -11,       /**< Buffer overflow */
    CHARM_STATUS_ERROR_UNDERFLOW = -12,      /**< Buffer underflow */
    CHARM_STATUS_ERROR_NOT_FOUND = -13,      /**< Resource not found */
    CHARM_STATUS_ERROR_ALREADY_EXISTS = -14, /**< Resource already exists */
    CHARM_STATUS_ERROR_ACCESS_DENIED = -15,  /**< Access denied */
    CHARM_STATUS_ERROR_INTERRUPTED = -16,    /**< Operation interrupted */
    CHARM_STATUS_ERROR_HARDWARE = -17,       /**< Hardware error */
    CHARM_STATUS_ERROR_NETWORK = -18,        /**< Network error */
    CHARM_STATUS_ERROR_PROTOCOL = -19,       /**< Protocol error */
    CHARM_STATUS_ERROR_FORMAT = -20,         /**< Format error */
    CHARM_STATUS_ERROR_CRYPTO = -21,         /**< Cryptographic error */
    CHARM_STATUS_ERROR_EXPIRED = -22,        /**< Resource expired */
    CHARM_STATUS_ERROR_BUSY_RETRY = -23,     /**< System busy, retry later */
    CHARM_STATUS_ERROR_DEGRADED = -24,       /**< System in degraded mode */
    CHARM_STATUS_ERROR_FAILOVER = -25,       /**< System in failover mode */
    CHARM_STATUS_ERROR_INTERNAL = -26,       /**< Internal error */
    CHARM_STATUS_ERROR_UNKNOWN = -99         /**< Unknown error */
} charm_status_t;

/**
 * @brief System lifecycle states
 */
typedef enum {
    CHARM_STATE_INIT = 0,      /**< Initialization state */
    CHARM_STATE_RUN,           /**< Normal running state */
    CHARM_STATE_DEGRADED,      /**< Degraded operation state */
    CHARM_STATE_FAILOVER,      /**< Failover operation state */
    CHARM_STATE_SHUTDOWN       /**< Shutdown state */
} charm_state_t;

/**
 * @brief Entropy source types
 */
typedef enum {
    CHARM_ENTROPY_SOURCE_HARDWARE = 0, /**< Hardware RNG */
    CHARM_ENTROPY_SOURCE_SYSTEM,       /**< System entropy pool */
    CHARM_ENTROPY_SOURCE_JITTER,       /**< CPU jitter-based entropy */
    CHARM_ENTROPY_SOURCE_EXTERNAL,     /**< External entropy source */
    CHARM_ENTROPY_SOURCE_MIXED,        /**< Mixed entropy sources */
    CHARM_ENTROPY_SOURCE_FALLBACK      /**< Fallback entropy source */
} charm_entropy_source_type_t;

/**
 * @brief Entropy source information
 */
typedef struct {
    char name[32];                     /**< Source name */
    charm_entropy_source_type_t type;  /**< Source type */
    double quality;                    /**< Quality estimate (0.0-1.0) */
    uint64_t bytes_contributed;        /**< Total bytes contributed */
    uint64_t last_update;              /**< Timestamp of last update */
    bool active;                       /**< Whether source is active */
} charm_entropy_source_t;

/**
 * @brief Entropy statistics
 */
typedef struct {
    double shannon_entropy;            /**< Shannon entropy value */
    double min_entropy;                /**< Min-entropy estimate */
    double chi_squared;                /**< Chi-squared test result */
    double monte_carlo_pi;             /**< Monte Carlo PI approximation */
    double serial_correlation;         /**< Serial correlation coefficient */
    uint64_t timestamp;                /**< Timestamp of measurement */
    uint32_t sample_size;              /**< Size of sample in bytes */
    charm_entropy_source_t source;     /**< Entropy source information */
} charm_entropy_stats_t;

/**
 * @brief Digest context for streaming operations
 */
typedef struct {
    uint32_t state[8];                 /**< Hash state */
    uint8_t buffer[64];                /**< Input buffer */
    uint64_t total_bytes;              /**< Total bytes processed */
    uint8_t buffer_size;               /**< Current buffer fill level */
    bool finalized;                    /**< Whether context is finalized */
} charm_digest_ctx_t;

/**
 * @brief Maximum number of subsystems
 */
#define CHARM_MAX_SUBSYSTEMS 8

/**
 * @brief Subsystem identifiers
 */
typedef enum {
    CHARM_SUBSYS_CORE = 0,       /**< Core subsystem */
    CHARM_SUBSYS_CAEDS = 1,      /**< CAEDS subsystem */
    CHARM_SUBSYS_CEE = 2,        /**< CEE subsystem */
    CHARM_SUBSYS_ECE = 3,        /**< ECE subsystem */
    CHARM_SUBSYS_ENTROPY_BUS = 4, /**< Entropy bus subsystem */
    CHARM_SUBSYS_CLI = 5,         /**< CLI subsystem */
    CHARM_SUBSYS_DRIVERS = 6,     /**< Hardware drivers subsystem */
    CHARM_SUBSYS_WATCHDOG = 7     /**< Watchdog subsystem */
} charm_subsystem_id_t;

/**
 * @brief Global context structure
 */
struct charm_context_s {
    void* subsystems[CHARM_MAX_SUBSYSTEMS]; /**< Subsystem contexts */
    charm_status_t status;                  /**< Global system status */
    charm_state_t state;                    /**< System lifecycle state */
    uint32_t active_subsystems;             /**< Bitmap of active subsystems */
    uint32_t flags;                         /**< Global flags */
    void* user_data;                        /**< User-defined data */
};

/**
 * @brief Global context handle
 */
typedef struct charm_context_s* charm_context;

#ifdef __cplusplus
}
#endif

#endif /* CHARM_TYPES_UNIFIED_H */
