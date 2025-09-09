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
 * @file fractal_charm.h
 * @brief CHARM with Fractal Down Memory-Bounded Execution
 * 
 * Experimental implementation combining CHARM's security properties with
 * Fractal Down's √N memory scaling and DAG-based execution planning.
 */

#ifndef FRACTAL_CHARM_H
#define FRACTAL_CHARM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Fractal CHARM Constants
 */
#define FRACTAL_CHARM_KEY_SIZE      32    // 256-bit key
#define FRACTAL_CHARM_NONCE_SIZE    16    // 128-bit nonce
#define FRACTAL_CHARM_TAG_SIZE      32    // 256-bit authentication tag
#define FRACTAL_CHARM_MAX_MEMORY    4096  // Maximum √N memory bound (bytes)

/**
 * @brief Fractal CHARM Status Codes
 */
typedef enum {
    FRACTAL_CHARM_SUCCESS = 0,
    FRACTAL_CHARM_ERROR_NULL_POINTER = -1,
    FRACTAL_CHARM_ERROR_INVALID_SIZE = -2,
    FRACTAL_CHARM_ERROR_AUTH_FAILED = -3,
    FRACTAL_CHARM_ERROR_MEMORY_EXCEEDED = -4,
    FRACTAL_CHARM_ERROR_EARLY_TERMINATION = -5
} fractal_charm_status_t;

/**
 * @brief Operation Priority Levels
 */
typedef enum {
    FRACTAL_PRIORITY_CRITICAL = 0,    // Authentication, key operations
    FRACTAL_PRIORITY_HIGH = 1,        // Encryption/decryption
    FRACTAL_PRIORITY_MEDIUM = 2,      // Keystream generation
    FRACTAL_PRIORITY_LOW = 3          // Auxiliary operations
} fractal_priority_t;

/**
 * @brief Execution Strategy
 */
typedef enum {
    FRACTAL_STRATEGY_FULL = 0,        // Complete execution
    FRACTAL_STRATEGY_ANYTIME = 1,     // Allow early termination
    FRACTAL_STRATEGY_BOUNDED = 2      // Strict memory bounds
} fractal_strategy_t;

/**
 * @brief DAG Node Type
 */
typedef enum {
    FRACTAL_NODE_HASH = 0,
    FRACTAL_NODE_HMAC = 1,
    FRACTAL_NODE_KEYSTREAM = 2,
    FRACTAL_NODE_XOR = 3,
    FRACTAL_NODE_AUTH = 4
} fractal_node_type_t;

/**
 * @brief DAG Node for representing cryptographic operations
 */
typedef struct fractal_node {
    fractal_node_type_t type;
    fractal_priority_t priority;
    uint32_t id;
    
    // Input dependencies
    uint32_t* dependencies;
    size_t dep_count;
    
    // Operation parameters
    uint8_t* input_data;
    size_t input_len;
    uint8_t* output_data;
    size_t output_len;
    
    // Execution state
    bool completed;
    bool in_progress;
    double progress;  // 0.0 to 1.0 for anytime results
    
    struct fractal_node* next;
} fractal_node_t;

/**
 * @brief Memory Pool for √N scaling
 */
typedef struct {
    uint8_t* pool;
    size_t pool_size;
    size_t used;
    size_t max_size;
    
    // Free list for efficient allocation
    uint8_t** free_blocks;
    size_t* block_sizes;
    size_t free_count;
} fractal_memory_pool_t;

/**
 * @brief Fractal CHARM Execution Context
 */
typedef struct {
    uint8_t key[FRACTAL_CHARM_KEY_SIZE];
    uint8_t nonce[FRACTAL_CHARM_NONCE_SIZE];
    
    // DAG execution state
    fractal_node_t* dag_head;
    uint32_t node_count;
    uint32_t completed_nodes;
    
    // Memory management
    fractal_memory_pool_t memory_pool;
    
    // Execution parameters
    fractal_strategy_t strategy;
    size_t memory_bound;
    double time_bound;  // Maximum execution time (seconds)
    
    // Progress tracking
    double overall_progress;
    bool terminated_early;
    
    bool initialized;
} fractal_charm_context_t;

