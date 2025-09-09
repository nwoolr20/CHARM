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
 * @file caeds_predict.h
 * @brief Entropy trend forecasting for the CAEDS subsystem
 * 
 * This header defines the entropy trend forecasting system for the CAEDS
 * (Charm Advanced Entropy Diagnostic System) subsystem, providing mechanisms
 * to predict future entropy trends and quality.
 */

#ifndef CAEDS_PREDICT_H
#define CAEDS_PREDICT_H

#include "charm_status.h"
#include "caeds_flux.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Prediction model types
 */
typedef enum {
    CAEDS_PREDICT_MODEL_EWMA = 0,        /**< Exponentially Weighted Moving Average */
    CAEDS_PREDICT_MODEL_ARIMA = 1,       /**< AutoRegressive Integrated Moving Average */
    CAEDS_PREDICT_MODEL_KALMAN = 2,      /**< Kalman Filter */
    CAEDS_PREDICT_MODEL_ADAPTIVE = 3     /**< Adaptive prediction */
} caeds_predict_model_t;

/**
 * @brief Prediction horizon
 */
typedef enum {
    CAEDS_PREDICT_HORIZON_SHORT = 0,     /**< Short-term prediction (seconds) */
    CAEDS_PREDICT_HORIZON_MEDIUM = 1,    /**< Medium-term prediction (minutes) */
    CAEDS_PREDICT_HORIZON_LONG = 2       /**< Long-term prediction (hours) */
} caeds_predict_horizon_t;

/**
 * @brief Prediction confidence levels
 */
typedef enum {
    CAEDS_PREDICT_CONFIDENCE_LOW = 0,    /**< Low confidence (68%) */
    CAEDS_PREDICT_CONFIDENCE_MEDIUM = 1, /**< Medium confidence (95%) */
    CAEDS_PREDICT_CONFIDENCE_HIGH = 2    /**< High confidence (99%) */
} caeds_predict_confidence_t;

/**
 * @brief Prediction configuration
 */
typedef struct {
    caeds_predict_model_t model;         /**< Prediction model type */
    uint32_t history_size;               /**< History size for training */
    uint32_t forecast_steps;             /**< Number of steps to forecast */
    double learning_rate;                /**< Learning rate for adaptive models */
    uint32_t min_samples;                /**< Minimum samples before prediction */
    bool auto_train;                     /**< Automatically train model */
} caeds_predict_config_t;

/**
 * @brief Prediction context handle
 */
typedef struct caeds_predict_context_s* caeds_predict_context_t;

/**
 * @brief Prediction result
 */
typedef struct {
    double forecast;                     /**< Forecasted entropy value */
    double lower_bound;                  /**< Lower confidence bound */
    double upper_bound;                  /**< Upper confidence bound */
    double confidence;                   /**< Confidence level (0.0-1.0) */
    uint64_t timestamp;                  /**< Prediction timestamp */
    caeds_flux_source_t source;          /**< Entropy source */
    uint32_t horizon_steps;              /**< Steps into the future */
} caeds_predict_result_t;

/**
 * @brief Prediction callback function type
 * 
 * @param result Prediction result
 * @param user_data User-defined data
 */
typedef void (*caeds_predict_callback_t)(const caeds_predict_result_t* result, void* user_data);

/**
 * @brief Default configuration
 */
extern const caeds_predict_config_t CAEDS_PREDICT_DEFAULT_CONFIG;

/**
 * @brief Initialize the prediction system
 * 
 * @param context Pointer to store the prediction context
 * @param flux_context Entropy flux context
 * @param config Configuration parameters (NULL for default)
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_init(caeds_predict_context_t* context,
                                 caeds_flux_context_t flux_context,
                                 const caeds_predict_config_t* config);

/**
 * @brief Register a callback for predictions
 * 
 * @param context Prediction context
 * @param callback Callback function
 * @param user_data User-defined data
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_register_callback(caeds_predict_context_t context,
                                             caeds_predict_callback_t callback,
                                             void* user_data);

/**
 * @brief Process entropy samples for prediction
 * 
 * @param context Prediction context
 * @param source Entropy source
 * @param samples Entropy samples
 * @param count Number of samples
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_process(caeds_predict_context_t context,
                                    caeds_flux_source_t source,
                                    const uint8_t* samples,
                                    size_t count);

/**
 * @brief Process entropy volatility for prediction
 * 
 * @param context Prediction context
 * @param source Entropy source
 * @param volatility Entropy volatility
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_process_volatility(caeds_predict_context_t context,
                                              caeds_flux_source_t source,
                                              const caeds_flux_volatility_t* volatility);

/**
 * @brief Generate a prediction
 * 
 * @param context Prediction context
 * @param source Entropy source
 * @param horizon Prediction horizon
 * @param confidence Confidence level
 * @param result Pointer to store prediction result
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_forecast(caeds_predict_context_t context,
                                     caeds_flux_source_t source,
                                     caeds_predict_horizon_t horizon,
                                     caeds_predict_confidence_t confidence,
                                     caeds_predict_result_t* result);

/**
 * @brief Train the prediction model
 * 
 * @param context Prediction context
 * @param source Entropy source
 * @param samples Training samples
 * @param count Number of samples
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_train(caeds_predict_context_t context,
                                  caeds_flux_source_t source,
                                  const uint8_t* samples,
                                  size_t count);

/**
 * @brief Reset the prediction model
 * 
 * @param context Prediction context
 * @param source Entropy source (CAEDS_FLUX_SOURCE_COUNT for all sources)
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_reset(caeds_predict_context_t context,
                                  caeds_flux_source_t source);

/**
 * @brief Get the prediction accuracy
 * 
 * @param context Prediction context
 * @param source Entropy source
 * @param accuracy Pointer to store accuracy (0.0-1.0)
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_get_accuracy(caeds_predict_context_t context,
                                         caeds_flux_source_t source,
                                         double* accuracy);

/**
 * @brief Shutdown the prediction system
 * 
 * @param context Prediction context
 * @return charm_status_t Status code
 */
charm_status_t caeds_predict_shutdown(caeds_predict_context_t context);

/**
 * @brief Get string representation of prediction model
 * 
 * @param model Prediction model type
 * @return const char* String representation
 */
const char* caeds_predict_model_to_string(caeds_predict_model_t model);

/**
 * @brief Get string representation of prediction horizon
 * 
 * @param horizon Prediction horizon
 * @return const char* String representation
 */
const char* caeds_predict_horizon_to_string(caeds_predict_horizon_t horizon);

/**
 * @brief Get string representation of prediction confidence
 * 
 * @param confidence Prediction confidence
 * @return const char* String representation
 */
const char* caeds_predict_confidence_to_string(caeds_predict_confidence_t confidence);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_PREDICT_H */
