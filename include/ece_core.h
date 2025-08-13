/**
 * @file ece_core.h
 * @brief Core functionality for the Entropic Collapse Engine
 * 
 * This header defines the core functionality for the ECE (Entropic Collapse
 * Engine) subsystem, providing the Entropic Collapse Function (ECF) using
 * multi-phase field compression, ternary logic gates, trampoline mappings,
 * and avalanche-fused output.
 */

#ifndef ECE_CORE_H
#define ECE_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECE status codes
 */
typedef enum {
    ECE_STATUS_OK = 0,           /**< Operation successful */
    ECE_STATUS_ERROR = -1,       /**< General error */
    ECE_STATUS_INVALID_ARG = -2, /**< Invalid argument */
    ECE_STATUS_NO_ENTROPY = -3,  /**< Insufficient entropy */
    ECE_STATUS_OVERFLOW = -4     /**< Buffer overflow */
} ece_status_t;

/**
 * @brief ECE context handle
 */
typedef struct ece_context* ece_handle_t;

/**
 * @brief ECE configuration structure
 */
typedef struct {
    uint32_t collapse_rounds;    /**< Number of collapse rounds (min 8, max 64) */
    bool use_ternary_logic;      /**< Use ternary logic gates */
    bool use_trampoline;         /**< Use trampoline mappings */
    bool use_avalanche;          /**< Use avalanche-fused output */
    double entropy_quality;      /**< Required entropy quality (0.0-1.0) */
} ece_config_t;

/**
 * @brief ECE statistics structure
 */
typedef struct {
    uint64_t bytes_processed;    /**< Total bytes processed */
    uint64_t collapses_performed;/**< Total collapses performed */
    double avg_entropy_quality;  /**< Average entropy quality */
    uint32_t avg_rounds;         /**< Average rounds per collapse */
    uint64_t operations_count;   /**< Total number of operations */
} ece_stats_t;

/**
 * @brief Initialize the ECE context
 * 
 * @param config Configuration structure
 * @return ece_handle_t Context handle or NULL on failure
 */
ece_handle_t ece_init(const ece_config_t* config);

/**
 * @brief Shutdown the ECE context and free resources
 * 
 * @param handle Context handle
 */
void ece_shutdown(ece_handle_t handle);

/**
 * @brief Reset the ECE context to initial state
 * 
 * @param handle Context handle
 * @return ece_status_t Status code
 */
ece_status_t ece_reset(ece_handle_t handle);

/**
 * @brief Process a block of data through the Entropic Collapse Function
 * 
 * @param handle Context handle
 * @param data Input data
 * @param size Size of data in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_process_block(ece_handle_t handle, const uint8_t* data, size_t size);

/**
 * @brief Finalize the collapse and get the digest
 * 
 * @param handle Context handle
 * @param digest Output buffer for digest
 * @param size Size of digest buffer in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_finalize(ece_handle_t handle, uint8_t* digest, size_t size);

/**
 * @brief Get statistics for the ECE context
 * 
 * @param handle Context handle
 * @param stats Statistics structure to fill
 * @return ece_status_t Status code
 */
ece_status_t ece_get_stats(ece_handle_t handle, ece_stats_t* stats);

/**
 * @brief Set the number of collapse rounds
 * 
 * @param handle Context handle
 * @param rounds Number of rounds (min 8, max 64)
 * @return ece_status_t Status code
 */
ece_status_t ece_set_rounds(ece_handle_t handle, uint32_t rounds);

/**
 * @brief Set the required entropy quality
 * 
 * @param handle Context handle
 * @param quality Entropy quality (0.0-1.0)
 * @return ece_status_t Status code
 */
ece_status_t ece_set_entropy_quality(ece_handle_t handle, double quality);

/**
 * @brief Enable or disable ternary logic gates
 * 
 * @param handle Context handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_set_ternary_logic(ece_handle_t handle, bool enable);

/**
 * @brief Enable or disable trampoline mappings
 * 
 * @param handle Context handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_set_trampoline(ece_handle_t handle, bool enable);

/**
 * @brief Enable or disable avalanche-fused output
 * 
 * @param handle Context handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_set_avalanche(ece_handle_t handle, bool enable);

/**
 * @brief Perform a one-shot collapse and get the digest
 * 
 * @param data Input data
 * @param data_size Size of input data in bytes
 * @param digest Output buffer for digest
 * @param digest_size Size of digest buffer in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_collapse(const uint8_t* data, size_t data_size, 
                         uint8_t* digest, size_t digest_size);

#ifdef __cplusplus
}
#endif

#endif /* ECE_CORE_H */
