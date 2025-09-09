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
 * @file cee_mix.c
 * @brief Entropic Amplification Unit (EAU) - Non-linear chaotic amplification
 * 
 * This module implements the Entropic Amplification Unit (EAU) component of the
 * Charm Entropic Engine (CEE). It applies non-linear transformations and chaotic
 * mixing functions to raw entropy inputs to enhance unpredictability and
 * statistical properties.
 * 
 * The implementation uses principles from chaotic systems to break statistical
 * correlations and expand the effective entropy content of the input.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define CEE_MIX_BUFFER_SIZE 1024
#define CEE_MIX_ITERATIONS 16
#define CEE_MIX_CHAOTIC_PARAM 3.99 // Parameter for logistic map (near chaotic regime)

// Internal state for the chaotic mixing system
typedef struct {
    double* state;
    size_t state_size;
    uint64_t iteration_count;
    double r_param; // Bifurcation parameter for logistic map
} cee_mix_state_t;

// Global mixing state
static cee_mix_state_t g_mix_state = {NULL, 0, 0, CEE_MIX_CHAOTIC_PARAM};

/**
 * @brief Initialize the chaotic mixing system
 * 
 * @param size Size of the internal state buffer
 * @return int 0 on success, non-zero on failure
 */
int cee_mix_init(size_t size) {
    if (g_mix_state.state != NULL) {
        free(g_mix_state.state);
    }
    
    g_mix_state.state = (double*)malloc(size * sizeof(double));
    if (g_mix_state.state == NULL) {
        return -1;
    }
    
    g_mix_state.state_size = size;
    g_mix_state.iteration_count = 0;
    
    // Initialize with some entropy from time
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    srand(ts.tv_nsec);
    
    // Initialize state with values between 0.1 and 0.9
    // (avoiding edge cases in logistic map)
    for (size_t i = 0; i < size; i++) {
        g_mix_state.state[i] = 0.1 + 0.8 * ((double)rand() / RAND_MAX);
    }
    
    return 0;
}

/**
 * @brief Clean up the chaotic mixing system
 */
void cee_mix_cleanup(void) {
    if (g_mix_state.state != NULL) {
        free(g_mix_state.state);
        g_mix_state.state = NULL;
    }
    g_mix_state.state_size = 0;
    g_mix_state.iteration_count = 0;
}

/**
 * @brief Apply logistic map iteration to a value
 * 
 * The logistic map is a simple chaotic function: x_next = r * x * (1 - x)
 * When r is near 4.0, the behavior is chaotic.
 * 
 * @param x Current value (must be between 0 and 1)
 * @param r Bifurcation parameter (typically between 3.57 and 4.0 for chaos)
 * @return double Next value
 */
static double logistic_map(double x, double r) {
    return r * x * (1.0 - x);
}

/**
 * @brief Apply a non-linear transformation to the input data
 * 
 * This function applies multiple iterations of chaotic mixing to amplify
 * the entropy content of the input data.
 * 
 * @param input Input data buffer
 * @param output Output data buffer (can be the same as input)
 * @param size Size of the input/output buffers in bytes
 * @return int 0 on success, non-zero on failure
 */