/**
 * @brief Initialize Fractal CHARM context
 * 
 * @param ctx Context to initialize
 * @param key Encryption key (32 bytes)
 * @param nonce Nonce (16 bytes)
 * @param strategy Execution strategy
 * @param memory_bound Maximum memory usage (√N scaling)
 * @return Status code
 */
fractal_charm_status_t fractal_charm_init(
    fractal_charm_context_t* ctx,
    const uint8_t key[FRACTAL_CHARM_KEY_SIZE],
    const uint8_t nonce[FRACTAL_CHARM_NONCE_SIZE],
    fractal_strategy_t strategy,
    size_t memory_bound
);

/**
 * @brief Plan DAG execution for AEAD encryption
 * 
 * Builds dependency graph for optimal execution order
 * 
 * @param ctx Fractal CHARM context
 * @param aad Additional authenticated data
 * @param aad_len Length of AAD
 * @param plaintext_len Length of plaintext
 * @return Status code
 */
fractal_charm_status_t fractal_charm_plan_encrypt(
    fractal_charm_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    size_t plaintext_len
);

/**
 * @brief Execute DAG for encryption with √N memory scaling
 * 
 * @param ctx Fractal CHARM context
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext
 * @param tag Output authentication tag
 * @return Status code
 */
fractal_charm_status_t fractal_charm_execute_encrypt(
    fractal_charm_context_t* ctx,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[FRACTAL_CHARM_TAG_SIZE]
);

/**
 * @brief Fractal CHARM encryption with anytime results
 * 
 * Combined planning and execution with early termination support
 * 
 * @param ctx Fractal CHARM context
 * @param aad Additional authenticated data
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext
 * @param tag Output authentication tag
 * @return Status code
 */
fractal_charm_status_t fractal_charm_encrypt(
    fractal_charm_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[FRACTAL_CHARM_TAG_SIZE]
);

/**
 * @brief Fractal CHARM decryption with bounded memory
 * 
 * @param ctx Fractal CHARM context
 * @param aad Additional authenticated data
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag
 * @param plaintext Output plaintext
 * @return Status code
 */
fractal_charm_status_t fractal_charm_decrypt(
    fractal_charm_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[FRACTAL_CHARM_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief Get execution progress for anytime results
 * 
 * @param ctx Fractal CHARM context
 * @return Progress value (0.0 to 1.0)
 */
double fractal_charm_get_progress(const fractal_charm_context_t* ctx);

/**
 * @brief Request early termination
 * 
 * @param ctx Fractal CHARM context
 * @return Status code
 */
fractal_charm_status_t fractal_charm_terminate_early(fractal_charm_context_t* ctx);

/**
 * @brief Get memory usage statistics
 * 
 * @param ctx Fractal CHARM context
 * @param current_usage Current memory usage (output)
 * @param peak_usage Peak memory usage (output)
 * @param bound_exceeded Whether memory bound was exceeded (output)
 * @return Status code
 */
fractal_charm_status_t fractal_charm_get_memory_stats(
    const fractal_charm_context_t* ctx,
    size_t* current_usage,
    size_t* peak_usage,
    bool* bound_exceeded
);

/**
 * @brief Benchmark Fractal CHARM vs. standard CHARM
 * 
 * @param strategy Execution strategy to test
 * @param payload_size Size of payload to test
 * @param memory_bound Memory bound for √N scaling
 * @param iterations Number of iterations
 * @param encrypt_mbps Output encryption throughput
 * @param decrypt_mbps Output decryption throughput
 * @param memory_efficiency Memory efficiency ratio (output)
 * @return Status code
 */
fractal_charm_status_t fractal_charm_benchmark(
    fractal_strategy_t strategy,
    size_t payload_size,
    size_t memory_bound,
    int iterations,
    double* encrypt_mbps,
    double* decrypt_mbps,
    double* memory_efficiency
);

/**
 * @brief Clean up Fractal CHARM context
 * 
 * @param ctx Context to clean
 */
void fractal_charm_cleanup(fractal_charm_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* FRACTAL_CHARM_H */