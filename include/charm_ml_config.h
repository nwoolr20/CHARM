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
 * @file charm_ml_config.h
 * @brief ML configuration structure for CHARM system
 * 
 * This header defines the configuration structure for the ML subsystem
 * used by CAEDS for anomaly detection and predictive analytics.
 */

#ifndef CHARM_ML_CONFIG_H
#define CHARM_ML_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ML configuration structure
 */
typedef struct {
    uint32_t sensitivity;        /**< Detection sensitivity (0-100) */
    uint32_t history_size;       /**< Number of samples to maintain in history */
    bool adaptive_learning;      /**< Whether to use adaptive learning */
    const char* model_file;      /**< Path to pre-trained model file (optional) */
} charm_ml_config_t;

/**
 * @brief Default ML configuration
 */
extern const charm_ml_config_t CHARM_ML_DEFAULT_CONFIG;

#ifdef __cplusplus
}
#endif

#endif /* CHARM_ML_CONFIG_H */
