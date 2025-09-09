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
 * @file charm_ml_model.h
 * @brief ML model structure definition for CHARM system
 * 
 * This header defines the internal structure of ML models used by CAEDS
 * for anomaly detection and predictive analytics.
 */

#ifndef CHARM_ML_MODEL_H
#define CHARM_ML_MODEL_H

#include <stdint.h>
#include <stdbool.h>
#include "charm_ml_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ML model structure
 */
struct charm_ml_model {
    uint32_t sensitivity;        /**< Detection sensitivity (0-100) */
    uint32_t history_size;       /**< Number of samples to maintain in history */
    bool adaptive_learning;      /**< Whether to use adaptive learning */
    
    /* Model parameters */
    double weights[CHARM_ML_MAX_FEATURES];  /**< Feature weights */
    double bias;                            /**< Model bias */
    double threshold;                       /**< Detection threshold */
    
    /* Runtime state */
    uint64_t samples_processed;             /**< Number of samples processed */
    uint32_t anomalies_detected;            /**< Number of anomalies detected */
    uint32_t flags;                         /**< Model flags */
};

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ML_MODEL_H */
