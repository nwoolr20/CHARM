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

#ifndef CHARM_ML_H
#define CHARM_ML_H

/**
 * @file charm_ml.h
 * @brief Machine learning support for CHARM system
 * 
 * This header defines the machine learning interfaces used by CAEDS
 * for anomaly detection and predictive analytics.
 */

#include "charm_ml_types.h"
#include "charm_ml_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ML model handle
 */
typedef struct charm_ml_model* charm_ml_model_handle_t;

/**
 * @brief Initialize the ML subsystem with configuration
 * 
 * @param handle Pointer to store model handle
 * @param config Configuration parameters
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_init_with_config(charm_ml_model_handle_t* handle, const charm_ml_config_t* config);

/**
 * @brief Initialize the ML subsystem
 * 
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_init(void);

/**
 * @brief Shutdown the ML subsystem
 * 
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_shutdown(void);

/**
 * @brief Create a new ML model
 * 
 * @param type Model type
 * @param name Model name
 * @param config Configuration string (JSON format)
 * @param handle Pointer to store model handle
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_create_model(charm_ml_model_type_t type, const char* name, 
                         const char* config, charm_ml_model_handle_t* handle);

/**
 * @brief Destroy an ML model
 * 
 * @param handle Model handle
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_destroy_model(charm_ml_model_handle_t handle);

/**
 * @brief Train an ML model with feature vector
 * 
 * @param handle Model handle
 * @param features Feature vector
 * @param is_anomaly Whether the features represent an anomaly
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_train_with_features(charm_ml_model_handle_t handle, 
                               const charm_ml_feature_vector_t* features,
                               bool is_anomaly);

/**
 * @brief Detect anomalies in data
 * 
 * @param handle Model handle (must be CHARM_ML_MODEL_ANOMALY type)
 * @param data Input data
 * @param data_size Size of input data in bytes
 * @param scores Output anomaly scores (caller must allocate)
 * @param threshold Anomaly threshold
 * @return int Number of anomalies detected, negative on failure
 */
int charm_ml_detect_anomalies(charm_ml_model_handle_t handle, const float* data, 
                             size_t data_size, float* scores, float threshold);

/**
 * @brief Make predictions using a model
 * 
 * @param handle Model handle (must be CHARM_ML_MODEL_PREDICTION type)
 * @param data Input data
 * @param data_size Size of input data in bytes
 * @param predictions Output predictions (caller must allocate)
 * @param prediction_count Number of predictions to make
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_predict(charm_ml_model_handle_t handle, const float* data,
                    size_t data_size, float* predictions, size_t prediction_count);

/**
 * @brief Detect anomalies using machine learning model
 * 
 * @param model Model handle (must be CHARM_ML_MODEL_ANOMALY type)
 * @param features Feature vector
 * @param score Output anomaly score
 * @param is_anomaly Output anomaly flag
 * @return charm_status_t Status code
 */
charm_status_t charm_ml_detect_anomaly(charm_ml_model_handle_t model, 
                                      charm_ml_feature_vector_t* features,
                                      double* score,
                                      bool* is_anomaly);

/**
 * @brief Save a model to a file
 * 
 * @param handle Model handle
 * @param filename Filename to save to
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_save_model(charm_ml_model_handle_t handle, const char* filename);

/**
 * @brief Load a model from a file
 * 
 * @param handle Pointer to store model handle
 * @param filename Filename to load from
 * @return int 0 on success, non-zero on failure
 */
int charm_ml_load_model(charm_ml_model_handle_t* handle, const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ML_H */
