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

#ifndef CAEDS_ANOMALY_H
#define CAEDS_ANOMALY_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <pthread.h>
#include "charm_status.h"
#include "charm_types.h"
#include "charm_ml_types.h"
#include "charm_ml_model.h"
#include "caeds_anomaly_types.h"
#include "caeds_flux_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of anomaly callbacks
 */
#define CAEDS_ANOMALY_MAX_CALLBACKS 16

/**
 * @brief Maximum number of anomaly sources
 */
#define CAEDS_ANOMALY_MAX_SOURCES 8

/**
 * @brief Anomaly detection method
 */
typedef enum {
    CAEDS_ANOMALY_METHOD_STATISTICAL = 0,  /**< Statistical methods */
    CAEDS_ANOMALY_METHOD_FREQUENCY = 1,     /**< Frequency analysis */
    CAEDS_ANOMALY_METHOD_PATTERN = 2,       /**< Pattern detection */
    CAEDS_ANOMALY_METHOD_CORRELATION = 3,   /**< Correlation analysis */
    CAEDS_ANOMALY_METHOD_MACHINE_LEARNING = 4, /**< Machine learning */
    CAEDS_ANOMALY_METHOD_CUSTOM = 5         /**< Custom method */
} caeds_anomaly_method_t;

/**
 * @brief Anomaly severity level
 */
typedef enum {
    CAEDS_ANOMALY_SEVERITY_NONE = 0,      /**< No severity (normal) */
    CAEDS_ANOMALY_SEVERITY_INFO = 1,      /**< Informational */
    CAEDS_ANOMALY_SEVERITY_LOW = 2,       /**< Low severity */
    CAEDS_ANOMALY_SEVERITY_MEDIUM = 3,    /**< Medium severity */
    CAEDS_ANOMALY_SEVERITY_HIGH = 4,      /**< High severity */
    CAEDS_ANOMALY_SEVERITY_CRITICAL = 5   /**< Critical severity */
} caeds_anomaly_severity_t;

/**
 * @brief Anomaly type
 */
typedef enum {
    CAEDS_ANOMALY_TYPE_NONE = 0,           /**< No anomaly */
    CAEDS_ANOMALY_TYPE_ENTROPY_LOW = 1,     /**< Low entropy */
    CAEDS_ANOMALY_TYPE_ENTROPY_HIGH = 2,    /**< High entropy */
    CAEDS_ANOMALY_TYPE_PATTERN = 3,         /**< Pattern detected */
    CAEDS_ANOMALY_TYPE_FREQUENCY = 4,       /**< Frequency anomaly */
    CAEDS_ANOMALY_TYPE_CORRELATION = 5,     /**< Correlation anomaly */
    CAEDS_ANOMALY_TYPE_SOURCE_FAILURE = 6,  /**< Source failure */
    CAEDS_ANOMALY_TYPE_SOURCE_BIAS = 7,     /**< Source bias */
    CAEDS_ANOMALY_TYPE_CUSTOM = 8           /**< Custom anomaly */
} caeds_anomaly_type_t;

/**
 * @brief Anomaly detection configuration
 */
typedef struct {
    caeds_anomaly_method_t method;         /**< Detection method */
    uint32_t window_size;                  /**< Analysis window size */
    uint32_t threshold;                    /**< Detection threshold */
    uint32_t min_samples;                  /**< Minimum samples required */
    uint32_t max_anomalies;               /**< Maximum anomalies to track */
    bool auto_reset;                       /**< Auto-reset after detection */
    uint32_t reset_interval;               /**< Reset interval in milliseconds */
    uint32_t source_mask;                  /**< Source mask */
    void* context;                         /**< User context */
} caeds_anomaly_config_t;

/**
 * @brief Anomaly event
 */
typedef struct {
    caeds_anomaly_type_t type;             /**< Anomaly type */
    caeds_anomaly_severity_t severity;     /**< Anomaly severity */
    uint32_t source_id;                    /**< Source ID */
    uint64_t timestamp;                    /**< Timestamp */
    uint32_t duration;                     /**< Duration in milliseconds */
    float confidence;                      /**< Detection confidence (0.0-1.0) */
    uint32_t sample_count;                 /**< Number of samples analyzed */
    uint8_t data[64];                      /**< Anomaly-specific data */
    uint32_t data_size;                    /**< Size of anomaly-specific data */
} caeds_anomaly_event_t;

/**
 * @brief Anomaly callback function
 */
typedef void (*caeds_anomaly_callback_t)(const caeds_anomaly_event_t* event, void* user_data);

