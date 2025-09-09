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
 * @file neon_backend.h
 * @brief NEON SIMD backend for ARM platforms in the CHARM system
 * 
 * This header defines the NEON SIMD backend interfaces for ARM platforms
 * in the CHARM system, providing optimized vector operations.
 */

#ifndef NEON_BACKEND_H
#define NEON_BACKEND_H

#include "charm_status.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief NEON operation types
 */
typedef enum {
    NEON_OP_NONE = 0,           /**< No operation */
    NEON_OP_XOR = 1,            /**< Bitwise XOR */
    NEON_OP_AND = 2,            /**< Bitwise AND */
    NEON_OP_OR = 3,             /**< Bitwise OR */
    NEON_OP_NOT = 4,            /**< Bitwise NOT */
    NEON_OP_ADD = 5,            /**< Addition */
    NEON_OP_SUB = 6,            /**< Subtraction */
    NEON_OP_MUL = 7,            /**< Multiplication */
    NEON_OP_ROTATE_LEFT = 8,    /**< Rotate left */
    NEON_OP_ROTATE_RIGHT = 9,   /**< Rotate right */
    NEON_OP_SHIFT_LEFT = 10,    /**< Shift left */
    NEON_OP_SHIFT_RIGHT = 11,   /**< Shift right */
    NEON_OP_PERMUTE = 12,       /**< Permutation */
    NEON_OP_BLEND = 13,         /**< Blend */
    NEON_OP_TRANSPOSE = 14,     /**< Transpose */
    NEON_OP_INTERLEAVE = 15     /**< Interleave */
} neon_op_t;

/**
 * @brief NEON vector size
 */
typedef enum {
    NEON_VECTOR_64 = 0,         /**< 64-bit vector */
    NEON_VECTOR_128 = 1         /**< 128-bit vector */
} neon_vector_size_t;

/**
 * @brief NEON element size
 */
typedef enum {
    NEON_ELEMENT_8 = 0,         /**< 8-bit elements */
    NEON_ELEMENT_16 = 1,        /**< 16-bit elements */
    NEON_ELEMENT_32 = 2,        /**< 32-bit elements */
    NEON_ELEMENT_64 = 3         /**< 64-bit elements */
} neon_element_size_t;

/**
 * @brief NEON backend context handle
 */
typedef struct neon_context* neon_handle_t;

/**
 * @brief NEON backend configuration
 */
typedef struct {
    bool use_neon;              /**< Use NEON if available */
    bool use_crypto_ext;        /**< Use crypto extensions if available */
    bool use_dot_product;       /**< Use dot product instructions if available */
    bool use_fp16;              /**< Use FP16 instructions if available */
} neon_config_t;

/**
 * @brief Default NEON backend configuration
 */
extern const neon_config_t NEON_DEFAULT_CONFIG;

/**
 * @brief Initialize NEON backend
 * 
 * @param config Configuration (NULL for default)
 * @return neon_handle_t Handle or NULL on failure
 */
neon_handle_t neon_init(const neon_config_t* config);

/**
 * @brief Shutdown NEON backend
 * 
 * @param handle Backend handle
 */
void neon_shutdown(neon_handle_t handle);

/**
 * @brief Check if NEON is supported
 * 
 * @return bool True if supported
 */
bool neon_is_supported(void);

/**
 * @brief Check if NEON crypto extensions are supported
 * 
 * @return bool True if supported
 */
bool neon_crypto_is_supported(void);

/**
 * @brief Perform NEON operation on vectors
 * 
 * @param handle Backend handle
 * @param op Operation type
 * @param vector_size Vector size
 * @param element_size Element size
 * @param src1 First source buffer
 * @param src2 Second source buffer (can be NULL for unary operations)
 * @param dst Destination buffer
 * @param count Number of vectors to process
 * @return charm_status_t Status code
 */
charm_status_t neon_operate(neon_handle_t handle, neon_op_t op, 
                           neon_vector_size_t vector_size, 
                           neon_element_size_t element_size,
                           const void* src1, const void* src2, 
                           void* dst, size_t count);

/**
 * @brief Perform NEON permutation
 * 
 * @param handle Backend handle
 * @param vector_size Vector size
 * @param element_size Element size
 * @param src Source buffer
 * @param indices Permutation indices
 * @param dst Destination buffer
 * @param count Number of vectors to process
 * @return charm_status_t Status code
 */
charm_status_t neon_permute(neon_handle_t handle, 
                           neon_vector_size_t vector_size, 
                           neon_element_size_t element_size,
                           const void* src, const uint8_t* indices, 
                           void* dst, size_t count);

/**
 * @brief Perform NEON AES encryption round
 * 
 * @param handle Backend handle
 * @param src Source buffer
 * @param key Round key
 * @param dst Destination buffer
 * @param count Number of blocks to process
 * @return charm_status_t Status code
 */
charm_status_t neon_aes_encrypt_round(neon_handle_t handle, 
                                     const void* src, const void* key, 
                                     void* dst, size_t count);

/**
 * @brief Perform NEON SHA256 transform
 * 
 * @param handle Backend handle
 * @param state SHA256 state (8 uint32_t values)
 * @param block Message block (64 bytes)
 * @return charm_status_t Status code
 */
charm_status_t neon_sha256_transform(neon_handle_t handle, 
                                    uint32_t* state, const void* block);

/**
 * @brief Get string representation of NEON operation
 * 
 * @param op Operation type
 * @return const char* String representation
 */
const char* neon_op_to_string(neon_op_t op);

/**
 * @brief Get string representation of NEON vector size
 * 
 * @param size Vector size
 * @return const char* String representation
 */
const char* neon_vector_size_to_string(neon_vector_size_t size);

/**
 * @brief Get string representation of NEON element size
 * 
 * @param size Element size
 * @return const char* String representation
 */
const char* neon_element_size_to_string(neon_element_size_t size);

#ifdef __cplusplus
}
#endif

#endif /* NEON_BACKEND_H */
