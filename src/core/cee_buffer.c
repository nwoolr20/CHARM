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
 * @file cee_buffer.c
 * @brief Buffer management for the Charm Entropic Engine
 * 
 * This file implements the buffer management system for the CEE (Charm Entropic
 * Engine) subsystem, providing thread-safe storage and retrieval of entropy
 * with backpressure routing and replay logging capabilities.
 */

#include "cee_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

// Forward declarations for entropy bus functions
extern int entropy_bus_init(void);
extern int entropy_bus_register_source(const char* name, double initial_quality);
extern int entropy_bus_push(int source_id, const uint8_t* data, size_t size, double quality);

/**
 * @brief Buffer segment structure
 */
typedef struct {
    uint8_t* data;                  /**< Segment data */
    size_t size;                    /**< Current size */
    size_t capacity;                /**< Segment capacity */
    cee_buffer_quality_t quality;   /**< Entropy quality level */
} cee_buffer_segment_t;

/**
 * @brief Worker thread context
 */
typedef struct {
    pthread_t thread;               /**< Thread handle */
    bool active;                    /**< Thread active flag */
    void* buffer_context;           /**< Pointer to buffer context */
} cee_buffer_worker_t;

/**
 * @brief Buffer context structure
 */
struct cee_buffer_context {
    // Buffer segments for different quality levels
    cee_buffer_segment_t segments[3];
    
    // Replay log
    uint8_t* replay_log;
    size_t replay_size;
    size_t replay_capacity;
    size_t replay_position;
    bool replay_enabled;
    
    // Statistics
    cee_buffer_stats_t stats;
    
    // Threading
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    cee_buffer_worker_t* workers;
    int min_threads;
    int max_threads;
    int active_threads;
    bool shutdown_requested;
    
    // Backpressure
    bool backpressure_enabled;
    double backpressure_threshold;
    
    // Entropy bus integration
    int bus_source_id;
    bool bus_registered;
};

/**
 * @brief Worker thread function
 * 
 * @param arg Worker context
 * @return void* NULL
 */
static void* cee_buffer_worker_thread(void* arg) {
    cee_buffer_worker_t* worker = (cee_buffer_worker_t*)arg;
    struct cee_buffer_context* ctx = (struct cee_buffer_context*)worker->buffer_context;
    
    // Log thread startup
    printf("Worker thread started with ID %lu\n", (unsigned long)pthread_self());
    
    while (worker->active) {
        // Check if we need to push entropy to the bus
        pthread_mutex_lock(&ctx->mutex);
        
        // Check for shutdown request
        if (ctx->shutdown_requested) {
            pthread_mutex_unlock(&ctx->mutex);
            printf("Worker thread %lu detected shutdown request, exiting\n", (unsigned long)pthread_self());
            break;
        }
        
        bool should_push = false;
        size_t push_size = 0;
        cee_buffer_quality_t push_quality = CEE_BUFFER_QUALITY_MEDIUM;
        
        // Determine if we should push entropy to the bus
        if (ctx->bus_registered) {
            // Check high quality segment first
            if (ctx->segments[CEE_BUFFER_QUALITY_HIGH].size > 0) {
                should_push = true;
                push_size = ctx->segments[CEE_BUFFER_QUALITY_HIGH].size;
                push_quality = CEE_BUFFER_QUALITY_HIGH;
            }
            // Then medium quality
            else if (ctx->segments[CEE_BUFFER_QUALITY_MEDIUM].size > 0) {
                should_push = true;
                push_size = ctx->segments[CEE_BUFFER_QUALITY_MEDIUM].size;
                push_quality = CEE_BUFFER_QUALITY_MEDIUM;
            }
            // Then low quality if we're desperate
            else if (ctx->segments[CEE_BUFFER_QUALITY_LOW].size > 0) {
                should_push = true;
                push_size = ctx->segments[CEE_BUFFER_QUALITY_LOW].size;
                push_quality = CEE_BUFFER_QUALITY_LOW;
            }
            
            // Limit push size to avoid flooding
            if (push_size > 1024) {
                push_size = 1024;
            }
        }
        
        // If we should push, prepare the data
        uint8_t* push_data = NULL;
        if (should_push && push_size > 0) {
            push_data = (uint8_t*)malloc(push_size);
            if (push_data) {
                // Copy data from the segment
                memcpy(push_data, ctx->segments[push_quality].data, push_size);
                
                // Remove the data from the segment
                memmove(ctx->segments[push_quality].data, 
                        ctx->segments[push_quality].data + push_size,
                        ctx->segments[push_quality].size - push_size);
                ctx->segments[push_quality].size -= push_size;
            } else {
                should_push = false;
            }
        }
        
        pthread_mutex_unlock(&ctx->mutex);
        
        // Push to the bus if needed
        if (should_push && push_data) {
            // Convert quality to a double (0.0-1.0)
            double quality_value = 0.0;
            switch (push_quality) {
                case CEE_BUFFER_QUALITY_LOW:
                    quality_value = 0.3;
                    break;
                case CEE_BUFFER_QUALITY_MEDIUM:
                    quality_value = 0.7;
                    break;
                case CEE_BUFFER_QUALITY_HIGH:
                    quality_value = 0.9;
                    break;
            }
            
            // Push to the bus
            entropy_bus_push(ctx->bus_source_id, push_data, push_size, quality_value);
            
            // Update statistics
            pthread_mutex_lock(&ctx->mutex);
            ctx->stats.bytes_retrieved += push_size;
            ctx->stats.operations_count++;
            pthread_mutex_unlock(&ctx->mutex);
            
            // Free the push data
            free(push_data);
        }
        
        // Sleep a bit to avoid spinning
        usleep(10000); // 10ms
    }
    
    printf("Worker thread %lu exiting\n", (unsigned long)pthread_self());
    return NULL;
}

