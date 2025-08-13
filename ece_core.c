/**
 * @file ece_core.c
 * @brief Core functionality for the Entropic Collapse Engine
 * 
 * This file implements the core functionality for the ECE (Entropic Collapse
 * Engine) subsystem, providing the Entropic Collapse Function (ECF) using
 * multi-phase field compression, ternary logic gates, trampoline mappings,
 * and avalanche-fused output.
 */

#define _POSIX_C_SOURCE 200809L
#include "ece_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h> // For getpid()
#include <stdint.h> // For uintptr_t

// Constants for the Entropic Collapse Function
#define ECE_BLOCK_SIZE 64
#define ECE_STATE_SIZE 32
#define ECE_MIN_ROUNDS 8
#define ECE_MAX_ROUNDS 64
#define ECE_DEFAULT_ROUNDS 16

// Trampoline mapping constants
#define TRAMPOLINE_TABLE_SIZE 256
#define TRAMPOLINE_ITERATIONS 3

// Ternary logic constants
#define TRIT_0 0
#define TRIT_1 1
#define TRIT_2 2

// ECE context structure
struct ece_context {
    // Configuration
    uint32_t collapse_rounds;
    bool use_ternary_logic;
    bool use_trampoline;
    bool use_avalanche;
    double entropy_quality;
    
    // State
    uint8_t state[ECE_STATE_SIZE];
    uint8_t buffer[ECE_BLOCK_SIZE];
    size_t buffer_used;
    bool finalized;
    
    // Trampoline tables
    uint8_t trampoline_table[TRAMPOLINE_TABLE_SIZE];
    
    // Statistics
    ece_stats_t stats;
};

// Forward declarations for internal functions
static void ece_init_trampoline_table(ece_handle_t handle);
static void ece_collapse_block(ece_handle_t handle, const uint8_t* block);
static uint8_t ece_ternary_operation(uint8_t a, uint8_t b, uint8_t c);
static void ece_apply_trampoline(ece_handle_t handle, uint8_t* data, size_t size);
static void ece_apply_avalanche(uint8_t* data, size_t size);

/**
 * @brief Initialize the ECE context
 * 
 * @param config Configuration structure
 * @return ece_handle_t Context handle or NULL on failure
 */
ece_handle_t ece_init(const ece_config_t* config) {
    if (config == NULL) {
        return NULL;
    }
    
    // Allocate context
    ece_handle_t handle = (ece_handle_t)malloc(sizeof(struct ece_context));
    if (handle == NULL) {
        return NULL;
    }
    
    // Initialize context
    memset(handle, 0, sizeof(struct ece_context));
    
    // Set configuration
    handle->collapse_rounds = (config->collapse_rounds >= ECE_MIN_ROUNDS && 
                              config->collapse_rounds <= ECE_MAX_ROUNDS) ? 
                              config->collapse_rounds : ECE_DEFAULT_ROUNDS;
    handle->use_ternary_logic = config->use_ternary_logic;
    handle->use_trampoline = config->use_trampoline;
    handle->use_avalanche = config->use_avalanche;
    handle->entropy_quality = (config->entropy_quality >= 0.0 && 
                              config->entropy_quality <= 1.0) ? 
                              config->entropy_quality : 0.7;
    
    // Initialize state
    memset(handle->state, 0, ECE_STATE_SIZE);
    memset(handle->buffer, 0, ECE_BLOCK_SIZE);
    handle->buffer_used = 0;
    handle->finalized = false;
    
    // Initialize statistics
    memset(&handle->stats, 0, sizeof(ece_stats_t));
    handle->stats.avg_rounds = handle->collapse_rounds;
    handle->stats.avg_entropy_quality = handle->entropy_quality;
    
    // Initialize trampoline table
    ece_init_trampoline_table(handle);
    
    return handle;
}

/**
 * @brief Shutdown the ECE context and free resources
 * 
 * @param handle Context handle
 */
void ece_shutdown(ece_handle_t handle) {
    if (handle != NULL) {
        // Clear sensitive data
        memset(handle->state, 0, ECE_STATE_SIZE);
        memset(handle->buffer, 0, ECE_BLOCK_SIZE);
        memset(handle->trampoline_table, 0, TRAMPOLINE_TABLE_SIZE);
        
        // Free context
        free(handle);
    }
}

/**
 * @brief Reset the ECE context to initial state
 * 
 * @param handle Context handle
 * @return ece_status_t Status code
 */
