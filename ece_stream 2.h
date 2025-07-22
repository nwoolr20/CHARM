/**
 * @file ece_stream.h
 * @brief Streaming interface for the Entropic Collapse Engine
 * 
 * This header defines the streaming interface for the ECE (Entropic Collapse
 * Engine) subsystem, providing mechanisms for processing large data streams
 * through the Entropic Collapse Function (ECF).
 */

#ifndef ECE_STREAM_H
#define ECE_STREAM_H

#include "ece_core.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECE stream handle
 */
typedef struct ece_stream* ece_stream_handle_t;

/**
 * @brief ECE stream configuration structure
 */
typedef struct {
    size_t buffer_size;          /**< Internal buffer size (0 for default) */
    uint32_t collapse_frequency; /**< Frequency of intermediate collapses (0 for auto) */
    bool auto_flush;             /**< Automatically flush when buffer is full */
    ece_config_t core_config;    /**< Configuration for the core ECE */
} ece_stream_config_t;

/**
 * @brief ECE stream statistics structure
 */
typedef struct {
    uint64_t bytes_streamed;     /**< Total bytes streamed */
    uint64_t chunks_processed;   /**< Number of chunks processed */
    uint64_t intermediate_collapses; /**< Number of intermediate collapses */
    double throughput;           /**< Processing throughput (bytes/sec) */
    ece_stats_t core_stats;      /**< Statistics from the core ECE */
} ece_stream_stats_t;

/**
 * @brief Initialize a streaming context
 * 
 * @param config Configuration structure (NULL for defaults)
 * @return ece_stream_handle_t Stream handle or NULL on failure
 */
ece_stream_handle_t ece_stream_init(const ece_stream_config_t* config);

/**
 * @brief Shutdown the streaming context and free resources
 * 
 * @param handle Stream handle
 */
void ece_stream_shutdown(ece_stream_handle_t handle);

/**
 * @brief Reset the streaming context to initial state
 * 
 * @param handle Stream handle
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_reset(ece_stream_handle_t handle);

/**
 * @brief Process a chunk of data in the stream
 * 
 * @param handle Stream handle
 * @param data Input data
 * @param size Size of data in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_update(ece_stream_handle_t handle, const uint8_t* data, size_t size);

/**
 * @brief Flush any buffered data and perform an intermediate collapse
 * 
 * @param handle Stream handle
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_flush(ece_stream_handle_t handle);

/**
 * @brief Finalize the stream and get the digest
 * 
 * @param handle Stream handle
 * @param digest Output buffer for digest
 * @param size Size of digest buffer in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_finalize(ece_stream_handle_t handle, uint8_t* digest, size_t size);

/**
 * @brief Get statistics for the streaming context
 * 
 * @param handle Stream handle
 * @param stats Statistics structure to fill
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_get_stats(ece_stream_handle_t handle, ece_stream_stats_t* stats);

/**
 * @brief Set the collapse frequency
 * 
 * @param handle Stream handle
 * @param frequency Collapse frequency (0 for auto)
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_set_collapse_frequency(ece_stream_handle_t handle, uint32_t frequency);

/**
 * @brief Set auto-flush mode
 * 
 * @param handle Stream handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_stream_set_auto_flush(ece_stream_handle_t handle, bool enable);

/**
 * @brief Get the underlying ECE core handle
 * 
 * @param handle Stream handle
 * @return ece_handle_t ECE core handle or NULL on failure
 */
ece_handle_t ece_stream_get_core(ece_stream_handle_t handle);

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
                                    size_t size, const ece_stream_config_t* config);

#ifdef __cplusplus
}
#endif

#endif /* ECE_STREAM_H */
