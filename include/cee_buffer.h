/**
 * @file cee_buffer.h
 * @brief Buffer management for the Charm Entropic Engine
 * 
 * This header defines the buffer management system for the CEE (Charm Entropic
 * Engine) subsystem, providing thread-safe storage and retrieval of entropy
 * with backpressure routing and replay logging capabilities.
 */

#ifndef CEE_BUFFER_H
#define CEE_BUFFER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Buffer quality levels
 */
typedef enum {
    CEE_BUFFER_QUALITY_LOW = 0,     /**< Low quality entropy */
    CEE_BUFFER_QUALITY_MEDIUM = 1,  /**< Medium quality entropy */
    CEE_BUFFER_QUALITY_HIGH = 2     /**< High quality entropy */
} cee_buffer_quality_t;

/**
 * @brief Buffer status codes
 */
typedef enum {
    CEE_BUFFER_STATUS_OK = 0,           /**< Operation successful */
    CEE_BUFFER_STATUS_EMPTY = 1,        /**< Buffer is empty */
    CEE_BUFFER_STATUS_FULL = 2,         /**< Buffer is full */
    CEE_BUFFER_STATUS_UNDERFLOW = -1,   /**< Not enough data available */
    CEE_BUFFER_STATUS_OVERFLOW = -2,    /**< Too much data to store */
    CEE_BUFFER_STATUS_ERROR = -3        /**< General error */
} cee_buffer_status_t;

/**
 * @brief Buffer configuration structure
 */
typedef struct {
    size_t capacity;                /**< Buffer capacity in bytes */
    bool enable_replay;             /**< Enable replay logging */
    size_t replay_capacity;         /**< Replay log capacity in bytes */
    bool enable_backpressure;       /**< Enable backpressure routing */
    int min_threads;                /**< Minimum number of worker threads */
    int max_threads;                /**< Maximum number of worker threads */
} cee_buffer_config_t;

/**
 * @brief Buffer statistics structure
 */
typedef struct {
    size_t bytes_stored;            /**< Total bytes stored */
    size_t bytes_retrieved;         /**< Total bytes retrieved */
    size_t current_size;            /**< Current buffer size */
    size_t capacity;                /**< Buffer capacity */
    size_t replay_size;             /**< Current replay log size */
    size_t replay_capacity;         /**< Replay log capacity */
    double fill_percentage;         /**< Buffer fill percentage */
    double quality_estimate;        /**< Estimated entropy quality (0.0-1.0) */
    int active_threads;             /**< Number of active worker threads */
    uint64_t operations_count;      /**< Total number of operations */
} cee_buffer_stats_t;

/**
 * @brief Buffer context handle
 */
typedef struct cee_buffer_context* cee_buffer_handle_t;

/**
 * @brief Initialize the entropy buffer
 * 
 * @param config Buffer configuration
 * @return cee_buffer_handle_t Buffer handle or NULL on failure
 */
cee_buffer_handle_t cee_buffer_init(const cee_buffer_config_t* config);

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
                                    cee_buffer_quality_t quality);

/**
 * @brief Retrieve entropy from the buffer
 * 
 * @param handle Buffer handle
 * @param data Output buffer
 * @param size Number of bytes to retrieve
 * @param min_quality Minimum acceptable quality level
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_retrieve(cee_buffer_handle_t handle,
                                       uint8_t* data,
                                       size_t size,
                                       cee_buffer_quality_t min_quality);

/**
 * @brief Get buffer statistics
 * 
 * @param handle Buffer handle
 * @param stats Statistics structure to fill
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_get_stats(cee_buffer_handle_t handle,
                                        cee_buffer_stats_t* stats);

/**
 * @brief Check if buffer has enough entropy available
 * 
 * @param handle Buffer handle
 * @param size Number of bytes needed
 * @param min_quality Minimum acceptable quality level
 * @return bool true if enough entropy is available, false otherwise
 */
bool cee_buffer_has_entropy(cee_buffer_handle_t handle,
                           size_t size,
                           cee_buffer_quality_t min_quality);

/**
 * @brief Enable or disable replay logging
 * 
 * @param handle Buffer handle
 * @param enable Enable flag
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_set_replay(cee_buffer_handle_t handle,
                                         bool enable);

/**
 * @brief Replay entropy from the log
 * 
 * @param handle Buffer handle
 * @param data Output buffer
 * @param size Number of bytes to replay
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_replay(cee_buffer_handle_t handle,
                                     uint8_t* data,
                                     size_t size);

/**
 * @brief Clear the buffer
 * 
 * @param handle Buffer handle
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_clear(cee_buffer_handle_t handle);

/**
 * @brief Shutdown the buffer and free resources
 * 
 * @param handle Buffer handle
 */
void cee_buffer_shutdown(cee_buffer_handle_t handle);

/**
 * @brief Push entropy to the entropy bus
 * 
 * @param handle Buffer handle
 * @param size Number of bytes to push
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_push_to_bus(cee_buffer_handle_t handle,
                                          size_t size);

/**
 * @brief Set the number of worker threads
 * 
 * @param handle Buffer handle
 * @param num_threads Number of threads
 * @return cee_buffer_status_t Status code
 */
cee_buffer_status_t cee_buffer_set_threads(cee_buffer_handle_t handle,
                                          int num_threads);

/**
 * @brief Get the estimated quality of available entropy
 * 
 * @param handle Buffer handle
 * @return double Quality estimate between 0.0 (poor) and 1.0 (excellent)
 */
double cee_buffer_get_quality(cee_buffer_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* CEE_BUFFER_H */
