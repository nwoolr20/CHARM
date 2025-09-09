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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Status codes for CHARM functions
 */
typedef enum {
    CHARM_STATUS_SUCCESS = 0,                /**< Operation completed successfully */
    CHARM_STATUS_ERROR_INVALID_PARAM = 1,    /**< Invalid parameter */
    CHARM_STATUS_ERROR_NULL_POINTER = 2,     /**< Null pointer */
    CHARM_STATUS_ERROR_BUFFER_TOO_SMALL = 3, /**< Buffer too small */
    CHARM_STATUS_ERROR_NOT_INITIALIZED = 4,  /**< Component not initialized */
    CHARM_STATUS_ERROR_ALREADY_INITIALIZED = 5, /**< Component already initialized */
    CHARM_STATUS_ERROR_NOT_SUPPORTED = 6,    /**< Operation not supported */
    CHARM_STATUS_ERROR_NOT_IMPLEMENTED = 7,  /**< Operation not implemented */
    CHARM_STATUS_ERROR_IO = 8,               /**< I/O error */
    CHARM_STATUS_ERROR_MEMORY = 9,           /**< Memory allocation error */
    CHARM_STATUS_ERROR_TIMEOUT = 10,         /**< Operation timed out */
    CHARM_STATUS_ERROR_BUSY = 11,            /**< Resource busy */
    CHARM_STATUS_ERROR_NOT_FOUND = 12,       /**< Resource not found */
    CHARM_STATUS_ERROR_ALREADY_EXISTS = 13,  /**< Resource already exists */
    CHARM_STATUS_ERROR_PERMISSION = 14,      /**< Permission denied */
    CHARM_STATUS_ERROR_SYSTEM = 15,          /**< System error */
    CHARM_STATUS_ERROR_CORRUPTED = 16,       /**< Data corrupted */
    CHARM_STATUS_ERROR_INVALID_STATE = 17,   /**< Invalid state */
    CHARM_STATUS_ERROR_NOT_READY = 18,       /**< Not ready */
    CHARM_STATUS_ERROR_NOT_ACTIVE = 19,      /**< Not active */
    CHARM_STATUS_ERROR_ENTROPY_LOW = 20,     /**< Entropy too low */
    CHARM_STATUS_ERROR_ENTROPY_ANOMALY = 21, /**< Entropy anomaly detected */
    CHARM_STATUS_ERROR_ENTROPY_SOURCE = 22,  /**< Entropy source error */
    CHARM_STATUS_ERROR_DIGEST = 23,          /**< Digest error */
    CHARM_STATUS_ERROR_STREAM = 24,          /**< Stream error */
    CHARM_STATUS_ERROR_WATCHDOG = 25,        /**< Watchdog error */
    CHARM_STATUS_ERROR_BUS = 26,             /**< Bus error */
    CHARM_STATUS_ERROR_SIMD = 27,            /**< SIMD error */
    CHARM_STATUS_ERROR_LIMIT_EXCEEDED = 28,  /**< Limit exceeded */
    CHARM_STATUS_ERROR_INTERRUPTED = 29,     /**< Operation interrupted */
    CHARM_STATUS_ERROR_INTERNAL = 30,        /**< Internal error */
    CHARM_STATUS_ERROR_CRYPTO = 31,          /**< Cryptographic error */
    CHARM_STATUS_ERROR_PROTOCOL = 32,        /**< Protocol error */
    CHARM_STATUS_ERROR_NETWORK = 33,         /**< Network error */
    CHARM_STATUS_ERROR_OVERFLOW = 34,        /**< Overflow error */
    CHARM_STATUS_ERROR_UNDERFLOW = 35,       /**< Underflow error */
    CHARM_STATUS_ERROR_RANGE = 36,           /**< Range error */
    CHARM_STATUS_ERROR_DOMAIN = 37,          /**< Domain error */
    CHARM_STATUS_ERROR_PRECISION = 38,       /**< Precision error */
    CHARM_STATUS_ERROR_CALIBRATION = 39,     /**< Calibration error */
    CHARM_STATUS_ERROR_VERIFICATION = 40,    /**< Verification error */
    CHARM_STATUS_ERROR_AUTHENTICATION = 41,  /**< Authentication error */
    CHARM_STATUS_ERROR_AUTHORIZATION = 42,   /**< Authorization error */
    CHARM_STATUS_ERROR_EXPIRED = 43,         /**< Expired error */
    CHARM_STATUS_ERROR_REVOKED = 44,         /**< Revoked error */
    CHARM_STATUS_ERROR_INVALID_FORMAT = 45,  /**< Invalid format */
    CHARM_STATUS_ERROR_INVALID_VERSION = 46, /**< Invalid version */
    CHARM_STATUS_ERROR_INVALID_SIGNATURE = 47, /**< Invalid signature */
    CHARM_STATUS_ERROR_INVALID_CERTIFICATE = 48, /**< Invalid certificate */
    CHARM_STATUS_ERROR_INVALID_KEY = 49,     /**< Invalid key */
    CHARM_STATUS_ERROR_INVALID_ALGORITHM = 50, /**< Invalid algorithm */
    CHARM_STATUS_ERROR_INVALID_MODE = 51,    /**< Invalid mode */
    CHARM_STATUS_ERROR_INVALID_PADDING = 52, /**< Invalid padding */
    CHARM_STATUS_ERROR_INVALID_MAC = 53,     /**< Invalid MAC */
    CHARM_STATUS_ERROR_INVALID_IV = 54,      /**< Invalid IV */
    CHARM_STATUS_ERROR_INVALID_SALT = 55,    /**< Invalid salt */
    CHARM_STATUS_ERROR_INVALID_TAG = 56,     /**< Invalid tag */
    CHARM_STATUS_ERROR_INVALID_NONCE = 57,   /**< Invalid nonce */
    CHARM_STATUS_ERROR_INVALID_COUNTER = 58, /**< Invalid counter */
    CHARM_STATUS_ERROR_INVALID_SEED = 59,    /**< Invalid seed */
    CHARM_STATUS_ERROR_INVALID_ENTROPY = 60, /**< Invalid entropy */
    CHARM_STATUS_ERROR_INVALID_HASH = 61,    /**< Invalid hash */
    CHARM_STATUS_ERROR_INVALID_DIGEST = 62,  /**< Invalid digest */
    CHARM_STATUS_ERROR_INVALID_SIGNATURE_SCHEME = 63, /**< Invalid signature scheme */
    CHARM_STATUS_ERROR_INVALID_KDF = 64,     /**< Invalid KDF */
    CHARM_STATUS_ERROR_INVALID_PRF = 65,     /**< Invalid PRF */
    CHARM_STATUS_ERROR_INVALID_HMAC = 66,    /**< Invalid HMAC */
    CHARM_STATUS_ERROR_INVALID_CURVE = 67,   /**< Invalid curve */
    CHARM_STATUS_ERROR_INVALID_POINT = 68,   /**< Invalid point */
    CHARM_STATUS_ERROR_INVALID_SCALAR = 69,  /**< Invalid scalar */
    CHARM_STATUS_ERROR_INVALID_GROUP = 70,   /**< Invalid group */
    CHARM_STATUS_ERROR_INVALID_FIELD = 71,   /**< Invalid field */
    CHARM_STATUS_ERROR_INVALID_RING = 72,    /**< Invalid ring */
    CHARM_STATUS_ERROR_INVALID_LATTICE = 73, /**< Invalid lattice */
    CHARM_STATUS_ERROR_INVALID_VECTOR = 74,  /**< Invalid vector */
    CHARM_STATUS_ERROR_INVALID_MATRIX = 75,  /**< Invalid matrix */
    CHARM_STATUS_ERROR_INVALID_TENSOR = 76,  /**< Invalid tensor */
    CHARM_STATUS_ERROR_INVALID_POLYNOMIAL = 77, /**< Invalid polynomial */
    CHARM_STATUS_ERROR_INVALID_MODULUS = 78, /**< Invalid modulus */
    CHARM_STATUS_ERROR_INVALID_PRIME = 79,   /**< Invalid prime */
    CHARM_STATUS_ERROR_INVALID_GENERATOR = 80, /**< Invalid generator */
    CHARM_STATUS_ERROR_INVALID_ORDER = 81,   /**< Invalid order */
    CHARM_STATUS_ERROR_INVALID_COFACTOR = 82, /**< Invalid cofactor */
    CHARM_STATUS_ERROR_INVALID_DISCRIMINANT = 83, /**< Invalid discriminant */
    CHARM_STATUS_ERROR_INVALID_TRACE = 84,   /**< Invalid trace */
    CHARM_STATUS_ERROR_INVALID_NORM = 85,    /**< Invalid norm */
    CHARM_STATUS_ERROR_INVALID_RANK = 86,    /**< Invalid rank */
    CHARM_STATUS_ERROR_INVALID_DIMENSION = 87, /**< Invalid dimension */
    CHARM_STATUS_ERROR_INVALID_DEGREE = 88,  /**< Invalid degree */
    CHARM_STATUS_ERROR_INVALID_COEFFICIENT = 89, /**< Invalid coefficient */
    CHARM_STATUS_ERROR_INVALID_ROOT = 90,    /**< Invalid root */
    CHARM_STATUS_ERROR_INVALID_FACTOR = 91,  /**< Invalid factor */
    CHARM_STATUS_ERROR_INVALID_DIVISOR = 92, /**< Invalid divisor */
    CHARM_STATUS_ERROR_INVALID_MULTIPLE = 93, /**< Invalid multiple */
    CHARM_STATUS_ERROR_INVALID_REMAINDER = 94, /**< Invalid remainder */
    CHARM_STATUS_ERROR_INVALID_QUOTIENT = 95, /**< Invalid quotient */
    CHARM_STATUS_ERROR_INVALID_PRODUCT = 96, /**< Invalid product */
    CHARM_STATUS_ERROR_INVALID_SUM = 97,     /**< Invalid sum */
    CHARM_STATUS_ERROR_INVALID_DIFFERENCE = 98, /**< Invalid difference */
    CHARM_STATUS_ERROR_INVALID_RATIO = 99,   /**< Invalid ratio */
    CHARM_STATUS_ERROR_INVALID_PROPORTION = 100, /**< Invalid proportion */
    CHARM_STATUS_ERROR_UNKNOWN = 101         /**< Unknown error */
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
