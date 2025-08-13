/**
 * @file caeds_predict.c
 * @brief Implementation of entropy trend forecasting for the CAEDS subsystem
 * 
 * This file implements the entropy trend forecasting system for the CAEDS
 * (Charm Advanced Entropy Diagnostic System) subsystem, providing mechanisms
 * to predict future entropy trends and quality.
 */

#include "caeds_predict.h"
#include "charm_status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

/**
 * @brief Maximum number of callbacks
 */
#define CAEDS_PREDICT_MAX_CALLBACKS 16

/**
 * @brief Maximum history size
 */
#define CAEDS_PREDICT_MAX_HISTORY 10000

/**
 * @brief Default forecast steps for different horizons
 */
#define CAEDS_PREDICT_STEPS_SHORT 10
#define CAEDS_PREDICT_STEPS_MEDIUM 60
#define CAEDS_PREDICT_STEPS_LONG 360

/**
 * @brief Confidence z-scores for different confidence levels
 */
#define CAEDS_PREDICT_Z_LOW 1.0    /* ~68% confidence */
#define CAEDS_PREDICT_Z_MEDIUM 1.96 /* ~95% confidence */
#define CAEDS_PREDICT_Z_HIGH 2.58   /* ~99% confidence */

/**
 * @brief Model parameters for EWMA
 */
typedef struct {
    double alpha;                /**< Smoothing factor */
    double ewma;                 /**< Current EWMA value */
    double ewmvar;               /**< Current EWMA variance */
    double* forecast;            /**< Forecast buffer */
    size_t forecast_size;        /**< Size of forecast buffer */
} caeds_predict_ewma_params_t;

/**
 * @brief Model parameters for ARIMA
 */
typedef struct {
    double* ar_coef;             /**< AR coefficients */
    size_t ar_order;             /**< AR order (p) */
    double* ma_coef;             /**< MA coefficients */
    size_t ma_order;             /**< MA order (q) */
    int diff_order;              /**< Differencing order (d) */
    double* residuals;           /**< Residual errors */
    size_t residual_size;        /**< Size of residual buffer */
    double* forecast;            /**< Forecast buffer */
    size_t forecast_size;        /**< Size of forecast buffer */
    double intercept;            /**< Model intercept */
} caeds_predict_arima_params_t;

/**
 * @brief Model parameters for Kalman filter
 */
typedef struct {
    double x;                    /**< State estimate */
    double p;                    /**< Estimation error covariance */
    double q;                    /**< Process noise covariance */
    double r;                    /**< Measurement noise covariance */
    double k;                    /**< Kalman gain */
    double* forecast;            /**< Forecast buffer */
    size_t forecast_size;        /**< Size of forecast buffer */
} caeds_predict_kalman_params_t;

/**
 * @brief Model parameters for adaptive prediction
 */
typedef struct {
    double* history;             /**< History buffer */
    size_t history_size;         /**< Size of history buffer */
    size_t history_index;        /**< Current index in history buffer */
    size_t history_count;        /**< Number of samples in history */
    double mean;                 /**< Current mean */
    double std_dev;              /**< Current standard deviation */
    double trend;                /**< Current trend */
    double* forecast;            /**< Forecast buffer */
    size_t forecast_size;        /**< Size of forecast buffer */
    double learning_rate;        /**< Learning rate */
} caeds_predict_adaptive_params_t;

/**
 * @brief Model parameters union
 */
typedef union {
    caeds_predict_ewma_params_t ewma;
    caeds_predict_arima_params_t arima;
    caeds_predict_kalman_params_t kalman;
    caeds_predict_adaptive_params_t adaptive;
} caeds_predict_model_params_t;

/**
 * @brief Source-specific prediction state
 */
typedef struct {
    caeds_predict_model_t model;
    caeds_predict_model_params_t params;
    double* history;
    size_t history_size;
    size_t history_index;
    size_t history_count;
    uint64_t last_update;
    double last_value;
    double accuracy;
    uint64_t prediction_count;
    uint64_t correct_predictions;
    bool trained;
} caeds_predict_source_state_t;

