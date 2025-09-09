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

#ifndef CHARM_ML_TYPES_H
#define CHARM_ML_TYPES_H

/**
 * @file charm_ml_types.h
 * @brief Machine learning type definitions for CHARM system
 * 
 * This header defines the machine learning data structures and types
 * used by CAEDS for anomaly detection and predictive analytics.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum feature vector dimension
 */
#define CHARM_ML_MAX_FEATURES 32

/**
 * @brief Feature vector structure
 * 
 * Contains a vector of features for machine learning algorithms
 */
typedef struct {
    double values[CHARM_ML_MAX_FEATURES]; /**< Feature values */
    size_t dimension;                     /**< Number of features */
    uint32_t flags;                       /**< Feature flags */
    
    /* Statistical features */
    double shannon_entropy;               /**< Shannon entropy value */
    double chi_squared;                   /**< Chi-squared test result */
    double monte_carlo_pi;                /**< Monte Carlo PI approximation */
    double serial_correlation;            /**< Serial correlation coefficient */
    double entropy_rate;                  /**< Rate of entropy change */
    
    /* Distribution features */
    double pattern_frequency[16];         /**< Pattern frequency distribution */
    double byte_distribution[16];         /**< Byte value distribution */
} charm_ml_feature_vector_t;

/**
 * @brief Model types
 */
typedef enum {
    CHARM_ML_MODEL_ANOMALY,    /**< Anomaly detection model */
    CHARM_ML_MODEL_PREDICTION, /**< Prediction model */
    CHARM_ML_MODEL_CLASSIFIER  /**< Classification model */
} charm_ml_model_type_t;

/**
 * @brief Training parameters
 */
typedef struct {
    size_t epochs;             /**< Number of training epochs */
    double learning_rate;      /**< Learning rate */
    double regularization;     /**< Regularization parameter */
    size_t batch_size;         /**< Batch size */
    bool shuffle;              /**< Whether to shuffle data */
    uint32_t flags;            /**< Training flags */
} charm_ml_training_params_t;

/**
 * @brief Model parameters
 */
typedef struct {
    charm_ml_model_type_t type;  /**< Model type */
    size_t input_dimension;      /**< Input dimension */
    size_t output_dimension;     /**< Output dimension */
    size_t hidden_layers;        /**< Number of hidden layers */
    size_t hidden_dimension;     /**< Hidden layer dimension */
    double dropout_rate;         /**< Dropout rate */
    char activation[16];         /**< Activation function name */
    uint32_t flags;              /**< Model flags */
} charm_ml_model_params_t;

/**
 * @brief Prediction result
 */
typedef struct {
    double values[CHARM_ML_MAX_FEATURES]; /**< Predicted values */
    double confidence;                    /**< Prediction confidence */
    size_t dimension;                     /**< Number of predictions */
    uint32_t flags;                       /**< Result flags */
} charm_ml_prediction_t;

/**
 * @brief Anomaly detection result
 */
typedef struct {
    double score;                /**< Anomaly score */
    double threshold;            /**< Detection threshold */
    bool is_anomaly;             /**< Whether an anomaly was detected */
    char description[128];       /**< Anomaly description */
    uint64_t timestamp;          /**< Detection timestamp */
    uint32_t flags;              /**< Result flags */
} charm_ml_anomaly_t;

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ML_TYPES_H */
