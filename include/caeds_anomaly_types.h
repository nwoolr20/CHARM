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

#ifndef CAEDS_ANOMALY_TYPES_H
#define CAEDS_ANOMALY_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Anomaly model type
 */
typedef enum {
    CAEDS_ANOMALY_MODEL_NONE = 0,     /**< No model */
    CAEDS_ANOMALY_MODEL_EWMA = 1,     /**< Exponentially Weighted Moving Average */
    CAEDS_ANOMALY_MODEL_CUSUM = 2,    /**< Cumulative Sum */
    CAEDS_ANOMALY_MODEL_ENTROPY = 3,  /**< Entropy-based */
    CAEDS_ANOMALY_MODEL_ADAPTIVE = 4, /**< Adaptive */
    CAEDS_ANOMALY_MODEL_CUSTOM = 5    /**< Custom model */
} caeds_anomaly_model_t;

/**
 * @brief EWMA parameters
 */
typedef struct {
    float lambda;                     /**< Smoothing factor */
    float threshold;                  /**< Detection threshold */
    float initial_mean;               /**< Initial mean */
    float initial_variance;           /**< Initial variance */
} caeds_anomaly_ewma_params_t;

/**
 * @brief CUSUM parameters
 */
typedef struct {
    float threshold;                  /**< Detection threshold */
    float drift;                      /**< Drift parameter */
    float target_mean;                /**< Target mean */
    float target_variance;            /**< Target variance */
} caeds_anomaly_cusum_params_t;

/**
 * @brief Entropy parameters
 */
typedef struct {
    float min_entropy;                /**< Minimum entropy */
    float max_entropy;                /**< Maximum entropy */
    uint32_t window_size;             /**< Window size */
    float threshold;                  /**< Detection threshold */
} caeds_anomaly_entropy_params_t;

/**
 * @brief Adaptive parameters
 */
typedef struct {
    float learning_rate;              /**< Learning rate */
    float forgetting_factor;          /**< Forgetting factor */
    uint32_t history_size;            /**< History size */
    float threshold;                  /**< Detection threshold */
} caeds_anomaly_adaptive_params_t;

/**
 * @brief Source state
 */
typedef struct {
    uint32_t id;                      /**< Source ID */
    uint32_t sample_count;            /**< Sample count */
    uint32_t anomaly_count;           /**< Anomaly count */
    uint64_t last_timestamp;          /**< Last timestamp */
    float quality;                    /**< Quality */
    float entropy;                    /**< Entropy */
    float mean;                       /**< Mean */
    float variance;                   /**< Variance */
    caeds_anomaly_model_t model;      /**< Model type */
    void* model_params;               /**< Model parameters */
    void* model_state;                /**< Model state */
    uint8_t* history;                 /**< Sample history */
    uint32_t history_size;            /**< History size */
    uint32_t history_index;           /**< History index */
} caeds_anomaly_source_state_t;

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_ANOMALY_TYPES_H */