/**
 * @brief Callback registration
 */
typedef struct {
    caeds_predict_callback_t callback;
    void* user_data;
    bool active;
} caeds_predict_callback_reg_t;

/**
 * @brief Internal context structure
 */
struct caeds_predict_context_s {
    caeds_flux_context_t flux_context;
    caeds_predict_source_state_t sources[CAEDS_FLUX_SOURCE_COUNT];
    caeds_predict_callback_reg_t callbacks[CAEDS_PREDICT_MAX_CALLBACKS];
    uint32_t callback_count;
    pthread_mutex_t mutex;
    uint32_t min_samples;
    bool auto_train;
    double learning_rate;
    uint32_t forecast_steps;
};

/**
 * @brief Default configuration
 */
const caeds_predict_config_t CAEDS_PREDICT_DEFAULT_CONFIG = {
    .model = CAEDS_PREDICT_MODEL_ADAPTIVE,
    .history_size = 1000,
    .forecast_steps = 60,
    .learning_rate = 0.01,
    .min_samples = 100,
    .auto_train = true
};

/**
 * @brief Get current timestamp in milliseconds
 * 
 * @return uint64_t Timestamp
 */
static uint64_t caeds_predict_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

/**
 * @brief Initialize EWMA model parameters
 * 
 * @param params Model parameters
 * @param forecast_steps Number of forecast steps
 */
static void caeds_predict_init_ewma(caeds_predict_ewma_params_t* params, uint32_t forecast_steps) {
    params->alpha = 0.1;
    params->ewma = 0.0;
    params->ewmvar = 0.0;
    params->forecast_size = forecast_steps;
    params->forecast = (double*)calloc(params->forecast_size, sizeof(double));
}

/**
 * @brief Initialize ARIMA model parameters
 * 
 * @param params Model parameters
 * @param forecast_steps Number of forecast steps
 */
static void caeds_predict_init_arima(caeds_predict_arima_params_t* params, uint32_t forecast_steps) {
    // Simple AR(1) model by default
    params->ar_order = 1;
    params->ar_coef = (double*)calloc(params->ar_order, sizeof(double));
    params->ar_coef[0] = 0.8; // Default AR coefficient
    
    params->ma_order = 0;
    params->ma_coef = NULL;
    
    params->diff_order = 0;
    
    params->residual_size = 10;
    params->residuals = (double*)calloc(params->residual_size, sizeof(double));
    
    params->forecast_size = forecast_steps;
    params->forecast = (double*)calloc(params->forecast_size, sizeof(double));
    
    params->intercept = 127.5; // Default intercept (middle of byte range)
}

/**
 * @brief Initialize Kalman filter model parameters
 * 
 * @param params Model parameters
 * @param forecast_steps Number of forecast steps
 */
static void caeds_predict_init_kalman(caeds_predict_kalman_params_t* params, uint32_t forecast_steps) {
    params->x = 127.5; // Initial state estimate (middle of byte range)
    params->p = 100.0; // Initial estimation error covariance
    params->q = 0.01;  // Process noise covariance
    params->r = 1.0;   // Measurement noise covariance
    params->k = 0.0;   // Kalman gain
    
    params->forecast_size = forecast_steps;
    params->forecast = (double*)calloc(params->forecast_size, sizeof(double));
}

/**
 * @brief Initialize adaptive prediction model parameters
 * 
 * @param params Model parameters
 * @param history_size History size
 * @param forecast_steps Number of forecast steps
 * @param learning_rate Learning rate
 */
