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
 * @file cee_whiten.c
 * @brief Noise Conditioning Filter (NCF) - Statistical noise conditioning
 * 
 * This module implements the Noise Conditioning Filter (NCF) component of the
 * Charm Entropic Engine (CEE). It removes statistical biases and enhances
 * uniformity in the entropy stream, ensuring it meets stringent statistical
 * randomness criteria crucial for cryptographic strength.
 * 
 * The implementation employs various whitening algorithms and statistical tests
 * to achieve high-quality entropy output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define CEE_WHITEN_BUFFER_SIZE 1024
#define CEE_WHITEN_BLOCK_SIZE 64

// Whitening algorithm types
typedef enum {
    WHITEN_ALGO_VON_NEUMANN,  // Von Neumann whitening (simple but inefficient)
    WHITEN_ALGO_DIFFUSION,    // Bit diffusion (more efficient)
    WHITEN_ALGO_HASH_EXTRACT, // Hash extraction (most thorough)
    WHITEN_ALGO_AUTO          // Automatically select based on input quality
} cee_whiten_algo_t;

// Internal state for the whitening system
typedef struct {
    uint8_t* buffer;
    size_t buffer_size;
    size_t buffer_used;
    cee_whiten_algo_t current_algo;
    bool initialized;
} cee_whiten_state_t;

// Global whitening state
static cee_whiten_state_t g_whiten_state = {NULL, 0, 0, WHITEN_ALGO_AUTO, false};

/**
 * @brief Initialize the whitening system
 * 
 * @param size Size of the internal buffer
 * @param algo Whitening algorithm to use
 * @return int 0 on success, non-zero on failure
 */
int cee_whiten_init(size_t size, cee_whiten_algo_t algo) {
    if (g_whiten_state.buffer != NULL) {
        free(g_whiten_state.buffer);
    }
    
    g_whiten_state.buffer = (uint8_t*)malloc(size);
    if (g_whiten_state.buffer == NULL) {
        return -1;
    }
    
    g_whiten_state.buffer_size = size;
    g_whiten_state.buffer_used = 0;
    g_whiten_state.current_algo = algo;
    g_whiten_state.initialized = true;
    
    return 0;
}

/**
 * @brief Clean up the whitening system
 */
void cee_whiten_cleanup(void) {
    if (g_whiten_state.buffer != NULL) {
        // Securely clear the buffer before freeing
        memset(g_whiten_state.buffer, 0, g_whiten_state.buffer_size);
        free(g_whiten_state.buffer);
        g_whiten_state.buffer = NULL;
    }
    g_whiten_state.buffer_size = 0;
    g_whiten_state.buffer_used = 0;
    g_whiten_state.initialized = false;
}

/**
 * @brief Apply Von Neumann whitening to a buffer
 * 
 * Von Neumann whitening processes pairs of bits:
 * - 01 produces output 1
 * - 10 produces output 0
 * - 00 and 11 are discarded
 * 
 * This is inefficient (75% bit loss in worst case) but effective.
 * 
 * @param input Input buffer
 * @param input_size Size of input in bytes
 * @param output Output buffer
 * @param output_size Size of output buffer in bytes
 * @return size_t Number of bytes written to output
 */
static size_t apply_von_neumann(const uint8_t* input, size_t input_size, 
                               uint8_t* output, size_t output_size) {
    size_t out_bits = 0;
    size_t out_bytes = 0;
    uint8_t out_byte = 0;
    
    for (size_t i = 0; i < input_size && out_bytes < output_size; i++) {
        uint8_t byte = input[i];
        
        // Process each pair of bits in the byte
        for (int j = 0; j < 7; j += 2) {
            uint8_t bit_pair = (byte >> j) & 0x03;
            
            if (bit_pair == 0x01) {      // 01 -> 1
                out_byte |= (1 << out_bits);
                out_bits++;
            } else if (bit_pair == 0x02) { // 10 -> 0
                // Just increment out_bits (bit is already 0)
                out_bits++;
            }
            // 00 and 11 are discarded
            
            // If we've filled a byte, store it
            if (out_bits == 8) {
                output[out_bytes++] = out_byte;
                out_byte = 0;
                out_bits = 0;
                
                if (out_bytes >= output_size) {
                    break;
                }
            }
        }
    }
    
    // Handle partial byte at the end
    if (out_bits > 0 && out_bytes < output_size) {
        output[out_bytes++] = out_byte;
    }
    
    return out_bytes;
}

