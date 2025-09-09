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
 * @file ece_stream.c
 * @brief Implementation of streaming interface for the Entropic Collapse Engine
 * 
 * This file implements the streaming interface for the ECE (Entropic Collapse
 * Engine) subsystem, providing mechanisms for processing large data streams
 * through the Entropic Collapse Function (ECF).
 */

#include "ece_stream.h"
#include "ece_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/**
 * @brief Default buffer size (64 KB)
 */
#define ECE_STREAM_DEFAULT_BUFFER_SIZE (64 * 1024)

/**
 * @brief Default collapse frequency (every 1 MB)
 */
#define ECE_STREAM_DEFAULT_COLLAPSE_FREQUENCY (1024 * 1024)

/**
 * @brief Minimum buffer size (1 KB)
 */
#define ECE_STREAM_MIN_BUFFER_SIZE (1024)

/**
 * @brief Maximum buffer size (16 MB)
 */
#define ECE_STREAM_MAX_BUFFER_SIZE (16 * 1024 * 1024)

/**
 * @brief Internal stream context structure
 */
struct ece_stream {
    ece_handle_t core;               /**< ECE core handle */
    uint8_t* buffer;                 /**< Internal buffer */
    size_t buffer_size;              /**< Buffer size */
    size_t buffer_used;              /**< Used buffer space */
    uint32_t collapse_frequency;     /**< Collapse frequency */
    bool auto_flush;                 /**< Auto-flush flag */
    uint64_t bytes_streamed;         /**< Total bytes streamed */
    uint64_t chunks_processed;       /**< Chunks processed */
    uint64_t intermediate_collapses; /**< Intermediate collapses */
    struct timeval start_time;       /**< Processing start time */
    struct timeval end_time;         /**< Processing end time */
    bool finalized;                  /**< Finalization flag */
};

/**
 * @brief Default ECE stream configuration
 */
static const ece_stream_config_t ECE_STREAM_DEFAULT_CONFIG = {
    .buffer_size = 0,  /* Use default */
    .collapse_frequency = 0,  /* Use default */
    .auto_flush = true,
    .core_config = {
        .collapse_rounds = 16,
        .use_ternary_logic = true,
        .use_trampoline = true,
        .use_avalanche = true,
        .entropy_quality = 0.8
    }
};

/**
 * @brief Get current timestamp
 * 
 * @param tv Timevalue structure to fill
 */
static void ece_stream_get_timestamp(struct timeval* tv) {
    gettimeofday(tv, NULL);
}

/**
 * @brief Calculate time difference in seconds
 * 
 * @param start Start time
 * @param end End time
 * @return double Time difference in seconds
 */
static double ece_stream_time_diff(const struct timeval* start, const struct timeval* end) {
    return (end->tv_sec - start->tv_sec) + 
           (end->tv_usec - start->tv_usec) / 1000000.0;
}

/**
 * @brief Initialize a streaming context
 * 
 * @param config Configuration structure (NULL for defaults)
 * @return ece_stream_handle_t Stream handle or NULL on failure
 */
ece_stream_handle_t ece_stream_init(const ece_stream_config_t* config) {
    // Use default config if none provided
    ece_stream_config_t effective_config;
    if (config) {
        effective_config = *config;
    } else {
        effective_config = ECE_STREAM_DEFAULT_CONFIG;
    }
    
    // Validate and adjust buffer size
    if (effective_config.buffer_size == 0) {
        effective_config.buffer_size = ECE_STREAM_DEFAULT_BUFFER_SIZE;
    } else if (effective_config.buffer_size < ECE_STREAM_MIN_BUFFER_SIZE) {
        effective_config.buffer_size = ECE_STREAM_MIN_BUFFER_SIZE;
    } else if (effective_config.buffer_size > ECE_STREAM_MAX_BUFFER_SIZE) {
        effective_config.buffer_size = ECE_STREAM_MAX_BUFFER_SIZE;
    }
    
    // Validate and adjust collapse frequency
    if (effective_config.collapse_frequency == 0) {
        effective_config.collapse_frequency = ECE_STREAM_DEFAULT_COLLAPSE_FREQUENCY;
    }
    
    // Allocate stream context
    struct ece_stream* stream = (struct ece_stream*)calloc(1, sizeof(struct ece_stream));
    if (!stream) {
        return NULL;
    }
    
    // Initialize ECE core
    stream->core = ece_init(&effective_config.core_config);
    if (!stream->core) {
        free(stream);
        return NULL;
    }
    
    // Allocate buffer
    stream->buffer = (uint8_t*)malloc(effective_config.buffer_size);
    if (!stream->buffer) {
        ece_shutdown(stream->core);
        free(stream);
        return NULL;
    }
    
    // Initialize other fields
    stream->buffer_size = effective_config.buffer_size;
    stream->buffer_used = 0;
    stream->collapse_frequency = effective_config.collapse_frequency;
    stream->auto_flush = effective_config.auto_flush;
    stream->bytes_streamed = 0;
    stream->chunks_processed = 0;
    stream->intermediate_collapses = 0;
    stream->finalized = false;
    
    // Initialize timing
    ece_stream_get_timestamp(&stream->start_time);
    stream->end_time = stream->start_time;
    
    return stream;
}