static void caeds_predict_init_adaptive(caeds_predict_adaptive_params_t* params, 
                                      size_t history_size,
                                      uint32_t forecast_steps,
                                      double learning_rate) {
    if (history_size > CAEDS_PREDICT_MAX_HISTORY) {
        history_size = CAEDS_PREDICT_MAX_HISTORY;
    }
    
    params->history_size = history_size;
    params->history = (double*)calloc(history_size, sizeof(double));
    params->history_index = 0;
    params->history_count = 0;
    params->mean = 127.5; // Middle of byte range
    params->std_dev = 73.9; // Standard deviation for uniform distribution over [0, 255]
    params->trend = 0.0;
    
    params->forecast_size = forecast_steps;
    params->forecast = (double*)calloc(params->forecast_size, sizeof(double));
    
    params->learning_rate = learning_rate;
}

/**
 * @brief Clean up model parameters
 * 
 * @param state Source state
 */
static void caeds_predict_cleanup_model(caeds_predict_source_state_t* state) {
    switch (state->model) {
        case CAEDS_PREDICT_MODEL_EWMA:
            if (state->params.ewma.forecast) {
                free(state->params.ewma.forecast);
                state->params.ewma.forecast = NULL;
            }
            break;
            
        case CAEDS_PREDICT_MODEL_ARIMA:
            if (state->params.arima.ar_coef) {
                free(state->params.arima.ar_coef);
                state->params.arima.ar_coef = NULL;
            }
            if (state->params.arima.ma_coef) {
                free(state->params.arima.ma_coef);
                state->params.arima.ma_coef = NULL;
            }
            if (state->params.arima.residuals) {
                free(state->params.arima.residuals);
                state->params.arima.residuals = NULL;
            }
            if (state->params.arima.forecast) {
                free(state->params.arima.forecast);
                state->params.arima.forecast = NULL;
            }
            break;
            
        case CAEDS_PREDICT_MODEL_KALMAN:
            if (state->params.kalman.forecast) {
                free(state->params.kalman.forecast);
                state->params.kalman.forecast = NULL;
            }
            break;
            
        case CAEDS_PREDICT_MODEL_ADAPTIVE:
            if (state->params.adaptive.history) {
                free(state->params.adaptive.history);
                state->params.adaptive.history = NULL;
            }
            if (state->params.adaptive.forecast) {
                free(state->params.adaptive.forecast);
                state->params.adaptive.forecast = NULL;
            }
            break;
    }
    
    if (state->history) {
        free(state->history);
        state->history = NULL;
    }
}

/**
 * @brief Update EWMA model and generate forecast
 * 
 * @param state Source state
 * @param value New value
 */
static void caeds_predict_update_ewma(caeds_predict_source_state_t* state, double value) {
    caeds_predict_ewma_params_t* params = &state->params.ewma;
    
    // Update EWMA
    double delta = value - params->ewma;
    params->ewma = params->ewma + params->alpha * delta;
    
    // Update EWMA variance
    params->ewmvar = (1 - params->alpha) * (params->ewmvar + params->alpha * delta * delta);
    
    // Generate forecast
    for (size_t i = 0; i < params->forecast_size; i++) {
        params->forecast[i] = params->ewma;
    }
}

/**
 * @brief Update ARIMA model and generate forecast
 * 
 * @param state Source state
 * @param value New value
 */
