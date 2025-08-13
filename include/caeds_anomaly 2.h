/**
 * @file caeds_anomaly.h
 * @brief ML-based entropy anomaly detection for the CAEDS subsystem
 * 
 * This header defines the entropy anomaly detection system for the CAEDS
 * (Charm Advanced Entropy Diagnostic System) subsystem, providing mechanisms
 * to detect anomalies in entropy sources using machine learning techniques.
 */

#ifndef CAEDS_ANOMALY_H
#define CAEDS_ANOMALY_H

#include "charm_status.h"
#include "caeds_flux.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Anomaly detection model types
 */
typedef enum {
    CAEDS_ANOMALY_MODEL_EWMA = 0,       /**< Exponentially Weighted Moving Average */
    CAEDS_ANOMALY_MODEL_CUSUM = 1,      /**< Cumulative Sum */
    CAEDS_ANOMALY_MODEL_ENTROPY = 2,     /**< Entropy-based */
    CAEDS_ANOMALY_MODEL_ADAPTIVE = 3    /**< Adaptive threshold */
} caeds_anomaly_model_t;

/**
 * @brief Anomaly severity levels
 */
typedef enum {
    CAEDS_ANOMALY_SEVERITY_NONE = 0,     /**< No anomaly */
    CAEDS_ANOMALY_SEVERITY_LOW = 1,      /**< Low severity anomaly */
    CAEDS_ANOMALY_SEVERITY_MEDIUM = 2,   /**< Medium severity anomaly */
    CAEDS_ANOMALY_SEVERITY_HIGH = 3,     /**< High severity anomaly */
    CAEDS_ANOMALY_SEVERITY_CRITICAL = 4  /**< Critical severity anomaly */
} caeds_anomaly_severity_t;

/**
 * @brief Anomaly detection configuration
 */
typedef struct {
    caeds_anomaly_model_t model;         /**< Detection model type */
    uint32_t window_size;                /**< Sliding window size */
    uint32_t history_size;               /**< History size for training */
    double threshold_low;                /**< Low severity threshold */
    double threshold_medium;             /**< Medium severity threshold */
    double threshold_high;               /**< High severity threshold */
    double threshold_critical;           /**< Critical severity threshold */
    double learning_rate;                /**< Learning rate for adaptive models */
    uint32_t min_samples;                /**< Minimum samples before detection */
    bool auto_train;                     /**< Automatically train model */
} caeds_anomaly_config_t;

/**
 * @brief Anomaly detection context handle
 */
typedef struct caeds_anomaly_context_s* caeds_anomaly_context_t;

/**
 * @brief Anomaly detection result
 */
typedef struct {
    caeds_anomaly_severity_t severity;   /**< Anomaly severity */
    double score;                        /**< Anomaly score */
    double threshold;                    /**< Threshold that was exceeded */
    uint64_t timestamp;                  /**< Detection timestamp */
    caeds_flux_source_t source;          /**< Entropy source */
    const char* description;             /**< Description of the anomaly */
} caeds_anomaly_result_t;

/**
 * @brief Anomaly detection callback function type
 * 
 * @param result Anomaly detection result
 * @param user_data User-defined data
 */
typedef void (*caeds_anomaly_callback_t)(const caeds_anomaly_result_t* result, void* user_data);

/**
 * @brief Default configuration
 */
extern const caeds_anomaly_config_t CAEDS_ANOMALY_DEFAULT_CONFIG;

