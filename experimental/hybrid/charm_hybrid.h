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
 * @file charm_hybrid.h
 * @brief CHARM Hybrid Optimization System
 * 
 * Advanced implementation combining CHARM, CHARM-B, AEAS, and Fractal Down
 * techniques with intelligent algorithm selection and SIMD optimizations.
 */

#ifndef CHARM_HYBRID_H
#define CHARM_HYBRID_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM Hybrid Constants
 */
#define CHARM_HYBRID_KEY_SIZE       32    // 256-bit key
#define CHARM_HYBRID_NONCE_SIZE     16    // 128-bit nonce
#define CHARM_HYBRID_TAG_SIZE       32    // 256-bit authentication tag
#define CHARM_HYBRID_MAX_BATCH      512   // Maximum batch size

/**
 * @brief CHARM Hybrid Status Codes
 */
typedef enum {
    CHARM_HYBRID_SUCCESS = 0,
    CHARM_HYBRID_ERROR_NULL_POINTER = -1,
    CHARM_HYBRID_ERROR_INVALID_SIZE = -2,
    CHARM_HYBRID_ERROR_AUTH_FAILED = -3,
    CHARM_HYBRID_ERROR_UNSUPPORTED_CPU = -4,
    CHARM_HYBRID_ERROR_INSUFFICIENT_MEMORY = -5
} charm_hybrid_status_t;

/**
 * @brief CPU Feature Detection
 */
typedef enum {
    CHARM_CPU_BASIC = 0x01,       // Basic CPU features
    CHARM_CPU_SSE2 = 0x02,        // SSE2 support
    CHARM_CPU_AVX2 = 0x04,        // AVX2 support
    CHARM_CPU_AVX512 = 0x08,      // AVX512 support
    CHARM_CPU_NEON = 0x10,        // ARM NEON support
    CHARM_CPU_AES_NI = 0x20       // AES-NI hardware acceleration
} charm_cpu_features_t;

/**
 * @brief Algorithm Selection Strategy
 */
typedef enum {
    CHARM_STRATEGY_AUTO = 0,      // Automatic selection based on profiling
    CHARM_STRATEGY_CHARM_B = 1,   // Force CHARM-B for small payloads
    CHARM_STRATEGY_CHARM = 2,     // Force regular CHARM
    CHARM_STRATEGY_AEAS = 3,      // Force AEAS optimizations
    CHARM_STRATEGY_FRACTAL = 4,   // Force Fractal Down execution
    CHARM_STRATEGY_CUSTOM = 5     // Custom algorithm selection
} charm_strategy_t;

/**
 * @brief Performance Profile
 */
typedef struct {
    size_t payload_size;
    double encryption_mbps;
    double decryption_mbps;
    size_t memory_usage;
    double cpu_usage;
    uint64_t cache_misses;
    double latency_us;
    charm_strategy_t strategy_used;
} charm_performance_profile_t;

/**
 * @brief System Resource Information
 */
typedef struct {
    size_t available_memory;
    size_t l1_cache_size;
    size_t l2_cache_size;
    size_t l3_cache_size;
    uint32_t cpu_frequency_mhz;
    charm_cpu_features_t cpu_features;
    bool thermal_throttling;
    double cpu_load;
} charm_system_info_t;

/**
 * @brief CHARM Hybrid Context
 */
typedef struct {
    uint8_t key[CHARM_HYBRID_KEY_SIZE];
    uint8_t nonce[CHARM_HYBRID_NONCE_SIZE];
    
    // Algorithm selection
    charm_strategy_t strategy;
    charm_strategy_t last_used_strategy;
    
    // System information
    charm_system_info_t system_info;
    
    // Performance history
    charm_performance_profile_t* profiles;
    size_t profile_count;
    size_t profile_capacity;
    
    // Optimization state
    uint8_t* batch_buffer;
    size_t batch_size;
    
    // SIMD function pointers
    void (*simd_xor)(const uint8_t*, const uint8_t*, uint8_t*, size_t);
    void (*simd_hash)(const uint8_t*, size_t, uint8_t*);
    
    bool initialized;
    bool profiling_enabled;
} charm_hybrid_context_t;