static void caeds_predict_update_arima(caeds_predict_source_state_t* state, double value) {
    caeds_predict_arima_params_t* params = &state->params.arima;
    
    // Shift history for AR terms
    for (size_t i = state->history_size - 1; i > 0; i--) {
        state->history[i] = state->history[i-1];
    }
    state->history[0] = value;
    
    // Shift residuals for MA terms
    for (size_t i = params->residual_size - 1; i > 0; i--) {
        params->residuals[i] = params->residuals[i-1];
    }
    
    // Calculate prediction for current step
    double prediction = params->intercept;
    
    // Add AR terms
    for (size_t i = 0; i < params->ar_order && i < state->history_count - 1; i++) {
        prediction += params->ar_coef[i] * state->history[i+1];
    }
    
    // Add MA terms
    for (size_t i = 0; i < params->ma_order && i < params->residual_size - 1; i++) {
        prediction += params->ma_coef[i] * params->residuals[i+1];
    }
    
    // Calculate residual
    params->residuals[0] = value - prediction;
    
    // Generate forecast
    double forecast_value = params->intercept;
    
    // First forecast step
    for (size_t i = 0; i < params->ar_order && i < state->history_count; i++) {
        forecast_value += params->ar_coef[i] * state->history[i];
    }
    
    for (size_t i = 0; i < params->ma_order && i < params->residual_size; i++) {
        forecast_value += params->ma_coef[i] * params->residuals[i];
    }
    
    params->forecast[0] = forecast_value;
    
    // Remaining forecast steps
    for (size_t step = 1; step < params->forecast_size; step++) {
        forecast_value = params->intercept;
        
        // AR terms
        for (size_t i = 0; i < params->ar_order && i < step; i++) {
            forecast_value += params->ar_coef[i] * params->forecast[step-i-1];
        }
        
        for (size_t i = 0; i < params->ar_order && i + step < state->history_count; i++) {
            forecast_value += params->ar_coef[i+step] * state->history[i];
        }
        
        // MA terms are zero for future steps (since we don't know future errors)
        
        params->forecast[step] = forecast_value;
    }
}

/**
 * @brief Update Kalman filter model and generate forecast
 * 
 * @param state Source state
 * @param value New value
 */
static void caeds_predict_update_kalman(caeds_predict_source_state_t* state, double value) {
    caeds_predict_kalman_params_t* params = &state->params.kalman;
    
    // Prediction step
    double x_pred = params->x;
    double p_pred = params->p + params->q;
    
    // Update step
    params->k = p_pred / (p_pred + params->r);
    params->x = x_pred + params->k * (value - x_pred);
    params->p = (1 - params->k) * p_pred;
    
    // Generate forecast
    for (size_t i = 0; i < params->forecast_size; i++) {
        params->forecast[i] = params->x;
    }
}

/**
 * @brief Update adaptive prediction model and generate forecast
 * 
 * @param state Source state
 * @param value New value
 */
static void caeds_predict_update_adaptive(caeds_predict_source_state_t* state, double value) {
    caeds_predict_adaptive_params_t* params = &state->params.adaptive;
    
    // Add to history
    params->history[params->history_index] = value;
    params->history_index = (params->history_index + 1) % params->history_size;
    
    if (params->history_count < params->history_size) {
        params->history_count++;
    }
    
    // Recalculate mean and standard deviation
    double sum = 0.0;
    double sum_sq = 0.0;
    
    for (size_t i = 0; i < params->history_count; i++) {
        sum += params->history[i];
        sum_sq += params->history[i] * params->history[i];
    }
    
    double new_mean = sum / params->history_count;
    double variance = (sum_sq / params->history_count) - (new_mean * new_mean);
    double new_std_dev = sqrt(variance > 0 ? variance : 0);
    
    // Calculate trend
    double new_trend = 0.0;
    if (params->history_count > 1) {
        // Simple linear trend based on first and last half of history
        size_t half = params->history_count / 2;
        double sum_first_half = 0.0;
        double sum_second_half = 0.0;
        
        for (size_t i = 0; i < half; i++) {
            sum_first_half += params->history[i];
        }
        
        for (size_t i = half; i < params->history_count; i++) {
            sum_second_half += params->history[i];
        }
        
        double mean_first_half = sum_first_half / half;
        double mean_second_half = sum_second_half / (params->history_count - half);
        
        new_trend = (mean_second_half - mean_first_half) / half;
    }
    
    // Update parameters with learning rate
    params->mean = params->mean * (1 - params->learning_rate) + new_mean * params->learning_rate;
    params->std_dev = params->std_dev * (1 - params->learning_rate) + new_std_dev * params->learning_rate;
    params->trend = params->trend * (1 - params->learni
(Content truncated due to size limit. Use line ranges to read in chunks)