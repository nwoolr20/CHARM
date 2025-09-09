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

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Machine learning model type
 */
typedef enum {
    CHARM_ML_MODEL_TYPE_NONE = 0,    /**< No model */
    CHARM_ML_MODEL_TYPE_LINEAR = 1,  /**< Linear model */
    CHARM_ML_MODEL_TYPE_NEURAL = 2,  /**< Neural network model */
    CHARM_ML_MODEL_TYPE_FOREST = 3,  /**< Random forest model */
    CHARM_ML_MODEL_TYPE_SVM = 4,     /**< Support vector machine model */
    CHARM_ML_MODEL_TYPE_CUSTOM = 5   /**< Custom model */
} charm_ml_model_type_t;

/**
 * @brief Machine learning model parameters
 */
typedef struct {
    uint32_t input_dim;              /**< Input dimension */
    uint32_t output_dim;             /**< Output dimension */
    uint32_t hidden_layers;          /**< Number of hidden layers (for neural networks) */
    uint32_t hidden_dim;             /**< Hidden dimension (for neural networks) */
    float learning_rate;             /**< Learning rate */
    float regularization;            /**< Regularization parameter */
    uint32_t max_iterations;         /**< Maximum number of iterations */
    float convergence_threshold;     /**< Convergence threshold */
    bool use_bias;                   /**< Whether to use bias */
    charm_ml_model_type_t type;      /**< Model type */
} charm_ml_model_params_t;

/**
 * @brief Machine learning model state
 */
typedef enum {
    CHARM_ML_MODEL_STATE_UNINITIALIZED = 0,  /**< Model is uninitialized */
    CHARM_ML_MODEL_STATE_INITIALIZED = 1,     /**< Model is initialized */
    CHARM_ML_MODEL_STATE_TRAINING = 2,        /**< Model is training */
    CHARM_ML_MODEL_STATE_TRAINED = 3,         /**< Model is trained */
    CHARM_ML_MODEL_STATE_ERROR = 4            /**< Model is in error state */
} charm_ml_model_state_t;

/**
 * @brief Machine learning model
 */
typedef struct charm_ml_model {
    charm_ml_model_type_t type;      /**< Model type */
    charm_ml_model_state_t state;    /**< Model state */
    charm_ml_model_params_t params;  /**< Model parameters */
    void* model_data;                /**< Model-specific data */
    uint32_t model_data_size;        /**< Size of model-specific data */
    void* context;                   /**< User context */
} charm_ml_model_t;

/**
 * @brief Machine learning training data
 */
typedef struct {
    float* inputs;                   /**< Input data */
    float* outputs;                  /**< Output data */
    uint32_t num_samples;            /**< Number of samples */
    uint32_t input_dim;              /**< Input dimension */
    uint32_t output_dim;             /**< Output dimension */
} charm_ml_training_data_t;

/**
 * @brief Machine learning prediction result
 */
typedef struct {
    float* outputs;                  /**< Output data */
    uint32_t output_dim;             /**< Output dimension */
    float confidence;                /**< Prediction confidence (0.0-1.0) */
} charm_ml_prediction_t;

/**
 * @brief Machine learning callback function
 */
typedef void (*charm_ml_callback_t)(charm_ml_model_t* model, void* user_data);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ML_TYPES_H */