/**
 * @brief Initialize the anomaly detection system
 * 
 * @param context Pointer to store the anomaly detection context
 * @param flux_context Entropy flux context
 * @param config Configuration parameters (NULL for default)
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_init(caeds_anomaly_context_t* context,
                                 caeds_flux_context_t flux_context,
                                 const caeds_anomaly_config_t* config);

/**
 * @brief Register a callback for anomaly detection
 * 
 * @param context Anomaly detection context
 * @param callback Callback function
 * @param user_data User-defined data
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_register_callback(caeds_anomaly_context_t context,
                                             caeds_anomaly_callback_t callback,
                                             void* user_data);

/**
 * @brief Process entropy samples for anomaly detection
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param samples Entropy samples
 * @param count Number of samples
 * @param result Pointer to store detection result (NULL if not needed)
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_process(caeds_anomaly_context_t context,
                                    caeds_flux_source_t source,
                                    const uint8_t* samples,
                                    size_t count,
                                    caeds_anomaly_result_t* result);

/**
 * @brief Process entropy volatility for anomaly detection
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param volatility Entropy volatility
 * @param result Pointer to store detection result (NULL if not needed)
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_process_volatility(caeds_anomaly_context_t context,
                                              caeds_flux_source_t source,
                                              const caeds_flux_volatility_t* volatility,
                                              caeds_anomaly_result_t* result);

/**
 * @brief Train the anomaly detection model
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param samples Training samples
 * @param count Number of samples
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_train(caeds_anomaly_context_t context,
                                  caeds_flux_source_t source,
                                  const uint8_t* samples,
                                  size_t count);

/**
 * @brief Reset the anomaly detection model
 * 
 * @param context Anomaly detection context
 * @param source Entropy source (CAEDS_FLUX_SOURCE_COUNT for all sources)
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_reset(caeds_anomaly_context_t context,
                                  caeds_flux_source_t source);

/**
 * @brief Get the current model parameters
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param model_params Buffer to store model parameters
 * @param size Size of the buffer
 * @param actual_size Pointer to store actual size of model parameters
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_model_params(caeds_anomaly_context_t context,
                                            caeds_flux_source_t source,
                                            void* model_params,
                                            size_t size,
                                            size_t* actual_size);

/**
 * @brief Set the model parameters
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param model_params Model parameters
 * @param size Size of model parameters
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_set_model_params(caeds_anomaly_context_t context,
                                            caeds_flux_source_t source,
                                            const void* model_params,
                                            size_t size);

/**
 * @brief Set the detection thresholds
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param threshold_low Low severity threshold
 * @param threshold_medium Medium severity threshold
 * @param threshold_high High severity threshold
 * @param threshold_critical Critical severity threshold
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_set_thresholds(caeds_anomaly_context_t context,
                                          caeds_flux_source_t source,
                                          double threshold_low,
                                          double threshold_medium,
                                          double threshold_high,
                                          double threshold_critical);

/**
 * @brief Get the detection thresholds
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param threshold_low Pointer to store low severity threshold
 * @param threshold_medium Pointer to store medium severity threshold
 * @param threshold_high Pointer to store high severity threshold
 * @param threshold_critical Pointer to store critical severity threshold
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_thresholds(caeds_anomaly_context_t context,
                                          caeds_flux_source_t source,
                                          double* threshold_low,
                                          double* threshold_medium,
                                          double* threshold_high,
                                          double* threshold_critical);

/**
 * @brief Get the current anomaly score
 * 
 * @param context Anomaly detection context
 * @param source Entropy source
 * @param score Pointer to store anomaly score
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_get_score(caeds_anomaly_context_t context,
                                      caeds_flux_source_t source,
                                      double* score);

/**
 * @brief Shutdown the anomaly detection system
 * 
 * @param context Anomaly detection context
 * @return charm_status_t Status code
 */
charm_status_t caeds_anomaly_shutdown(caeds_anomaly_context_t context);

/**
 * @brief Get string representation of anomaly severity
 * 
 * @param severity Anomaly severity
 * @return const char* String representation
 */
const char* caeds_anomaly_severity_to_string(caeds_anomaly_severity_t severity);

/**
 * @brief Get string representation of anomaly model type
 * 
 * @param model Anomaly model type
 * @return const char* String representation
 */
const char* caeds_anomaly_model_to_string(caeds_anomaly_model_t model);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_ANOMALY_H */