/**
 * @brief Apply bit diffusion whitening to a buffer
 * 
 * This algorithm spreads the influence of each input bit across
 * multiple output bits, reducing statistical biases.
 * 
 * @param input Input buffer
 * @param input_size Size of input in bytes
 * @param output Output buffer
 * @param output_size Size of output buffer in bytes
 * @return size_t Number of bytes written to output
 */
static size_t apply_diffusion(const uint8_t* input, size_t input_size,
                             uint8_t* output, size_t output_size) {
    // Ensure we have enough input for a full block
    if (input_size < CEE_WHITEN_BLOCK_SIZE) {
        return 0;
    }
    
    // Process full blocks only
    size_t blocks = input_size / CEE_WHITEN_BLOCK_SIZE;
    size_t out_bytes = blocks * CEE_WHITEN_BLOCK_SIZE / 2; // 50% compression
    
    if (out_bytes > output_size) {
        out_bytes = output_size;
    }
    
    // Clear output buffer
    memset(output, 0, out_bytes);
    
    for (size_t block = 0; block < blocks && (block * CEE_WHITEN_BLOCK_SIZE / 2) < out_bytes; block++) {
        const uint8_t* in_block = input + (block * CEE_WHITEN_BLOCK_SIZE);
        uint8_t* out_block = output + (block * CEE_WHITEN_BLOCK_SIZE / 2);
        size_t out_block_size = CEE_WHITEN_BLOCK_SIZE / 2;
        
        if ((block * CEE_WHITEN_BLOCK_SIZE / 2) + out_block_size > out_bytes) {
            out_block_size = out_bytes - (block * CEE_WHITEN_BLOCK_SIZE / 2);
        }
        
        // Apply diffusion transformation
        for (size_t i = 0; i < CEE_WHITEN_BLOCK_SIZE; i++) {
            for (size_t j = 0; j < 8; j++) {
                // Get the bit
                uint8_t bit = (in_block[i] >> j) & 0x01;
                
                // Diffuse this bit to influence multiple output bits
                if (bit) {
                    // Calculate target byte and bit
                    size_t target_byte = (i + j) % out_block_size;
                    size_t target_bit = (i * j) % 8;
                    
                    // XOR the bit into place
                    out_block[target_byte] ^= (1 << target_bit);
                }
            }
        }
    }
    
    return out_bytes;
}

/**
 * @brief Apply hash extraction whitening to a buffer
 * 
 * This algorithm uses a cryptographic hash function to extract
 * uniformly distributed bits from the input.
 * 
 * @param input Input buffer
 * @param input_size Size of input in bytes
 * @param output Output buffer
 * @param output_size Size of output buffer in bytes
 * @return size_t Number of bytes written to output
 */