/**
 * @brief Initialize the entropy buffer
 * 
 * @param config Buffer configuration
 * @return cee_buffer_handle_t Buffer handle or NULL on failure
 */
cee_buffer_handle_t cee_buffer_init(const cee_buffer_config_t* config) {
    if (!config || config->capacity == 0) {
        return NULL;
    }
    
    // Allocate context
    struct cee_buffer_context* ctx = (struct cee_buffer_context*)malloc(sizeof(struct cee_buffer_context));
    if (!ctx) {
        return NULL;
    }
    
    // Initialize context
    memset(ctx, 0, sizeof(struct cee_buffer_context));
    
    // Initialize mutex and condition variable
    if (pthread_mutex_init(&ctx->mutex, NULL) != 0) {
        free(ctx);
        return NULL;
    }
    
    if (pthread_cond_init(&ctx->cond, NULL) != 0) {
        pthread_mutex_destroy(&ctx->mutex);
        free(ctx);
        return NULL;
    }
    
    // Initialize segments
    size_t segment_capacity = config->capacity / 3;
    for (int i = 0; i < 3; i++) {
        ctx->segments[i].data = (uint8_t*)malloc(segment_capacity);
        if (!ctx->segments[i].data) {
            // Clean up previously allocated segments
            for (int j = 0; j < i; j++) {
                free(ctx->segments[j].data);
            }
            pthread_mutex_destroy(&ctx->mutex);
            pthread_cond_destroy(&ctx->cond);
            free(ctx);
            return NULL;
        }
        
        ctx->segments[i].size = 0;
        ctx->segments[i].capacity = segment_capacity;
        ctx->segments[i].quality = (cee_buffer_quality_t)i;
    }
    
    // Initialize replay log if enabled
    if (config->enable_replay && config->replay_capacity > 0) {
        ctx->replay_log = (uint8_t*)malloc(config->replay_capacity);
        if (!ctx->replay_log) {
            // Clean up segments
            for (int i = 0; i < 3; i++) {
                free(ctx->segments[i].data);
            }
            pthread_mutex_destroy(&ctx->mutex);
            pthread_cond_destroy(&ctx->cond);
            free(ctx);
            return NULL;
        }
        
        ctx->replay_capacity = config->replay_capacity;
        ctx->replay_size = 0;
        ctx->replay_position = 0;
        ctx->replay_enabled = true;
    } else {
        ctx->replay_log = NULL;
        ctx->replay_capacity = 0;
        ctx->replay_size = 0;
        ctx->replay_position = 0;
        ctx->replay_enabled = false;
    }
    
    // Initialize statistics
    ctx->stats.bytes_stored = 0;
    ctx->stats.bytes_retrieved = 0;
    ctx->stats.current_size = 0;
    ctx->stats.capacity = config->capacity;
    ctx->stats.replay_size = 0;
    ctx->stats.replay_capacity = config->replay_capacity;
    ctx->stats.fill_percentage = 0.0;
    ctx->stats.quality_estimate = 0.0;
    ctx->stats.active_threads = ctx->min_threads; // Set to min_threads as we'll start these
    ctx->stats.operations_count = 0;
    
    // Initialize threading
    ctx->min_threads = config->min_threads > 0 ? config->min_threads : 1;
    ctx->max_threads = config->max_threads > 0 ? config->max_threads : 4;
    ctx->active_threads = ctx->min_threads; // Initialize to match min_threads
    ctx->shutdown_requested = false;
    
    // Allocate worker threads
    ctx->workers = (cee_buffer_worker_t*)malloc(ctx->max_threads * sizeof(cee_buffer_worker_t));
    if (!ctx->workers) {
        // Clean up segments and replay log
        for (int i = 0; i < 3; i++) {
            free(ctx->segments[i].data);
        }
        if (ctx->replay_log) {
            free(ctx->replay_log);
        }
        pthread_mutex_destroy(&ctx->mutex);
        pthread_cond_destroy(&ctx->cond);
        free(ctx);
        return NULL;
    }
    
    // Initialize worker threads
    memset(ctx->workers, 0, ctx->max_threads * sizeof(cee_buffer_worker_t));
    
    // Start minimum number of worker threads
    for (int i = 0; i < ctx->min_threads; i++) {
        ctx->workers[i].active = true;
        ctx->workers[i].buffer_context = ctx;
        
        // Create thread attributes for detached state
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
        
        if (pthread_create(&ctx->workers[i].thread, &attr, cee_buffer_worker_thread, &ctx->workers[i]) != 0) {
            // Clean up previously created threads
            for (int j = 0; j < i; j++) {
                ctx->workers[j].active = false;
                pthread_join(ctx->workers[j].thread, NULL);
            }
            
            // Clean up segments, replay log, and workers
            for (int j = 0; j < 3; j++) {
                free(ctx->segments[j].data);
            }
            if (ctx->replay_log) {
                free(ctx->replay_log);
            }
            free(ctx->workers);
            pthread_mutex_destroy(&ctx->mutex);
            pthread_cond_destroy(&ctx->cond);
            free(ctx);
            pthread_attr_destroy(&attr);
            return NULL;
        }
        
        pthread_attr_destroy(&attr);
        ctx->active_threads++;
        
        // Explicitly log thread creation
        printf("Worker thread %d created successfully\n", i);
    }
    
    // Ensure stats reflect the actual thread count
    ctx->stats.active_threads = ctx->active_threads;
    
    // Initialize backpressure
    ctx->backpressure_enabled = config->enable_backpressure;
    ctx->backpressure_threshold = 0.8; // 80% full triggers backpressure
    
    // Initialize entropy bus integration
    ctx->bus_source_id = -1;
    ctx->bus_registered = false;
    
    // Register with entropy bus
    if (entropy_bus_init() == 0) {
        ctx->bus_source_id = entropy_bus_register_source("CEE_BUFFER", 0.7);
        if (ctx->bus_source_id >= 0) {
            ctx->bus_registered = true;
        }
    }
    
    return (cee_buffer_handle_t)ctx;
}

