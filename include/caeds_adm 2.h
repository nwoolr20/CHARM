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
 * @file caeds_adm.h
 * @brief Anomaly Detection Module for CAEDS
 * 
 * This header defines the interfaces for the Anomaly Detection Module (ADM)
 * component of the CAEDS subsystem, responsible for detecting anomalies
 * in entropy quality and patterns.
 */

#ifndef CAEDS_ADM_H
#define CAEDS_ADM_H

#include "core/charm.h"
#include "ml/charm_ml.h"
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ADM status codes
 */
typedef enum {
    CHARM_ADM_SUCCESS = 0,
    CHARM_ADM_ERROR_INITIALIZATION = -1,
    CHARM_ADM_ERROR_INVALID_PARAMETER = -2,
    CHARM_ADM_ERROR_INSUFFICIENT_DATA = -3,
    CHARM_ADM_ERROR_MODEL_FAILURE = -4,
    CHARM_ADM_ERROR_NOT_INITIALIZED = -5
} charm_adm_status_t;

/**
 * @brief Anomaly severity levels
 */
typedef enum {
    CHARM_ANOMALY_NONE = 0,     /**< No anomaly detected */
    CHARM_ANOMALY_INFO,         /**< Informational anomaly */
    CHARM_ANOMALY_WARNING,      /**< Warning-level anomaly */
    CHARM_ANOMALY_CRITICAL      /**< Critical anomaly */
} charm_anomaly_severity_t;

/**
 * @brief Anomaly types
 */
typedef enum {
    CHARM_ANOMALY_TYPE_ENTROPY_DROP = 0,  /**< Sudden drop in entropy */
    CHARM_ANOMALY_TYPE_PATTERN,           /**< Unexpected pattern detected */
    CHARM_ANOMALY_TYPE_BIAS,              /**< Statistical bias detected */
    CHARM_ANOMALY_TYPE_CORRELATION,       /**< Unexpected correlation */
    CHARM_ANOMALY_TYPE_TREND,             /**< Concerning trend detected */
    CHARM_ANOMALY_TYPE_UNKNOWN            /**< Unknown anomaly type */
} charm_anomaly_type_t;

/**
 * @brief Detected anomaly information
 */
typedef struct {
    time_t timestamp;                  /**< Detection timestamp */
    charm_anomaly_severity_t severity; /**< Anomaly severity */
    charm_anomaly_type_t type;         /**< Anomaly type */
    double confidence;                 /**< Detection confidence (0-1) */
    charm_entropy_stats_t stats;       /**< Entropy statistics at detection */
    char description[256];             /**< Human-readable description */
} charm_anomaly_info_t;

/**
 * @brief ADM configuration options
 */
typedef struct {
    uint32_t detection_window;         /**< Number of samples to analyze */
    uint32_t sensitivity;              /**< Detection sensitivity (0-100) */
    bool enable_ml_detection;          /**< Enable ML-based detection */
    bool enable_statistical_detection; /**< Enable statistical detection */
    bool enable_pattern_detection;     /**< Enable pattern detection */
    double critical_threshold;         /**< Threshold for critical anomalies */
    double warning_threshold;          /**< Threshold for warning anomalies */
    const char* model_file;            /**< Path to pre-trained model file (optional) */
} charm_adm_config_t;

/**
 * @brief Default configuration for the ADM
 */
extern const charm_adm_config_t CHARM_ADM_DEFAULT_CONFIG;

/**
 * @brief Initialize the Anomaly Detection Module
 * 
 * This function initializes the ADM with the specified configuration.
 * 
 * @param ctx CHARM context
 * @param config Pointer to configuration structure
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_init(charm_context ctx, const charm_adm_config_t* config);

/**
 * @brief Process new entropy data for anomaly detection
 * 
 * This function processes new entropy data to detect anomalies.
 * 
 * @param ctx CHARM context
 * @param stats Pointer to entropy statistics
 * @param anomaly Pointer to receive anomaly information (if detected)
 * @param detected Pointer to receive detection flag
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_process(charm_context ctx, 
                                    const charm_entropy_stats_t* stats,
                                    charm_anomaly_info_t* anomaly,
                                    bool* detected);

/**
 * @brief Register a callback for anomaly detection
 * 
 * This function registers a callback to be called when an anomaly is detected.
 * 
 * @param ctx CHARM context
 * @param callback Function to call on anomaly detection
 * @param user_data User data to pass to callback
 * @return Status code indicating success or failure
 */
typedef void (*charm_adm_anomaly_callback)(void* user_data, 
                                          const charm_anomaly_info_t* anomaly);

charm_adm_status_t charm_adm_set_anomaly_callback(charm_context ctx,
                                                charm_adm_anomaly_callback callback,
                                                void* user_data);

/**
 * @brief Get historical anomalies
 * 
 * This function returns a specified number of historical anomalies
 * detected by the ADM.
 * 
 * @param ctx CHARM context
 * @param anomalies Array to receive anomalies
 * @param count Number of anomalies to retrieve
 * @param actual_count Pointer to receive actual number of anomalies retrieved
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_get_history(charm_context ctx,
                                        charm_anomaly_info_t* anomalies,
                                        uint32_t count,
                                        uint32_t* actual_count);

/**
 * @brief Get the current detection threshold
 * 
 * This function returns the current detection threshold for the
 * specified severity level.
 * 
 * @param ctx CHARM context
 * @param severity Anomaly severity level
 * @param threshold Pointer to receive threshold value
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_get_threshold(charm_context ctx,
                                          charm_anomaly_severity_t severity,
                                          double* threshold);

/**
 * @brief Set the detection threshold
 * 
 * This function sets the detection threshold for the specified
 * severity level.
 * 
 * @param ctx CHARM context
 * @param severity Anomaly severity level
 * @param threshold New threshold value
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_set_threshold(charm_context ctx,
                                          charm_anomaly_severity_t severity,
                                          double threshold);

/**
 * @brief Save the ADM model to file
 * 
 * This function saves the current ADM model to a file for later use.
 * 
 * @param ctx CHARM context
 * @param filename Path to output file
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_save_model(charm_context ctx, const char* filename);

/**
 * @brief Load an ADM model from file
 * 
 * This function loads an ADM model from a file.
 * 
 * @param ctx CHARM context
 * @param filename Path to model file
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_load_model(charm_context ctx, const char* filename);

/**
 * @brief Shutdown the ADM
 * 
 * This function shuts down the ADM, releasing all allocated resources.
 * 
 * @param ctx CHARM context
 * @return Status code indicating success or failure
 */
charm_adm_status_t charm_adm_shutdown(charm_context ctx);

/**
 * @brief Get a string description of an ADM status code
 * 
 * @param status Status code to describe
 * @return String description of the status code
 */
const char* charm_adm_status_string(charm_adm_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_ADM_H */
