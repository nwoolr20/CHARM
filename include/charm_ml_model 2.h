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

#ifndef CHARM_ML_MODEL_H
#define CHARM_ML_MODEL_H

#include "charm_ml_types.h"
#include "charm_status.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize a machine learning model
 * 
 * @param model Pointer to model structure
 * @param params Model parameters
 * @param context User context
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_init(charm_ml_model_t* model, const charm_ml_model_params_t* params, void* context);

/**
 * @brief Train a machine learning model
 * 
 * @param model Pointer to model structure
 * @param training_data Training data
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_train(charm_ml_model_t* model, const charm_ml_training_data_t* training_data);

/**
 * @brief Make a prediction using a machine learning model
 * 
 * @param model Pointer to model structure
 * @param inputs Input data
 * @param input_dim Input dimension
 * @param prediction Prediction result
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_predict(const charm_ml_model_t* model, const float* inputs, uint32_t input_dim, charm_ml_prediction_t* prediction);

/**
 * @brief Save a machine learning model to a file
 * 
 * @param model Pointer to model structure
 * @param filename Filename
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_save(const charm_ml_model_t* model, const char* filename);

/**
 * @brief Load a machine learning model from a file
 * 
 * @param model Pointer to model structure
 * @param filename Filename
 * @param context User context
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_load(charm_ml_model_t* model, const char* filename, void* context);

/**
 * @brief Get model parameters
 * 
 * @param model Pointer to model structure
 * @param params Pointer to parameters structure
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_get_params(const charm_ml_model_t* model, charm_ml_model_params_t* params);

/**
 * @brief Set model parameters
 * 
 * @param model Pointer to model structure
 * @param params Pointer to parameters structure
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_set_params(charm_ml_model_t* model, const charm_ml_model_params_t* params);

/**
 * @brief Register a callback function
 * 
 * @param model Pointer to model structure
 * @param callback Callback function
 * @param user_data User data
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_register_callback(charm_ml_model_t* model, charm_ml_callback_t callback, void* user_data);

/**
 * @brief Shutdown a machine learning model
 * 
 * @param model Pointer to model structure
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_model_shutdown(charm_ml_model_t* model);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ML_MODEL_H */