/**
 * @brief Shutdown the streaming context and free resources
 * 
 * @param handle Stream handle
 */
void ece_stream_shutdown(ece_stream_handle_t handle) {
    if (!handle) {
        return;
    }
    
    // Free resources
    if (handle->core) {
        ece_shutdown(handle->core);
    }
    
    if (handle->buffer) {
        free(handle->buffer);
    }
    
    free(handle);
}

/**
 * @brief Reset the streaming context to initial state
 * 
 * @param handle Stream handle
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_reset(ece_stream_handle_t handle) {
    if (!handle) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Reset ECE core
    ece_status_t status = ece_reset(handle->core);
    if (status != ECE_STATUS_OK) {
        return status;
    }
    
    // Reset buffer
    memset(handle->buffer, 0, handle->buffer_size);
    handle->buffer_used = 0;
    
    // Reset statistics
    handle->bytes_streamed = 0;
    handle->chunks_processed = 0;
    handle->intermediate_collapses = 0;
    handle->finalized = false;
    
    // Reset timing
    ece_stream_get_timestamp(&handle->start_time);
    handle->end_time = handle->start_time;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Process a chunk of data in the stream
 * 
 * @param handle Stream handle
 * @param data Input data
 * @param size Size of data in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_update(ece_stream_handle_t handle, const uint8_t* data, size_t size) {
    if (!handle || !data || size == 0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Check if already finalized
    if (handle->finalized) {
        return ECE_STATUS_ERROR;
    }
    
    // Update statistics
    handle->bytes_streamed += size;
    handle->chunks_processed++;
    
    // Update timing
    ece_stream_get_timestamp(&handle->end_time);
    
    // Process data
    size_t remaining = size;
    size_t offset = 0;
    ece_status_t status = ECE_STATUS_OK;
    
    while (remaining > 0) {
        // Calculate how much data to copy to buffer
        size_t to_copy = remaining;
        if (handle->buffer_used + to_copy > handle->buffer_size) {
            to_copy = handle->buffer_size - handle->buffer_used;
        }
        
        // Copy data to buffer
        memcpy(handle->buffer + handle->buffer_used, data + offset, to_copy);
        handle->buffer_used += to_copy;
        offset += to_copy;
        remaining -= to_copy;
        
        // Check if buffer is full or if we've reached collapse frequency
        if (handle->buffer_used == handle->buffer_size || 
            (handle->auto_flush && handle->bytes_streamed % handle->collapse_frequency == 0)) {
            // Process buffer
            status = ece_process_block(handle->core, handle->buffer, handle->buffer_used);
            if (status != ECE_STATUS_OK) {
                return status;
            }
            
            // Reset buffer
            handle->buffer_used = 0;
            
            // Update statistics
            handle->intermediate_collapses++;
        }
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Flush any buffered data and perform an intermediate collapse
 * 
 * @param handle Stream handle
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_flush(ece_stream_handle_t handle) {
    if (!handle) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Check if already finalized
    if (handle->finalized) {
        return ECE_STATUS_ERROR;
    }
    
    // Check if buffer is empty
    if (handle->buffer_used == 0) {
        return ECE_STATUS_OK;
    }
    
    // Process buffer
    ece_status_t status = ece_process_block(handle->core, handle->buffer, handle->buffer_used);
    if (status != ECE_STATUS_OK) {
        return status;
    }
    
    // Reset buffer
    handle->buffer_used = 0;
    
    // Update statistics
    handle->intermediate_collapses++;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Finalize the stream and get the digest
 * 
 * @param handle Stream handle
 * @param digest Output buffer for digest
 * @param size Size of digest buffer in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_finalize(ece_stream_handle_t handle, uint8_t* digest, size_t size) {
    if (!handle || !digest || size == 0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Check if already finalized
    if (handle->finalized) {
        return ECE_STATUS_ERROR;
    }
    
    // Flush any remaining data
    ece_status_t status = ece_stream_flush(handle);
    if (status != ECE_STATUS_OK) {
        return status;
    }
    
    // Finalize and get digest
    status = ece_finalize(handle->core, digest, size);
    if (status != ECE_STATUS_OK) {
        return status;
    }
    
    // Update timing
    ece_stream_get_timestamp(&handle->end_time);
    
    // Mark as finalized
    handle->finalized = true;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Get statistics for the streaming context
 * 
 * @param handle Stream handle
 * @param stats Statistics structure to fill
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_get_stats(ece_stream_handle_t handle, ece_stream_stats_t* stats) {
    if (!handle || !stats) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Fill statistics
    stats->bytes_streamed = handle->bytes_streamed;
    stats->chunks_processed = handle->chunks_processed;
    stats->intermediate_collapses = handle->intermediate_collapses;
    
    // Calculate throughput
    double elapsed = ece_stream_time_diff(&handle->start_time, &handle->end_time);
    if (elapsed > 0) {
        stats->throughput = handle->bytes_streamed / elapsed;
    } else {
        stats->throughput = 0;
    }
    
    // Get core statistics
    ece_status_t status = ece_get_stats(handle->core, &stats->core_stats);
    if (status != ECE_STATUS_OK) {
        return status;
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Set the collapse frequency
 * 
 * @param handle Stream handle
 * @param frequency Collapse frequency (0 for auto)
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_set_collapse_frequency(ece_stream_handle_t handle, uint32_t frequency) {
    if (!handle) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Set collapse frequency
    if (frequency == 0) {
        handle->collapse_frequency = ECE_STREAM_DEFAULT_COLLAPSE_FREQUENCY;
    } else {
        handle->collapse_frequency = frequency;
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Set auto-flush mode
 * 
 * @param handle Stream handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_set_auto_flush(ece_stream_handle_t handle, bool enable) {
    if (!handle) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Set auto-flush mode
    handle->auto_flush = enable;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Get the underlying ECE core handle
 * 
 * @param handle Stream handle
 * @return ece_handle_t ECE core handle or NULL on failure
 */
