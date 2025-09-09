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
 * @file charmb_entropy.h
 * @brief CHARM-B Entropy Engine - Ultra-Fast Bit-Level Processing
 * 
 * Specialized entropy engine for ultra-small inputs with:
 * - Bit-level optimization for maximum efficiency
 * - SIMD acceleration using AVX2 instructions  
 * - Zero overhead design
 * - Thread-local static state
 */

#ifndef CHARMB_ENTROPY_H
#define CHARMB_ENTROPY_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM-B Entropy Engine Configuration
 */
typedef struct {
    bool use_simd;              // Enable SIMD acceleration
    bool constant_time;         // Constant-time operations
    uint8_t rounds;             // Number of entropy rounds (1-4)
    uint8_t reserved[5];        // Reserved for future use
} charmb_entropy_config_t;

/**
 * @brief Thread-local entropy state (zero-overhead design)
 */
typedef struct {
    uint64_t state[8];          // 512-bit internal state
    uint32_t counter;           // Operation counter
    uint8_t initialized;        // Initialization flag
    uint8_t reserved[3];        // Padding
} charmb_entropy_state_t;

/**
 * @brief Initialize CHARM-B entropy engine
 * 
 * @param config Engine configuration
 * @return 0 on success, negative on error
 */
int charmb_entropy_init(const charmb_entropy_config_t* config);

/**
 * @brief Bit-level entropy processing for 8-byte inputs
 * 
 * @param input Input data (8 bytes)
 * @param output Output buffer (variable size)
 * @param output_size Desired output size
 * @return 0 on success, negative on error
 */
int charmb_entropy_process_8b(const uint8_t input[8], uint8_t* output, size_t output_size);

/**
 * @brief Bit-level entropy processing for 16-byte inputs
 * 
 * @param input Input data (16 bytes)
 * @param output Output buffer (variable size)
 * @param output_size Desired output size
 * @return 0 on success, negative on error
 */
int charmb_entropy_process_16b(const uint8_t input[16], uint8_t* output, size_t output_size);

/**
 * @brief Bit-level entropy processing for 32-byte inputs
 * 
 * @param input Input data (32 bytes)
 * @param output Output buffer (variable size)
 * @param output_size Desired output size
 * @return 0 on success, negative on error
 */
int charmb_entropy_process_32b(const uint8_t input[32], uint8_t* output, size_t output_size);

/**
 * @brief Bit-level entropy processing for 64-byte inputs
 * 
 * @param input Input data (64 bytes)
 * @param output Output buffer (variable size)
 * @param output_size Desired output size
 * @return 0 on success, negative on error
 */
int charmb_entropy_process_64b(const uint8_t input[64], uint8_t* output, size_t output_size);

/**
 * @brief SIMD-accelerated entropy mixing using AVX2
 * 
 * @param data Input/output data
 * @param size Data size (must be multiple of 32)
 * @return 0 on success, negative on error
 */
int charmb_entropy_simd_mix(uint8_t* data, size_t size);

/**
 * @brief Ultra-fast bit diffusion for small inputs
 * 
 * @param input Input data
 * @param size Input size
 * @param output Output buffer
 * @param rounds Number of diffusion rounds
 * @return 0 on success, negative on error
 */
int charmb_entropy_bit_diffusion(const uint8_t* input, size_t size, 
                                 uint8_t* output, int rounds);

/**
 * @brief Get thread-local entropy state
 * 
 * @return Pointer to thread-local state
 */
charmb_entropy_state_t* charmb_entropy_get_state(void);

/**
 * @brief Reset entropy engine state
 */
void charmb_entropy_reset(void);

/**
 * @brief Check if AVX2 acceleration is available
 * 
 * @return true if AVX2 is supported
 */
bool charmb_entropy_avx2_available(void);

#ifdef __cplusplus
}
#endif

#endif /* CHARMB_ENTROPY_H */