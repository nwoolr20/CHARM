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

#ifndef CHARM_STATUS_H
#define CHARM_STATUS_H

/**
 * @file charm_status.h
 * @brief Common status codes for the CHARM v2.0 system
 * 
 * This header defines the common status codes used throughout the CHARM system,
 * providing a unified error handling mechanism for all subsystems.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Common status codes for CHARM operations
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
 * @brief Get string representation of status code
 * 
 * @param status Status code
 * @return const char* String representation
 */
const char* charm_status_to_string(charm_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_STATUS_H */