int cee_mix_apply(const uint8_t* input, uint8_t* output, size_t size) {
    if (g_mix_state.state == NULL || g_mix_state.state_size == 0) {
        if (cee_mix_init(CEE_MIX_BUFFER_SIZE) != 0) {
            return -1;
        }
    }
    
    // Convert input bytes to doubles for chaotic mixing
    double* temp = (double*)malloc(size * sizeof(double));
    if (temp == NULL) {
        return -1;
    }
    
    // Normalize input bytes to [0,1] range
    for (size_t i = 0; i < size; i++) {
        temp[i] = input[i] / 255.0;
    }
    
    // Apply chaotic mixing iterations
    for (int iter = 0; iter < CEE_MIX_ITERATIONS; iter++) {
        // Mix with internal state
        for (size_t i = 0; i < size; i++) {
            size_t state_idx = i % g_mix_state.state_size;
            
            // Cross-couple the state and input
            double mixed = 0.5 * (temp[i] + g_mix_state.state[state_idx]);
            
            // Apply logistic map to both
            temp[i] = logistic_map(mixed, g_mix_state.r_param);
            g_mix_state.state[state_idx] = logistic_map(g_mix_state.state[state_idx], g_mix_state.r_param);
        }
        
        // Apply additional non-linear transformations
        for (size_t i = 1; i < size; i++) {
            // Couple adjacent values
            temp[i] = logistic_map(0.5 * (temp[i] + temp[i-1]), g_mix_state.r_param);
        }
        
        // Apply one more round of logistic map
        for (size_t i = 0; i < size; i++) {
            temp[i] = logistic_map(temp[i], g_mix_state.r_param);
        }
    }
    
    // Convert back to bytes
    for (size_t i = 0; i < size; i++) {
        // Ensure we stay in valid range [0,255]
        output[i] = (uint8_t)(255.0 * fmod(fabs(temp[i]), 1.0));
    }
    
    // Update internal state
    for (size_t i = 0; i < g_mix_state.state_size; i++) {
        g_mix_state.state[i] = logistic_map(g_mix_state.state[i], g_mix_state.r_param);
    }
    
    g_mix_state.iteration_count++;
    
    // Periodically adjust the chaotic parameter slightly to avoid cycles
    if (g_mix_state.iteration_count % 100 == 0) {
        // Small random adjustment to r_param, keeping it in chaotic regime
        double adjust = 0.01 * ((double)rand() / RAND_MAX - 0.5);
        g_mix_state.r_param = 3.9 + 0.09 * ((double)rand() / RAND_MAX);
    }
    
    free(temp);
    return 0;
}

/**
 * @brief Get the current entropy quality estimate
 * 
 * This function provides a rough estimate of the quality of the
 * entropy being produced by the mixing function.
 * 
 * @return double Value between 0.0 (poor) and 1.0 (excellent)
 */
double cee_mix_quality(void) {
    if (g_mix_state.state == NULL || g_mix_state.state_size == 0) {
        return 0.0;
    }
    
    // Simple quality metric based on state distribution
    double sum = 0.0;
    double sum_sq = 0.0;
    
    for (size_t i = 0; i < g_mix_state.state_size; i++) {
        sum += g_mix_state.state[i];
        sum_sq += g_mix_state.state[i] * g_mix_state.state[i];
    }
    
    double mean = sum / g_mix_state.state_size;
    double variance = (sum_sq / g_mix_state.state_size) - (mean * mean);
    
    // Ideal variance for uniform distribution in [0,1] is 1/12 ≈ 0.0833
    // We want variance to be close to this value
    double ideal_variance = 1.0 / 12.0;
    double variance_quality = 1.0 - fabs(variance - ideal_variance) / ideal_variance;
    
    // Penalize if mean is far from 0.5
    double mean_quality = 1.0 - 2.0 * fabs(mean - 0.5);
    
    // Combined quality metric
    return 0.7 * variance_quality + 0.3 * mean_quality;
}

/**
 * @brief Feed external entropy into the mixing system
 * 
 * This function allows external entropy sources to influence
 * the internal state of the mixing system.
 * 
 * @param data External entropy data
 * @param size Size of the data in bytes
 */
void cee_mix_feed_entropy(const uint8_t* data, size_t size) {
    if (g_mix_state.state == NULL || g_mix_state.state_size == 0) {
        if (cee_mix_init(CEE_MIX_BUFFER_SIZE) != 0) {
            return;
        }
    }
    
    // Incorporate external entropy into internal state
    for (size_t i = 0; i < size && i < g_mix_state.state_size; i++) {
        double normalized = data[i] / 255.0;
        g_mix_state.state[i] = logistic_map(0.5 * (g_mix_state.state[i] + normalized), g_mix_state.r_param);
    }
    
    // If we have more entropy than state size, fold it in
    if (size > g_mix_state.state_size) {
        for (size_t i = g_mix_state.state_size; i < size; i++) {
            size_t idx = i % g_mix_state.state_size;
            double normalized = data[i] / 255.0;
            g_mix_state.state[idx] = logistic_map(0.5 * (g_mix_state.state[idx] + normalized), g_mix_state.r_param);
        }
    }
}
