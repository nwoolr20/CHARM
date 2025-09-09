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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "charm.h"
#include "caeds_anomaly.h"

/**
 * caeds_anomaly.c - CHARM Anomaly Detection Module
 * 
 * This module is responsible for detecting anomalies in entropy sources
 * and providing alerts when entropy quality degrades. It uses statistical
 * analysis and pattern recognition to identify potential weaknesses.
 */

/* Internal constants */
#define ANOMALY_WINDOW_SIZE 4096
#define ANOMALY_THRESHOLD_DEFAULT 0.85
#define MAX_ANOMALY_SOURCES 16
#define PATTERN_BUFFER_SIZE 256

/* Anomaly detection state */
typedef struct {
    uint32_t source_id;
    char source_name[64];
    uint32_t anomaly_count;
    double last_score;
    uint8_t pattern_buffer[PATTERN_BUFFER_SIZE];
    size_t pattern_index;
    bool active;
} anomaly_source_t;

/* Module state */
static struct {
    anomaly_source_t sources[MAX_ANOMALY_SOURCES];
    uint32_t source_count;
    double threshold;
    bool initialized;
} anomaly_state = {
    .source_count = 0,
    .threshold = ANOMALY_THRESHOLD_DEFAULT,
    .initialized = false
};

/**
 * Initialize the anomaly detection module
 */
int caeds_anomaly_init(void) {
    if (anomaly_state.initialized) {
        return CHARM_SUCCESS;
    }
    
    memset(&anomaly_state.sources, 0, sizeof(anomaly_state.sources));
    anomaly_state.source_count = 0;
    anomaly_state.threshold = ANOMALY_THRESHOLD_DEFAULT;
    anomaly_state.initialized = true;
    
    return CHARM_SUCCESS;
}

/**
 * Register an entropy source for anomaly detection
 */
int caeds_anomaly_register_source(uint32_t source_id, const char *name) {
    if (!anomaly_state.initialized) {
        return CHARM_ERROR_NOT_INITIALIZED;
    }
    
    if (anomaly_state.source_count >= MAX_ANOMALY_SOURCES) {
        return CHARM_ERROR_INVALID_PARAMETER;
    }
    
    // Check if source already exists
    for (uint32_t i = 0; i < anomaly_state.source_count; i++) {
        if (anomaly_state.sources[i].source_id == source_id) {
            // Update name if different
            if (strcmp(anomaly_state.sources[i].source_name, name) != 0) {
                strncpy(anomaly_state.sources[i].source_name, name, sizeof(anomaly_state.sources[i].source_name) - 1);
                anomaly_state.sources[i].source_name[sizeof(anomaly_state.sources[i].source_name) - 1] = '\0';
            }
            return CHARM_SUCCESS;
        }
    }
    
    // Add new source
    anomaly_source_t *source = &anomaly_state.sources[anomaly_state.source_count];
    source->source_id = source_id;
    strncpy(source->source_name, name, sizeof(source->source_name) - 1);
    source->source_name[sizeof(source->source_name) - 1] = '\0';
    source->anomaly_count = 0;
    source->last_score = 1.0;
    memset(source->pattern_buffer, 0, PATTERN_BUFFER_SIZE);
    source->pattern_index = 0;
    source->active = true;
    
    anomaly_state.source_count++;
    
    return CHARM_SUCCESS;
}

/**
 * Calculate entropy score for a buffer
 * Returns a value between 0.0 (completely predictable) and 1.0 (perfectly random)
 */
static double calculate_entropy_score(const uint8_t *data, size_t length) {
    if (length == 0) {
        return 0.0;
    }
    
    // Count occurrences of each byte value
    uint32_t counts[256] = {0};
    for (size_t i = 0; i < length; i++) {
        counts[data[i]]++;
    }
    
    // Calculate Shannon entropy
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            double probability = (double)counts[i] / length;
            entropy -= probability * log2(probability);
        }
    }
    
    // Normalize to [0,1] range (max entropy for bytes is 8 bits)
    return entropy / 8.0;
}

/**
 * Detect repeating patterns in data
 * Returns true if patterns are found
 */