ece_handle_t ece_stream_get_core(ece_stream_handle_t handle) {
    if (!handle) {
        return NULL;
    }
    
    return handle->core;
}

/**
 * @brief Process a file through the streaming interface
 * 
 * @param filename Input filename
 * @param digest Output buffer for digest
 * @param size Size of digest buffer in bytes
 * @param config Configuration structure (NULL for defaults)
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_process_file(const char* filename, uint8_t* digest, 
                                    size_t size, const ece_stream_config_t* config) {
    if (!filename || !digest || size == 0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Open file
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return ECE_STATUS_ERROR;
    }
    
    // Initialize stream
    ece_stream_handle_t stream = ece_stream_init(config);
    if (!stream) {
        fclose(file);
        return ECE_STATUS_ERROR;
    }
    
    // Process file in chunks
    uint8_t buffer[ECE_STREAM_DEFAULT_BUFFER_SIZE];
    size_t bytes_read;
    ece_status_t status = ECE_STATUS_OK;
    
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        status = ece_stream_update(stream, buffer, bytes_read);
        if (status != ECE_STATUS_OK) {
            ece_stream_shutdown(stream);
            fclose(file);
            return status;
        }
    }
    
    // Check for file errors
    if (ferror(file)) {
        ece_stream_shutdown(stream);
        fclose(file);
        return ECE_STATUS_ERROR;
    }
    
    // Finalize and get digest
    status = ece_stream_finalize(stream, digest, size);
    
    // Clean up
    ece_stream_shutdown(stream);
    fclose(file);
    
    return status;
}