/**
 * @brief Initialize CHARM Hybrid system
 * 
 * @param ctx Context to initialize
 * @param key Encryption key (32 bytes)
 * @param nonce Nonce (16 bytes) 
 * @param strategy Algorithm selection strategy
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_init(
    charm_hybrid_context_t* ctx,
    const uint8_t key[CHARM_HYBRID_KEY_SIZE],
    const uint8_t nonce[CHARM_HYBRID_NONCE_SIZE],
    charm_strategy_t strategy
);

/**
 * @brief Detect CPU features and system capabilities
 * 
 * @param system_info Output system information
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_detect_system(charm_system_info_t* system_info);

/**
 * @brief Select optimal algorithm for given payload
 * 
 * @param ctx Hybrid context
 * @param payload_size Size of payload to process
 * @param aad_size Size of additional authenticated data
 * @param strategy Output selected strategy
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_select_algorithm(
    charm_hybrid_context_t* ctx,
    size_t payload_size,
    size_t aad_size,
    charm_strategy_t* strategy
);

/**
 * @brief CHARM Hybrid Encryption with automatic optimization
 * 
 * Automatically selects best algorithm and optimizations based on
 * payload characteristics and system resources.
 * 
 * @param ctx Hybrid context
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param plaintext Input plaintext
 * @param plaintext_len Length of plaintext
 * @param ciphertext Output ciphertext (same length as plaintext)
 * @param tag Output authentication tag (32 bytes)
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_encrypt(
    charm_hybrid_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* plaintext, size_t plaintext_len,
    uint8_t* ciphertext,
    uint8_t tag[CHARM_HYBRID_TAG_SIZE]
);

/**
 * @brief CHARM Hybrid Decryption with automatic optimization
 * 
 * @param ctx Hybrid context
 * @param aad Additional authenticated data (can be NULL)
 * @param aad_len Length of AAD
 * @param ciphertext Input ciphertext
 * @param ciphertext_len Length of ciphertext
 * @param tag Authentication tag (32 bytes)
 * @param plaintext Output plaintext (same length as ciphertext)
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_decrypt(
    charm_hybrid_context_t* ctx,
    const uint8_t* aad, size_t aad_len,
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t tag[CHARM_HYBRID_TAG_SIZE],
    uint8_t* plaintext
);

/**
 * @brief Enable/disable performance profiling
 * 
 * @param ctx Hybrid context
 * @param enable Whether to enable profiling
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_set_profiling(
    charm_hybrid_context_t* ctx,
    bool enable
);

/**
 * @brief Get performance statistics
 * 
 * @param ctx Hybrid context
 * @param profiles Output array of performance profiles
 * @param count Output count of profiles
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_get_profiles(
    const charm_hybrid_context_t* ctx,
    const charm_performance_profile_t** profiles,
    size_t* count
);

/**
 * @brief SIMD-optimized XOR operation (automatically selects best implementation)
 * 
 * @param a Input buffer A
 * @param b Input buffer B
 * @param out Output buffer (a XOR b)
 * @param len Length of buffers
 * @param features Available CPU features
 */
void charm_hybrid_simd_xor(
    const uint8_t* a,
    const uint8_t* b,
    uint8_t* out,
    size_t len,
    charm_cpu_features_t features
);

/**
 * @brief Benchmark all available algorithms and optimizations
 * 
 * @param payload_sizes Array of payload sizes to test
 * @param size_count Number of payload sizes
 * @param iterations Number of iterations per test
 * @param profiles Output performance profiles
 * @param profile_count Output number of profiles
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_benchmark_all(
    const size_t* payload_sizes,
    size_t size_count,
    int iterations,
    charm_performance_profile_t** profiles,
    size_t* profile_count
);

/**
 * @brief Get optimal algorithm recommendation for workload
 * 
 * @param payload_size Typical payload size
 * @param frequency Operations per second
 * @param memory_constraint Maximum memory usage
 * @param strategy Output recommended strategy
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_recommend_strategy(
    size_t payload_size,
    double frequency,
    size_t memory_constraint,
    charm_strategy_t* strategy
);

/**
 * @brief Warm up optimizations (cache function pointers, etc.)
 * 
 * @param ctx Hybrid context
 * @return Status code
 */
charm_hybrid_status_t charm_hybrid_warmup(charm_hybrid_context_t* ctx);

/**
 * @brief Clean up CHARM Hybrid context
 * 
 * @param ctx Context to clean
 */
void charm_hybrid_cleanup(charm_hybrid_context_t* ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_HYBRID_H */