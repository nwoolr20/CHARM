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
 * @file caeds_pae.h
 * @brief Predictive Analytics Engine for CAEDS
 * 
 * This header defines the interfaces for the Predictive Analytics Engine (PAE)
 * component of the CAEDS subsystem, responsible for forecasting entropy trends
 * and providing early warnings for potential entropy degradation.
 */

#ifndef CAEDS_PAE_H
#define CAEDS_PAE_H

#include "core/charm.h"
#include "ml/charm_ml.h"
#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief PAE status codes
 */
typedef enum {
    CHARM_PAE_SUCCESS = 0,
    CHARM_PAE_ERROR_INITIALIZATION = -1,
    CHARM_PAE_ERROR_INVALID_PARAMETER = -2,
    CHARM_PAE_ERROR_INSUFFICIENT_DATA = -3,
    CHARM_PAE_ERROR_MODEL_FAILURE = -4,
    CHARM_PAE_ERROR_NOT_INITIALIZED = -5
} charm_pae_status_t;

/**
 * @brief Prediction horizon types
 */
typedef enum {
    CHARM_HORIZON_SHORT = 0,   /**< Short-term prediction (seconds to minutes) */
    CHARM_HORIZON_MEDIUM,      /**< Medium-term prediction (minutes to hours) */
    CHARM_HORIZON_LONG         /**< Long-term prediction (hours to days) */
} charm_prediction_horizon_t;

/**
 * @brief Entropy trend prediction
 */
typedef struct {
    time_t timestamp;                  /**< Prediction timestamp */
    charm_prediction_horizon_t horizon; /**< Prediction horizon */
    charm_entropy_stats_t predicted_stats; /**< Predicted entropy statistics */
    double confidence;                 /**< Prediction confidence (0-1) */
    bool warning_flag;                 /**< Warning flag for potential issues */
    char warning_message[256];         /**< Warning message (if applicable) */
} charm_entropy_prediction_t;

/**
 * @brief PAE configuration options
 */
typedef struct {
    uint32_t history_window;           /**< Number of samples to use for prediction */
    uint32_t update_interval_ms;       /**< Interval between model updates in ms */
    bool enable_short_horizon;         /**< Enable short-term predictions */
    bool enable_medium_horizon;        /**< Enable medium-term predictions */
    bool enable_long_horizon;          /**< Enable long-term predictions */
    double warning_threshold;          /**< Threshold for issuing warnings (0-1) */
    const char* model_file;            /**< Path to pre-trained model file (optional) */
} charm_pae_config_t;

/**
 * @brief Default configuration for the PAE
 */
extern const charm_pae_config_t CHARM_PAE_DEFAULT_CONFIG;

/**
 * @brief Initialize the Predictive Analytics Engine
 * 
 * This function initializes the PAE with the specified configuration.
 * 
 * @param ctx CHARM context
 * @param config Pointer to configuration structure
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_init(charm_context ctx, const charm_pae_config_t* config);

/**
 * @brief Update the PAE with new entropy data
 * 
 * This function updates the PAE's internal models with new entropy data.
 * 
 * @param ctx CHARM context
 * @param stats Pointer to entropy statistics
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_update(charm_context ctx, const charm_entropy_stats_t* stats);

/**
 * @brief Generate entropy predictions
 * 
 * This function generates predictions for future entropy quality based on
 * historical data and current trends.
 * 
 * @param ctx CHARM context
 * @param horizon Prediction horizon
 * @param prediction Pointer to receive prediction
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_predict(charm_context ctx, 
                                    charm_prediction_horizon_t horizon,
                                    charm_entropy_prediction_t* prediction);

/**
 * @brief Register a callback for entropy warnings
 * 
 * This function registers a callback to be called when the PAE detects
 * a potential entropy issue in the future.
 * 
 * @param ctx CHARM context
 * @param callback Function to call on warnings
 * @param user_data User data to pass to callback
 * @return Status code indicating success or failure
 */
typedef void (*charm_pae_warning_callback)(void* user_data, 
                                          const charm_entropy_prediction_t* prediction);

charm_pae_status_t charm_pae_set_warning_callback(charm_context ctx,
                                                charm_pae_warning_callback callback,
                                                void* user_data);

/**
 * @brief Get the prediction accuracy metrics
 * 
 * This function returns metrics about the accuracy of previous predictions.
 * 
 * @param ctx CHARM context
 * @param horizon Prediction horizon
 * @param accuracy Pointer to receive accuracy value (0-1)
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_get_accuracy(charm_context ctx,
                                         charm_prediction_horizon_t horizon,
                                         double* accuracy);

/**
 * @brief Save the PAE model to file
 * 
 * This function saves the current PAE model to a file for later use.
 * 
 * @param ctx CHARM context
 * @param filename Path to output file
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_save_model(charm_context ctx, const char* filename);

/**
 * @brief Load a PAE model from file
 * 
 * This function loads a PAE model from a file.
 * 
 * @param ctx CHARM context
 * @param filename Path to model file
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_load_model(charm_context ctx, const char* filename);

/**
 * @brief Shutdown the PAE
 * 
 * This function shuts down the PAE, releasing all allocated resources.
 * 
 * @param ctx CHARM context
 * @return Status code indicating success or failure
 */
charm_pae_status_t charm_pae_shutdown(charm_context ctx);

/**
 * @brief Get a string description of a PAE status code
 * 
 * @param status Status code to describe
 * @return String description of the status code
 */
const char* charm_pae_status_string(charm_pae_status_t status);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_PAE_H */