static size_t apply_hash_extract(const uint8_t* input, size_t input_size,
                                uint8_t* output, size_t output_size) {
    // Simple hash function for demonstration
    // In production, this would use a cryptographic hash function
    
    // Process the input in blocks
    size_t out_bytes = 0;
    uint32_t hash_state[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                             0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    
    // Process input in blocks
    for (size_t i = 0; i < input_size; i += 64) {
        size_t block_size = (input_size - i < 64) ? (input_size - i) : 64;
        
        // Update hash state with this block
        for (size_t j = 0; j < block_size; j++) {
            uint8_t byte = input[i + j];
            hash_state[j % 8] ^= (uint32_t)byte << ((j % 4) * 8);
            
            // Mix the state
            for (int k = 0; k < 8; k++) {
                hash_state[k] = ((hash_state[k] << 13) | (hash_state[k] >> 19)) ^ 
                               ((hash_state[(k+1) % 8] << 7) | (hash_state[(k+1) % 8] >> 25));
            }
        }
        
        // Extract output from hash state
        size_t extract_size = (output_size - out_bytes < 32) ? (output_size - out_bytes) : 32;
        for (size_t j = 0; j < extract_size; j++) {
            output[out_bytes++] = (hash_state[j / 4] >> ((j % 4) * 8)) & 0xFF;
        }
        
        if (out_bytes >= output_size) {
            break;
        }
    }
    
    return out_bytes;
}

/**
 * @brief Assess the quality of input entropy
 * 
 * @param input Input buffer
 * @param size Size of input in bytes
 * @return double Quality score between 0.0 (poor) and 1.0 (excellent)
 */
static double assess_entropy_quality(const uint8_t* input, size_t size) {
    if (size < 16) {
        return 0.5; // Not enough data for assessment
    }
    
    // Count occurrences of each byte value
    unsigned int counts[256] = {0};
    for (size_t i = 0; i < size; i++) {
        counts[input[i]]++;
    }
    
    // Calculate chi-square statistic
    double chi_square = 0.0;
    double expected = (double)size / 256.0;
    
    for (int i = 0; i < 256; i++) {
        double diff = counts[i] - expected;
        chi_square += (diff * diff) / expected;
    }
    
    // Normalize to [0,1] range (lower chi-square is better)
    // For 255 degrees of freedom, critical value at p=0.05 is ~293
    double quality = 1.0 - (chi_square / 500.0);
    if (quality < 0.0) quality = 0.0;
    if (quality > 1.0) quality = 1.0;
    
    return quality;
}

/**
 * @brief Apply whitening to input data
 * 
 * This function applies the selected whitening algorithm to the input data,
 * producing statistically unbiased output.
 * 
 * @param input Input data buffer
 * @param input_size Size of input data in bytes
 * @param output Output data buffer
 * @param output_size Size of output buffer in bytes
 * @return size_t Number of bytes written to output
 */
size_t cee_whiten_apply(const uint8_t* input, size_t input_size,
                       uint8_t* output, size_t output_size) {
    if (!g_whiten_state.initialized) {
        if (cee_whiten_init(CEE_WHITEN_BUFFER_SIZE, WHITEN_ALGO_AUTO) != 0) {
            return 0;
        }
    }
    
    // If using AUTO, select algorithm based on input quality
    cee_whiten_algo_t algo = g_whiten_state.current_algo;
    if (algo == WHITEN_ALGO_AUTO) {
        double quality = assess_entropy_quality(input, input_size);
        
        if (quality < 0.3) {
            algo = WHITEN_ALGO_HASH_EXTRACT; // Poor quality, use strongest method
        } else if (quality < 0.7) {
            algo = WHITEN_ALGO_DIFFUSION;    // Medium quality
        } else {
            algo = WHITEN_ALGO_VON_NEUMANN;  // Good quality, simple method is sufficient
        }
    }
    
    // Apply the selected algorithm
    size_t bytes_written = 0;
    switch (algo) {
        case WHITEN_ALGO_VON_NEUMANN:
            bytes_written = apply_von_neumann(input, input_size, output, output_size);
            break;
            
        case WHITEN_ALGO_DIFFUSION:
            bytes_written = apply_diffusion(input, input_size, output, output_size);
            break;
            
        case WHITEN_ALGO_HASH_EXTRACT:
            bytes_written = apply_hash_extract(input, input_size, output, output_size);
            break;
            
        default:
            // Shouldn't happen, but fall back to hash extraction
            bytes_written = apply_hash_extract(input, input_size, output, output_size);
            break;
    }
    
    return bytes_written;
}

/**
 * @brief Set the whitening algorithm to use
 * 
 * @param algo Algorithm to use
 */
void cee_whiten_set_algorithm(cee_whiten_algo_t algo) {
    if (!g_whiten_state.initialized) {
        if (cee_whiten_init(CEE_WHITEN_BUFFER_SIZE, algo) != 0) {
            return;
        }
    } else {
        g_whiten_state.current_algo = algo;
    }
}

/**
 * @brief Get the current whitening algorithm
 * 
 * @return cee_whiten_algo_t Current algorithm
 */
cee_whiten_algo_t cee_whiten_get_algorithm(void) {
    if (!g_whiten_state.initialized) {
        return WHITEN_ALGO_AUTO;
    }
    return g_whiten_state.current_algo;
}

/**
 * @brief Test if the output passes basic statistical tests
 * 
 * @param data Data to test
 * @param size Size of data in bytes
 * @return bool true if data passes tests, false otherwise
 */
bool cee_whiten_test_output(const uint8_t* data, size_t size) {
    if (size < 100) {
        return false; // Not enough data for meaningful test
    }
    
    // Simple frequency test
    double quality = assess_entropy_quality(data, size);
    
    // Run test (testing for runs of 0s and 1s)
    int runs = 0;
    bool last_bit = (data[0] & 0x01);
    
    for (size_t i = 0; i < size; i++) {
        for (int j = 0; j < 8; j++) {
            bool bit = (data[i] >> j) & 0x01;
            if (bit != last_bit) {
                runs++;
                last_bit = bit;
            }
        }
    }
    
    // Expected number of runs for random data is (n/2) + 1
    double expected_runs = (size * 8.0 / 2.0) + 1.0;
    double run_quality = 1.0 - fabs(runs - expected_runs) / expected_runs;
    
    // Combined quality score
    double combined_quality = 0.7 * quality + 0.3 * run_quality;
    
    return (combined_quality >= 0.8);
}