ece_status_t ece_reset(ece_handle_t handle) {
    if (handle == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Reset state
    memset(handle->state, 0, ECE_STATE_SIZE);
    memset(handle->buffer, 0, ECE_BLOCK_SIZE);
    handle->buffer_used = 0;
    handle->finalized = false;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Process a block of data through the Entropic Collapse Function
 * 
 * @param handle Context handle
 * @param data Input data
 * @param size Size of data in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_process_block(ece_handle_t handle, const uint8_t* data, size_t size) {
    if (handle == NULL || data == NULL || size == 0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    if (handle->finalized) {
        return ECE_STATUS_ERROR;
    }
    
    // Process data
    size_t remaining = size;
    size_t offset = 0;
    
    while (remaining > 0) {
        // Fill buffer
        size_t to_copy = ECE_BLOCK_SIZE - handle->buffer_used;
        if (to_copy > remaining) {
            to_copy = remaining;
        }
        
        memcpy(handle->buffer + handle->buffer_used, data + offset, to_copy);
        handle->buffer_used += to_copy;
        offset += to_copy;
        remaining -= to_copy;
        
        // Process full blocks
        if (handle->buffer_used == ECE_BLOCK_SIZE) {
            ece_collapse_block(handle, handle->buffer);
            handle->buffer_used = 0;
        }
    }
    
    // Update statistics
    handle->stats.bytes_processed += size;
    handle->stats.operations_count++;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Finalize the collapse and get the digest
 * 
 * @param handle Context handle
 * @param digest Output buffer for digest
 * @param size Size of digest buffer in bytes
 * @return ece_status_t Status code
 */
ece_status_t ece_finalize(ece_handle_t handle, uint8_t* digest, size_t size) {
    if (handle == NULL || digest == NULL || size == 0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    if (handle->finalized) {
        // Already finalized, just copy the state
        size_t to_copy = (size < ECE_STATE_SIZE) ? size : ECE_STATE_SIZE;
        memcpy(digest, handle->state, to_copy);
        return ECE_STATUS_OK;
    }
    
    // Process any remaining data in the buffer
    if (handle->buffer_used > 0) {
        // Pad with zeros
        memset(handle->buffer + handle->buffer_used, 0, ECE_BLOCK_SIZE - handle->buffer_used);
        
        // Process final block
        ece_collapse_block(handle, handle->buffer);
    }
    
    // Apply final avalanche effect if enabled
    if (handle->use_avalanche) {
        ece_apply_avalanche(handle->state, ECE_STATE_SIZE);
    }
    
    // Copy state to digest
    size_t to_copy = (size < ECE_STATE_SIZE) ? size : ECE_STATE_SIZE;
    memcpy(digest, handle->state, to_copy);
    
    // Mark as finalized
    handle->finalized = true;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Get statistics for the ECE context
 * 
 * @param handle Context handle
 * @param stats Statistics structure to fill
 * @return ece_status_t Status code
 */
ece_status_t ece_get_stats(ece_handle_t handle, ece_stats_t* stats) {
    if (handle == NULL || stats == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Copy statistics
    memcpy(stats, &handle->stats, sizeof(ece_stats_t));
    
    return ECE_STATUS_OK;
}

/**
 * @brief Set the number of collapse rounds
 * 
 * @param handle Context handle
 * @param rounds Number of rounds (min 8, max 64)
 * @return ece_status_t Status code
 */
ece_status_t ece_set_rounds(ece_handle_t handle, uint32_t rounds) {
    if (handle == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    if (rounds < ECE_MIN_ROUNDS || rounds > ECE_MAX_ROUNDS) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    handle->collapse_rounds = rounds;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Set the required entropy quality
 * 
 * @param handle Context handle
 * @param quality Entropy quality (0.0-1.0)
 * @return ece_status_t Status code
 */
ece_status_t ece_set_entropy_quality(ece_handle_t handle, double quality) {
    if (handle == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    if (quality < 0.0 || quality > 1.0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    handle->entropy_quality = quality;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Enable or disable ternary logic gates
 * 
 * @param handle Context handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_set_ternary_logic(ece_handle_t handle, bool enable) {
    if (handle == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    handle->use_ternary_logic = enable;
    
    return ECE_STATUS_OK;
}

/**
 * @brief Enable or disable trampoline mappings
 * 
 * @param handle Context handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_set_trampoline(ece_handle_t handle, bool enable) {
    if (handle == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    handle->use_trampoline = enable;
    
    if (enable) {
        // Reinitialize trampoline table
        ece_init_trampoline_table(handle);
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Enable or disable avalanche-fused output
 * 
 * @param handle Context handle
 * @param enable Enable flag
 * @return ece_status_t Status code
 */
ece_status_t ece_set_avalanche(ece_handle_t handle, bool enable) {
    if (handle == NULL) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    handle->use_avalanche = enable;
    
    return ECE_STATUS_OK;
}

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
                         uint8_t* digest, size_t digest_size) {
    if (data == NULL || data_size == 0 || digest == NULL || digest_size == 0) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Create default configuration
    ece_config_t config;
    memset(&config, 0, sizeof(config));
    config.collapse_rounds = ECE_DEFAULT_ROUNDS;
    config.use_ternary_logic = true;
    config.use_trampoline = true;
    config.use_avalanche = true;
    config.entropy_quality = 0.7;
    
    // Initialize context
    ece_handle_t handle = ece_init(&config);
    if (handle == NULL) {
        return ECE_STATUS_ERROR;
    }
    
    // Process data
    ece_status_t status = ece_process_block(handle, data, data_size);
    if (status != ECE_STATUS_OK) {
        ece_shutdown(handle);
        return status;
    }
    
    // Finalize and get digest
    status = ece_finalize(handle, digest, digest_size);
    
    // Cleanup
    ece_shutdown(handle);
    
    return status;
}

/**
 * @brief Initialize the trampoline table
 * 
 * @param handle Context handle
 */
static void ece_init_trampoline_table(ece_handle_t handle) {
    if (handle == NULL) {
        return;
    }
    
    // Initialize table with identity mapping
    for (int i = 0; i < TRAMPOLINE_TABLE_SIZE; i++) {
        handle->trampoline_table[i] = (uint8_t)i;
    }
    
#include <unistd.h> // For getpid()

    // Shuffle table using Fisher-Yates algorithm with better entropy source
    // Use combination of time, pointer value, and process ID for better randomization
    unsigned int seed = (unsigned int)time(NULL);
    seed ^= (unsigned int)(uintptr_t)handle;
    seed ^= (unsigned int)getpid();
    
    // Add some runtime entropy by measuring CPU cycles
    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    for (volatile int i = 0; i < 10000; i++) { /* Busy loop to add timing jitter */ }
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    unsigned int jitter = (unsigned int)((ts_end.tv_nsec - ts_start.tv_nsec) & 0xFFFFFFFF);
    seed ^= jitter;
    
    srand(seed);
    
    // Apply multiple shuffle passes for better mixing
    for (int pass = 0; pass < 3; pass++) {
        for (int i = TRAMPOLINE_TABLE_SIZE - 1; i > 0; i--) {
            int j = rand() % (i + 1);
            uint8_t temp = handle->trampoline_table[i];
            handle->trampoline_table[i] = handle->trampoline_table[j];
            handle->trampoline_table[j] = temp;
        }
        
        // Mix in more entropy between passes
        seed = seed * 1103515245 + 12345;
        srand(seed ^ (unsigned int)clock());
    }
}

/**
 * @brief Collapse a block of data
 * 
 * @param handle Context handle
 * @param block Block of data (ECE_BLOCK_SIZE bytes)
 */
static void ece_collapse_block(ece_handle_t handle, const uint8_t* block) {
    if (handle == NULL || block == NULL) {
        return;
    }
    
    // Temporary state for block processing
    uint8_t temp_state[ECE_STATE_SIZE];
    memcpy(temp_state, handle->state, ECE_STATE_SIZE);
    
    // Mix block into state
    for (size_t i = 0; i < ECE_STATE_SIZE; i++) {
        temp_state[i] ^= block[i % ECE_BLOCK_SIZE];
    }
    
    // Apply trampoline mapping if enabled
    if (handle->use_trampoline) {
        ece_apply_trampoline(handle, temp_state, ECE_STATE_SIZE);
    }
    
    // Perform collapse rounds
    for (uint32_t round = 0; round < handle->collapse_rounds; round++) {
        // Apply ternary logic if enabled
        if (handle->use_ternary_logic) {
            for (size_t i = 0; i < ECE_STATE_SIZE - 2; i++) {
                temp_state[i] = ece_ternary_operation(temp_state[i], temp_state[i+1], temp_state[i+2]);
            }
            
            // Handle wrap-around for last two elements
            temp_state[ECE_STATE_SIZE-2] = ece_ternary_operation(temp_state[ECE_STATE_SIZE-2], 
                                                               temp_state[ECE_STATE_SIZE-1], 
                                                               temp_state[0]);
            temp_state[ECE_STATE_SIZE-1] = ece_ternary_operation(temp_state[ECE_STATE_SIZE-1], 
                                                               temp_state[0], 
                                                               temp_state[1]);
        } else {
            // Simple mixing if ternary logic is disabled
            for (size_t i = 0; i < ECE_STATE_SIZE - 1; i++) {
                temp_state[i] = (temp_state[i] + temp_state[i+1]) ^ (temp_state[i] * 0x1B);
            }
            temp_state[ECE_STATE_SIZE-1] = (temp_state[ECE_STATE_SIZE-1] + temp_state[0]) ^ 
                                         (temp_state[ECE_STATE_SIZE-1] * 0x1B);
        }
        
        // Apply trampoline mapping every other round if enabled
        if (handle->use_trampoline && (round % 2 == 1)) {
            ece_apply_trampoline(handle, temp_state, ECE_STATE_SIZE);
        }
        
        // Apply round constant
        for (size_t i = 0; i < ECE_STATE_SIZE; i++) {
            temp_state[i] ^= ((round * 7 + i * 13) & 0xFF);
        }
    }
    
    // Update state
    for (size_t i = 0; i < ECE_STATE_SIZE; i++) {
        handle->state[i] ^= temp_state[i];
    }
    
    // Update statistics
    handle->stats.collapses_performed++;
}

/**
 * @brief Perform ternary logic operation
 * 
 * @param a First input byte
 * @param b Second input byte
 * @param c Third input byte
 * @return uint8_t Result of ternary operation
 */
static uint8_t ece_ternary_operation(uint8_t a, uint8_t b, uint8_t c) {
    // Convert to ternary representation (3 trits per byte)
    uint8_t trits_a[3], trits_b[3], trits_c[3], result_trits[3];
    
    for (int i = 0; i < 3; i++) {
        trits_a[i] = (a >> (i*2)) & 0x3;
        trits_b[i] = (b >> (i*2)) & 0x3;
        trits_c[i] = (c >> (i*2)) & 0x3;
        
        // Ensure valid trit values (0, 1, 2)
        trits_a[i] %= 3;
        trits_b[i] %= 3;
        trits_c[i] %= 3;
        
        // Ternary majority function with chaos injection
        if ((trits_a[i] == trits_b[i]) || (trits_a[i] == trits_c[i])) {
            result_trits[i] = trits_a[i];
        } else if (trits_b[i] == trits_c[i]) {
            result_trits[i] = trits_b[i];
        } else {
            // Chaos injection for maximum entropy
            result_trits[i] = (trits_a[i] + trits_b[i] + trits_c[i]) % 3;
        }
    }
    
    // Convert back to byte representation
    uint8_t result = 0;
    for (int i = 0; i < 3; i++) {
        result |= (result_trits[i] & 0x3) << (i * 2);
    }
    
    return result;
}

/**
 * @brief Apply trampoline mapping for non-linear transformations
 * 
 * @param handle ECE context handle
 * @param data Data to transform
 * @param size Size of data
 */
static void ece_apply_trampoline(ece_handle_t handle, uint8_t* data, size_t size) {
    if (!handle || !data || size == 0) return;
    
    for (size_t i = 0; i < size; i++) {
        // Multi-stage trampoline mapping
        uint8_t val = data[i];
        for (int stage = 0; stage < TRAMPOLINE_ITERATIONS; stage++) {
            val = handle->trampoline_table[val];
            // Inject entropy from position and stage
            val ^= (uint8_t)(i * stage + handle->stats.operations_count);
        }
        data[i] = val;
    }
}

/**
 * @brief Apply avalanche effect for maximum diffusion
 * 
 * @param data Data to transform
 * @param size Size of data
 */
static void ece_apply_avalanche(uint8_t* data, size_t size) {
    if (!data || size == 0) return;
    
    // Forward avalanche pass
    for (size_t i = 1; i < size; i++) {
        data[i] ^= data[i-1];
        data[i] = (data[i] << 1) | (data[i] >> 7); // Rotate left by 1
    }
    
    // Backward avalanche pass
    for (size_t i = size - 1; i > 0; i--) {
        data[i-1] ^= data[i];
        data[i-1] = (data[i-1] << 3) | (data[i-1] >> 5); // Rotate left by 3
    }
    
    // Cross-diffusion pass
    if (size >= 4) {
        for (size_t i = 0; i < size - 3; i += 4) {
            // XOR with distant positions for maximum chaos
            data[i] ^= data[i+3];
            data[i+1] ^= data[i+2];
            data[i+2] ^= data[i+1] ^ data[i];
            data[i+3] ^= data[i+2] ^ data[i+1] ^ data[i];
        }
    }
}