/**
 * @brief Store entropy in the buffer
 * 
 * @param handle Buffer handle
 * @param data Entropy data
 * @param size Data size in bytes
 * @param quality Entropy quality level
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_store(cee_buffer_handle_t handle, 
                                    const uint8_t* data, 
                                    size_t size,
                                    cee_buffer_quality_t quality) {
    if (!handle || !data || size == 0 || quality > CEE_BUFFER_QUALITY_HIGH) {
        return CEE_BUFFER_STATUS_ERROR;
    }
    
    struct cee_buffer_context* ctx = (struct cee_buffer_context*)handle;
    
    pthread_mutex_lock(&ctx->mutex);
    
    // Check if there's enough space
    if (ctx->segments[quality].size + size > ctx->segments[quality].capacity) {
        // Apply backpressure if enabled
        if (ctx->backpressure_enabled) {
            // Try to store in a lower quality segment
            if (quality > CEE_BUFFER_QUALITY_LOW) {
                cee_buffer_quality_t lower_quality = (cee_buffer_quality_t)(quality - 1);
                if (ctx->segments[lower_quality].size + size <= ctx->segments[lower_quality].capacity) {
                    quality = lower_quality;
                } else {
                    pthread_mutex_unlock(&ctx->mutex);
                    return CEE_BUFFER_STATUS_OVERFLOW;
                }
            } else {
                pthread_mutex_unlock(&ctx->mutex);
                return CEE_BUFFER_STATUS_OVERFLOW;
            }
        } else {
            pthread_mutex_unlock(&ctx->mutex);
            return CEE_BUFFER_STATUS_OVERFLOW;
        }
    }
    
    // Store the data
    memcpy(ctx->segments[quality].data + ctx->segments[quality].size, data, size);
    ctx->segments[quality].size += size;
    
    // Update statistics
    ctx->stats.bytes_stored += size;
    ctx->stats.current_size += size;
    ctx->stats.fill_percentage = (double)ctx->stats.current_size / (double)ctx->stats.capacity;
    ctx->stats.operations_count++;
    
    // Update quality estimate (weighted average based on segment sizes)
    double total_size = 0.0;
    double quality_sum = 0.0;
    
    for (int i = 0; i < 3; i++) {
        total_size += ctx->segments[i].size;
        
        double quality_value = 0.0;
        switch (i) {
            case CEE_BUFFER_QUALITY_LOW:
                quality_value = 0.3;
                break;
            case CEE_BUFFER_QUALITY_MEDIUM:
                quality_value = 0.7;
                break;
            case CEE_BUFFER_QUALITY_HIGH:
                quality_value = 0.9;
                break;
        }
        
        quality_sum += ctx->segments[i].size * quality_value;
    }
    
    if (total_size > 0.0) {
        ctx->stats.quality_estimate = quality_sum / total_size;
    } else {
        ctx->stats.quality_estimate = 0.0;
    }
    
    // Add to replay log if enabled
    if (ctx->replay_enabled && ctx->replay_log && size <= ctx->replay_capacity) {
        // If not enough space, shift the log
        if (ctx->replay_size + size > ctx->replay_capacity) {
            size_t shift_amount = size - (ctx->replay_capacity - ctx->replay_size);
            memmove(ctx->replay_log, ctx->replay_log + shift_amount, ctx->replay_size - shift_amount);
            ctx->replay_size -= shift_amount;
        }
        
        // Add to log
        memcpy(ctx->replay_log + ctx->replay_size, data, size);
        ctx->replay_size += size;
        ctx->stats.replay_size = ctx->replay_size;
    }
    
    // Signal waiting threads
    pthread_cond_broadcast(&ctx->cond);
    
    pthread_mutex_unlock(&ctx->mutex);
    
    return CEE_BUFFER_STATUS_OK;
}

/**
 * @brief Retrieve entropy from the buffer
 * 
 * @param handle Buffer handle
 * @param data Output buffer
 * @param size Number of bytes to retrieve
 * @param min_quality Minimum acceptable quality level
 * @return cee_b
(Content truncated due to size limit. Use line ranges to read in chunks)