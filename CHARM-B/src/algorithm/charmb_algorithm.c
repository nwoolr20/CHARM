/**
 * @file charmb_algorithm.c
 * @brief CHARM-B (CHARMbit) Main Algorithm Implementation
 * 
 * Ultra-optimized hash algorithm for small inputs with size-specific dispatch
 */

#include "../include/charmb.h"
#include "../include/charmb_entropy.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <immintrin.h>

/**
 * @brief Default configuration for optimal performance
 */
static const charmb_entropy_config_t default_config = {
    .use_simd = true,
    .constant_time = true,
    .rounds = 1,  // Minimal rounds for maximum speed
    .reserved = {0}
};

/**
 * @brief Initialize CHARM-B if not already initialized
 */
static int ensure_initialized(void) {
    charmb_entropy_state_t* state = charmb_entropy_get_state();
    if (!state->initialized) {
        return charmb_entropy_init(&default_config);
    }
    return 0;
}

/**
 * @brief CHARM-B large input hash using chunked processing
 */
static charmb_status_t charmb_hash_large(const uint8_t* data, size_t size, 
                                         uint8_t* digest, charmb_digest_size_t digest_size);

/**
 * @brief Main CHARM-B hash function with automatic size optimization
 */
charmb_status_t charmb_hash(const uint8_t* data, size_t size, 
                            uint8_t* digest, charmb_digest_size_t digest_size) {
    // Input validation
    if (!data || !digest) return CHARMB_ERROR_NULL_POINTER;
    if (size == 0 || size > CHARMB_MAX_INPUT_SIZE) return CHARMB_ERROR_INVALID_SIZE;
    if (digest_size != CHARMB_DIGEST_128 && digest_size != CHARMB_DIGEST_256) {
        return CHARMB_ERROR_INVALID_DIGEST_SIZE;
    }
    
    // Ensure entropy engine is initialized
    if (ensure_initialized() != 0) return CHARMB_ERROR_NULL_POINTER;
    
    size_t output_bytes = (digest_size == CHARMB_DIGEST_128) ? 16 : 32;
    
    // Size-specific dispatch for optimal performance
    switch (size) {
        case 8:
            return charmb_hash_8b(data, digest, digest_size);
        case 16:
            return charmb_hash_16b(data, digest, digest_size);
        case 32:
            return charmb_hash_32b(data, digest, digest_size);
        case 64:
            return charmb_hash_64b(data, digest, digest_size);
        default:
            // Handle arbitrary sizes efficiently
            if (size <= 8) {
                uint8_t padded[8] = {0};
                memcpy(padded, data, size);
                return charmb_hash_8b(padded, digest, digest_size);
            } else if (size <= 16) {
                uint8_t padded[16] = {0};
                memcpy(padded, data, size);
                return charmb_hash_16b(padded, digest, digest_size);
            } else if (size <= 32) {
                uint8_t padded[32] = {0};
                memcpy(padded, data, size);
                return charmb_hash_32b(padded, digest, digest_size);
            } else if (size <= 64) {
                uint8_t padded[64] = {0};
                memcpy(padded, data, size);
                return charmb_hash_64b(padded, digest, digest_size);
            } else {
                // For inputs > 64 bytes, use chunked processing with 64-byte blocks
                return charmb_hash_large(data, size, digest, digest_size);
            }
    }
}

/**
 * @brief Ultra-optimized 8-byte hash
 */
charmb_status_t charmb_hash_8b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size) {
    if (!data || !digest) return CHARMB_ERROR_NULL_POINTER;
    if (digest_size != CHARMB_DIGEST_128 && digest_size != CHARMB_DIGEST_256) {
        return CHARMB_ERROR_INVALID_DIGEST_SIZE;
    }
    
    if (ensure_initialized() != 0) return CHARMB_ERROR_NULL_POINTER;
    
    size_t output_bytes = (digest_size == CHARMB_DIGEST_128) ? 16 : 32;
    
    // Direct 64-bit processing with single-round mixing
    int result = charmb_entropy_process_8b(data, digest, output_bytes);
    return (result == 0) ? CHARMB_SUCCESS : CHARMB_ERROR_NULL_POINTER;
}

/**
 * @brief Ultra-optimized 16-byte hash
 */
charmb_status_t charmb_hash_16b(const uint8_t* data, uint8_t* digest, 
                                charmb_digest_size_t digest_size) {
    if (!data || !digest) return CHARMB_ERROR_NULL_POINTER;
    if (digest_size != CHARMB_DIGEST_128 && digest_size != CHARMB_DIGEST_256) {
        return CHARMB_ERROR_INVALID_DIGEST_SIZE;
    }
    
    if (ensure_initialized() != 0) return CHARMB_ERROR_NULL_POINTER;
    
    size_t output_bytes = (digest_size == CHARMB_DIGEST_128) ? 16 : 32;
    
    // SIMD 128-bit operations with optimized shuffling
    int result = charmb_entropy_process_16b(data, digest, output_bytes);
    return (result == 0) ? CHARMB_SUCCESS : CHARMB_ERROR_NULL_POINTER;
}

/**
 * @brief Ultra-optimized 32-byte hash
 */