/**
 * @brief Anomaly detection handle
 */
typedef caeds_anomaly_context_t* caeds_anomaly_handle_t;

/**
 * @brief Initialize anomaly detection
 * 
 * @param config Configuration
 * @return caeds_anomaly_handle_t Handle or NULL on failure
 */
caeds_anomaly_handle_t caeds_anomaly_init(const caeds_anomaly_config_t* config);

/**
 * @brief Shutdown anomaly detection
 * 
 * @param handle Anomaly detection handle
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_shutdown(caeds_anomaly_handle_t handle);

/**
 * @brief Process entropy sample
 * 
 * @param handle Anomaly detection handle
 * @param source_id Source ID
 * @param sample Sample data
 * @param sample_size Sample size
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_process_sample(caeds_anomaly_handle_t handle, uint32_t source_id, const uint8_t* sample, uint32_t sample_size);

/**
 * @brief Register anomaly callback
 * 
 * @param handle Anomaly detection handle
 * @param callback Callback function
 * @param user_data User data
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_register_callback(caeds_anomaly_handle_t handle, caeds_anomaly_callback_t callback, void* user_data);

/**
 * @brief Unregister anomaly callback
 * 
 * @param handle Anomaly detection handle
 * @param callback Callback function
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_unregister_callback(caeds_anomaly_handle_t handle, caeds_anomaly_callback_t callback);

/**
 * @brief Get anomaly statistics
 * 
 * @param handle Anomaly detection handle
 * @param source_id Source ID
 * @param anomaly_count Pointer to receive anomaly count
 * @param last_anomaly Pointer to receive last anomaly event (can be NULL)
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_stats(caeds_anomaly_handle_t handle, uint32_t source_id, uint32_t* anomaly_count, caeds_anomaly_event_t* last_anomaly);

/**
 * @brief Reset anomaly detection
 * 
 * @param handle Anomaly detection handle
 * @param source_id Source ID (UINT32_MAX for all sources)
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_reset(caeds_anomaly_handle_t handle, uint32_t source_id);

/**
 * @brief Set detection threshold
 * 
 * @param handle Anomaly detection handle
 * @param threshold Detection threshold
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_set_threshold(caeds_anomaly_handle_t handle, uint32_t threshold);

/**
 * @brief Get detection threshold
 * 
 * @param handle Anomaly detection handle
 * @param threshold Pointer to receive threshold
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_threshold(caeds_anomaly_handle_t handle, uint32_t* threshold);

/**
 * @brief Set detection method
 * 
 * @param handle Anomaly detection handle
 * @param method Detection method
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_set_method(caeds_anomaly_handle_t handle, caeds_anomaly_method_t method);

/**
 * @brief Get detection method
 * 
 * @param handle Anomaly detection handle
 * @param method Pointer to receive method
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_method(caeds_anomaly_handle_t handle, caeds_anomaly_method_t* method);

/**
 * @brief Set machine learning model
 * 
 * @param handle Anomaly detection handle
 * @param model Machine learning model
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_set_model(caeds_anomaly_handle_t handle, charm_ml_model_t* model);

/**
 * @brief Get machine learning model
 * 
 * @param handle Anomaly detection handle
 * @param model Pointer to receive model
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_model(caeds_anomaly_handle_t handle, charm_ml_model_t** model);

/**
 * @brief Set model parameters
 * 
 * @param handle Anomaly detection handle
 * @param params Model parameters
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_set_model_params(caeds_anomaly_handle_t handle, const charm_ml_model_params_t* params);

/**
 * @brief Get model parameters
 * 
 * @param handle Anomaly detection handle
 * @param params Pointer to receive parameters
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_model_params(caeds_anomaly_handle_t handle, charm_ml_model_params_t* params);

/**
 * @brief Train model with historical data
 * 
 * @param handle Anomaly detection handle
 * @param training_data Training data
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_train_model(caeds_anomaly_handle_t handle, const charm_ml_training_data_t* training_data);

/**
 * @brief Get timestamp in milliseconds
 * 
 * @return uint64_t Timestamp
 */
uint64_t caeds_anomaly_get_timestamp(void);

/**
 * @brief Convert model type to string
 * 
 * @param model Model type
 * @return const char* String representation
 */
const char* caeds_anomaly_model_to_string(caeds_anomaly_model_t model);

/**
 * @brief Process entropy from source
 * 
 * @param handle Anomaly detection handle
 * @param source Entropy source
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_process_source(caeds_anomaly_handle_t handle, caeds_flux_source_t source);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_ANOMALY_H */