static bool detect_patterns(const uint8_t *data, size_t length) {
    if (length < 8) {
        return false;
    }
    
    // Check for repeating byte patterns (2, 4, 8 byte sequences)
    for (size_t pattern_len = 2; pattern_len <= 8; pattern_len *= 2) {
        for (size_t i = 0; i < length - pattern_len * 4; i++) {
            bool match = true;
            
            // Check if we have 4 repetitions of the pattern
            for (size_t j = 0; j < pattern_len * 3; j++) {
                if (data[i + j] != data[i + j + pattern_len]) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                return true;
            }
        }
    }
    
    return false;
}

/**
 * Analyze entropy sample for anomalies
 * Returns anomaly score (0.0 = no anomalies, 1.0 = severe anomalies)
 */
double caeds_anomaly_analyze(uint32_t source_id, const uint8_t *data, size_t length) {
    if (!anomaly_state.initialized) {
        return 1.0;
    }
    
    // Find source
    anomaly_source_t *source = NULL;
    for (uint32_t i = 0; i < anomaly_state.source_count; i++) {
        if (anomaly_state.sources[i].source_id == source_id) {
            source = &anomaly_state.sources[i];
            break;
        }
    }
    
    if (source == NULL) {
        return 1.0;
    }
    
    // Calculate entropy score
    double entropy_score = calculate_entropy_score(data, length);
    
    // Update pattern buffer (circular buffer)
    size_t copy_size = (length < PATTERN_BUFFER_SIZE) ? length : PATTERN_BUFFER_SIZE;
    if (copy_size + source->pattern_index <= PATTERN_BUFFER_SIZE) {
        memcpy(source->pattern_buffer + source->pattern_index, data, copy_size);
        source->pattern_index += copy_size;
    } else {
        // Handle wrap-around
        size_t first_chunk = PATTERN_BUFFER_SIZE - source->pattern_index;
        size_t second_chunk = copy_size - first_chunk;
        
        memcpy(source->pattern_buffer + source->pattern_index, data, first_chunk);
        memcpy(source->pattern_buffer, data + first_chunk, second_chunk);
        source->pattern_index = second_chunk;
    }
    
    // Check for patterns
    bool has_patterns = detect_patterns(source->pattern_buffer, PATTERN_BUFFER_SIZE);
    
    // Calculate anomaly score (inverse of entropy score, adjusted for patterns)
    double anomaly_score = 1.0 - entropy_score;
    if (has_patterns) {
        anomaly_score = (anomaly_score + 1.0) / 2.0; // Increase score if patterns detected
    }
    
    // Update source state
    source->last_score = anomaly_score;
    if (anomaly_score > anomaly_state.threshold) {
        source->anomaly_count++;
    }
    
    return anomaly_score;
}

/**
 * Get the number of anomalies detected for a source
 */
uint32_t caeds_anomaly_get_count(uint32_t source_id) {
    if (!anomaly_state.initialized) {
        return 0;
    }
    
    for (uint32_t i = 0; i < anomaly_state.source_count; i++) {
        if (anomaly_state.sources[i].source_id == source_id) {
            return anomaly_state.sources[i].anomaly_count;
        }
    }
    
    return 0;
}

/**
 * Reset anomaly counters for a source
 */
int caeds_anomaly_reset(uint32_t source_id) {
    if (!anomaly_state.initialized) {
        return CHARM_ERROR_NOT_INITIALIZED;
    }
    
    for (uint32_t i = 0; i < anomaly_state.source_count; i++) {
        if (anomaly_state.sources[i].source_id == source_id) {
            anomaly_state.sources[i].anomaly_count = 0;
            return CHARM_SUCCESS;
        }
    }
    
    return CHARM_ERROR_INVALID_PARAMETER;
}

/**
 * Set the anomaly detection threshold
 */
int caeds_anomaly_set_threshold(double threshold) {
    if (threshold < 0.0 || threshold > 1.0) {
        return CHARM_ERROR_INVALID_PARAMETER;
    }
    
    anomaly_state.threshold = threshold;
    return CHARM_SUCCESS;
}

/**
 * Get the current anomaly detection threshold
 */
double caeds_anomaly_get_threshold(void) {
    return anomaly_state.threshold;
}

/**
 * Clean up the anomaly detection module
 */
void caeds_anomaly_cleanup(void) {
    memset(&anomaly_state.sources, 0, sizeof(anomaly_state.sources));
    anomaly_state.source_count = 0;
    anomaly_state.initialized = false;
}