charmb_status_t charmb_hash_32b(const uint8_t* data, uint8_t* digest, 
                                charmb_digest_size_t digest_size) {
    if (!data || !digest) return CHARMB_ERROR_NULL_POINTER;
    if (digest_size != CHARMB_DIGEST_128 && digest_size != CHARMB_DIGEST_256) {
        return CHARMB_ERROR_INVALID_DIGEST_SIZE;
    }
    
    if (ensure_initialized() != 0) return CHARMB_ERROR_NULL_POINTER;
    
    size_t output_bytes = (digest_size == CHARMB_DIGEST_128) ? 16 : 32;
    
    // Cross-block SIMD processing with avalanche diffusion
    int result = charmb_entropy_process_32b(data, digest, output_bytes);
    return (result == 0) ? CHARMB_SUCCESS : CHARMB_ERROR_NULL_POINTER;
}

/**
 * @brief Ultra-optimized 64-byte hash
 */
charmb_status_t charmb_hash_64b(const uint8_t* data, uint8_t* digest, 
                                charmb_digest_size_t digest_size) {
    if (!data || !digest) return CHARMB_ERROR_NULL_POINTER;
    if (digest_size != CHARMB_DIGEST_128 && digest_size != CHARMB_DIGEST_256) {
        return CHARMB_ERROR_INVALID_DIGEST_SIZE;
    }
    
    if (ensure_initialized() != 0) return CHARMB_ERROR_NULL_POINTER;
    
    size_t output_bytes = (digest_size == CHARMB_DIGEST_128) ? 16 : 32;
    
    // Parallel SIMD with four 128-bit blocks
    int result = charmb_entropy_process_64b(data, digest, output_bytes);
    return (result == 0) ? CHARMB_SUCCESS : CHARMB_ERROR_NULL_POINTER;
}

/**
 * @brief Check if SIMD acceleration is available
 */
bool charmb_simd_available(void) {
    return charmb_entropy_avx2_available();
}

/**
 * @brief Get CHARM-B algorithm information
 */
charmb_status_t charmb_get_info(char* version_string, char* features) {
    if (!version_string || !features) return CHARMB_ERROR_NULL_POINTER;
    
    snprintf(version_string, 32, "%s", CHARMB_VERSION_STRING);
    
    snprintf(features, 128, "Size-specific dispatch, %s, Zero-overhead design, Dual digest modes",
             charmb_simd_available() ? "AVX2 SIMD" : "Scalar optimized");
    
    return CHARMB_SUCCESS;
}

/**
 * @brief High-precision timing for benchmarks
 */
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/**
 * @brief CHARM-B performance benchmark function
 */
charmb_status_t charmb_benchmark(size_t size, int iterations, double* throughput_mbps) {
    if (!throughput_mbps || size == 0 || size > CHARMB_MAX_INPUT_SIZE || iterations <= 0) {
        return CHARMB_ERROR_INVALID_SIZE;
    }
    
    // Prepare test data
    uint8_t* test_data = malloc(size);
    uint8_t digest[32];
    
    if (!test_data) return CHARMB_ERROR_NULL_POINTER;
    
    // Initialize with pseudo-random data
    for (size_t i = 0; i < size; i++) {
        test_data[i] = (uint8_t)(i * 0x9E3779B9U);
    }
    
    // Warmup
    for (int i = 0; i < 100; i++) {
        charmb_hash(test_data, size, digest, CHARMB_DIGEST_256);
    }
    
    // Benchmark
    double start_time = get_time_ms();
    
    for (int i = 0; i < iterations; i++) {
        charmb_status_t result = charmb_hash(test_data, size, digest, CHARMB_DIGEST_256);
        if (result != CHARMB_SUCCESS) {
            free(test_data);
            return result;
        }
    }
    
    double end_time = get_time_ms();
    double elapsed_ms = end_time - start_time;
    
    if (elapsed_ms <= 0.0) {
        *throughput_mbps = 0.0;
    } else {
        double total_bytes = (double)size * iterations;
        *throughput_mbps = (total_bytes / (1024.0 * 1024.0)) / (elapsed_ms / 1000.0);
    }
    
    free(test_data);
    return CHARMB_SUCCESS;
}

/**
 * @brief CHARM-B large input hash using chunked processing
 */
static charmb_status_t charmb_hash_large(const uint8_t* data, size_t size, 
                                         uint8_t* digest, charmb_digest_size_t digest_size) {
    if (!data || !digest) return CHARMB_ERROR_NULL_POINTER;
    if (size <= 64) return CHARMB_ERROR_INVALID_SIZE; // Should use specialized functions
    
    // Initialize intermediate hash state
    uint8_t state[32];
    memset(state, 0, 32);
    
    // Process input in 64-byte chunks
    size_t processed = 0;
    while (processed < size) {
        uint8_t chunk[64];
        size_t chunk_size = (size - processed < 64) ? (size - processed) : 64;
        
        // Copy chunk and pad if necessary
        memset(chunk, 0, 64);
        memcpy(chunk, data + processed, chunk_size);
        
        // XOR previous state into chunk for chaining
        for (int i = 0; i < 32 && i < chunk_size; i++) {
            chunk[i] ^= state[i];
        }
        
        // Hash the chunk
        charmb_status_t status = charmb_hash_64b(chunk, state, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return status;
        
        processed += chunk_size;
    }
    
    // Final output truncation if needed
    size_t output_bytes = (digest_size == CHARMB_DIGEST_128) ? 16 : 32;
    memcpy(digest, state, output_bytes);
    
    return CHARMB_SUCCESS;
}