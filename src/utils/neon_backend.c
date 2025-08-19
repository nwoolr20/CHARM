/**
 * @file neon_backend.c
 * @brief NEON-optimized implementation for ARM platforms
 * 
 * This module provides NEON-optimized implementations of key cryptographic
 * operations for ARM platforms, improving performance on devices that
 * support the NEON SIMD instruction set.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__)
#include <arm_neon.h>
#define HAVE_NEON 1
#else
#define HAVE_NEON 0
#endif

/**
 * @brief Check if NEON is available at runtime
 * 
 * @return bool true if NEON is available, false otherwise
 */
bool neon_is_available(void) {
#if HAVE_NEON
    return true;
#else
    return false;
#endif
}

/**
 * @brief NEON-optimized block mixing function
 * 
 * This function performs a mixing operation on a block of data using NEON
 * SIMD instructions when available.
 * 
 * @param state State buffer (16 32-bit words)
 * @param input Input block (16 32-bit words)
 * @param output Output block (16 32-bit words)
 */
void neon_mix_block(uint32_t* state, const uint32_t* input, uint32_t* output) {
#if HAVE_NEON
    // Load state and input into NEON registers
    uint32x4_t s0 = vld1q_u32(state);
    uint32x4_t s1 = vld1q_u32(state + 4);
    uint32x4_t s2 = vld1q_u32(state + 8);
    uint32x4_t s3 = vld1q_u32(state + 12);
    
    uint32x4_t i0 = vld1q_u32(input);
    uint32x4_t i1 = vld1q_u32(input + 4);
    uint32x4_t i2 = vld1q_u32(input + 8);
    uint32x4_t i3 = vld1q_u32(input + 12);
    
    // Mix state with input
    uint32x4_t x0 = veorq_u32(s0, i0);
    uint32x4_t x1 = veorq_u32(s1, i1);
    uint32x4_t x2 = veorq_u32(s2, i2);
    uint32x4_t x3 = veorq_u32(s3, i3);
    
    // Perform multiple rounds of mixing
    for (int i = 0; i < 8; i++) {
        // Quarter round on x0, x1, x2, x3
        x0 = vaddq_u32(x0, x1);
        x3 = veorq_u32(x3, x0);
        x3 = vorrq_u32(vshlq_n_u32(x3, 16), vshrq_n_u32(x3, 16)); // ROL 16
        
        x2 = vaddq_u32(x2, x3);
        x1 = veorq_u32(x1, x2);
        x1 = vorrq_u32(vshlq_n_u32(x1, 12), vshrq_n_u32(x1, 20)); // ROL 12
        
        x0 = vaddq_u32(x0, x1);
        x3 = veorq_u32(x3, x0);
        x3 = vorrq_u32(vshlq_n_u32(x3, 8), vshrq_n_u32(x3, 24)); // ROL 8
        
        x2 = vaddq_u32(x2, x3);
        x1 = veorq_u32(x1, x2);
        x1 = vorrq_u32(vshlq_n_u32(x1, 7), vshrq_n_u32(x1, 25)); // ROL 7
        
        // Shuffle for next round
        x1 = vextq_u32(x1, x1, 1);
        x2 = vextq_u32(x2, x2, 2);
        x3 = vextq_u32(x3, x3, 3);
        
        // Quarter round on x0, x1, x2, x3 (different pattern)
        x0 = vaddq_u32(x0, x1);
        x3 = veorq_u32(x3, x0);
        x3 = vorrq_u32(vshlq_n_u32(x3, 16), vshrq_n_u32(x3, 16)); // ROL 16
        
        x2 = vaddq_u32(x2, x3);
        x1 = veorq_u32(x1, x2);
        x1 = vorrq_u32(vshlq_n_u32(x1, 12), vshrq_n_u32(x1, 20)); // ROL 12
        
        x0 = vaddq_u32(x0, x1);
        x3 = veorq_u32(x3, x0);
        x3 = vorrq_u32(vshlq_n_u32(x3, 8), vshrq_n_u32(x3, 24)); // ROL 8
        
        x2 = vaddq_u32(x2, x3);
        x1 = veorq_u32(x1, x2);
        x1 = vorrq_u32(vshlq_n_u32(x1, 7), vshrq_n_u32(x1, 25)); // ROL 7
        
        // Restore original positions
        x1 = vextq_u32(x1, x1, 3);
        x2 = vextq_u32(x2, x2, 2);
        x3 = vextq_u32(x3, x3, 1);
    }
    
    // Add back to state
    s0 = vaddq_u32(s0, x0);
    s1 = vaddq_u32(s1, x1);
    s2 = vaddq_u32(s2, x2);
    s3 = vaddq_u32(s3, x3);
    
    // Store result
    vst1q_u32(output, s0);
    vst1q_u32(output + 4, s1);
    vst1q_u32(output + 8, s2);
    vst1q_u32(output + 12, s3);
#else
    // Fallback implementation for non-NEON platforms
    memcpy(output, state, 16 * sizeof(uint32_t));
    
    for (int i = 0; i < 16; i++) {
        output[i] ^= input[i];
    }
    
    // Simple mixing function
    for (int r = 0; r < 8; r++) {
        for (int i = 0; i < 16; i++) {
            output[i] += output[(i + 1) % 16];
            output[(i + 8) % 16] ^= output[i];
            output[(i + 8) % 16] = (output[(i + 8) % 16] << 7) | (output[(i + 8) % 16] >> 25);
        }
    }
#endif
}

/**
 * @brief NEON-optimized hash update function
 * 
 * This function updates a hash state with new data using NEON
 * SIMD instructions when available.
 * 
 * @param state Hash state (8 32-bit words)
 * @param data Input data
 * @param len Length of input data in bytes
 */
void neon_hash_update(uint32_t* state, const uint8_t* data, size_t len) {
#if HAVE_NEON
    // Process data in 64-byte blocks
    while (len >= 64) {
        uint32x4_t s0 = vld1q_u32(state);
        uint32x4_t s1 = vld1q_u32(state + 4);
        
        // Load data (handling potential unaligned access)
        uint32x4_t d0 = vld1q_u32((const uint32_t*)data);
        uint32x4_t d1 = vld1q_u32((const uint32_t*)(data + 16));
        uint32x4_t d2 = vld1q_u32((const uint32_t*)(data + 32));
        uint32x4_t d3 = vld1q_u32((const uint32_t*)(data + 48));
        
        // Mix data into state
        s0 = veorq_u32(s0, d0);
        s1 = veorq_u32(s1, d1);
        
        // Perform multiple rounds of mixing
        for (int i = 0; i < 4; i++) {
            // Mix with data
            s0 = veorq_u32(s0, d2);
            s1 = veorq_u32(s1, d3);
            
            // Rotate and shuffle
            uint32x4_t t0 = vaddq_u32(s0, s1);
            uint32x4_t t1 = veorq_u32(s1, t0);
            t1 = vorrq_u32(vshlq_n_u32(t1, 13), vshrq_n_u32(t1, 19)); // ROL 13
            
            s1 = t1;
            s0 = t0;
            
            // Shuffle data for next round
            uint32x4_t tmp = d0;
            d0 = d1;
            d1 = d2;
            d2 = d3;
            d3 = tmp;
        }
        
        // Store updated state
        vst1q_u32(state, s0);
        vst1q_u32(state + 4, s1);
        
        data += 64;
        len -= 64;
    }
#else
    // Fallback implementation for non-NEON platforms
    // Process data in 64-byte blocks
    while (len >= 64) {
        // Mix data into state
        for (int i = 0; i < 8; i++) {
            uint32_t word = 0;
            for (int j = 0; j < 4; j++) {
                word |= ((uint32_t)data[i*4 + j]) << (j * 8);
            }
            state[i] ^= word;
        }
        
        // Simple mixing function
        for (int r = 0; r < 4; r++) {
            for (int i = 0; i < 8; i++) {
                state[i] += state[(i + 1) % 8];
                state[(i + 4) % 8] ^= state[i];
                state[(i + 4) % 8] = (state[(i + 4) % 8] << 13) | (state[(i + 4) % 8] >> 19);
            }
        }
        
        data += 64;
        len -= 64;
    }
#endif

    // Handle remaining bytes (if any)
    if (len > 0) {
        uint8_t buffer[64] = {0};
        memcpy(buffer, data, len);
        
        // Process the final partial block
#if HAVE_NEON
        uint32x4_t s0 = vld1q_u32(state);
        uint32x4_t s1 = vld1q_u32(state + 4);
        
        uint32x4_t d0 = vld1q_u32((const uint32_t*)buffer);
        uint32x4_t d1 = vld1q_u32((const uint32_t*)(buffer + 16));
        uint32x4_t d2 = vld1q_u32((const uint32_t*)(buffer + 32));
        uint32x4_t d3 = vld1q_u32((const uint32_t*)(buffer + 48));
        
        s0 = veorq_u32(s0, d0);
        s1 = veorq_u32(s1, d1);
        
        for (int i = 0; i < 4; i++) {
            s0 = veorq_u32(s0, d2);
            s1 = veorq_u32(s1, d3);
            
            uint32x4_t t0 = vaddq_u32(s0, s1);
            uint32x4_t t1 = veorq_u32(s1, t0);
            t1 = vorrq_u32(vshlq_n_u32(t1, 13), vshrq_n_u32(t1, 19));
            
            s1 = t1;
            s0 = t0;
            
            uint32x4_t tmp = d0;
            d0 = d1;
            d1 = d2;
            d2 = d3;
            d3 = tmp;
        }
        
        vst1q_u32(state, s0);
        vst1q_u32(state + 4, s1);
#else
        for (int i = 0; i < 8; i++) {
            uint32_t word = 0;
            for (int j = 0; j < 4; j++) {
                word |= ((uint32_t)buffer[i*4 + j]) << (j * 8);
            }
            state[i] ^= word;
        }
        
        for (int r = 0; r < 4; r++) {
            for (int i = 0; i < 8; i++) {
                state[i] += state[(i + 1) % 8];
                state[(i + 4) % 8] ^= state[i];
                state[(i + 4) % 8] = (state[(i + 4) % 8] << 13) | (state[(i + 4) % 8] >> 19);
            }
        }
#endif
    }
}

/**
 * @brief NEON-optimized entropy whitening function
 * 
 * This function applies whitening to entropy data using NEON
 * SIMD instructions when available.
 * 
 * @param data Input/output data buffer
 * @param len Length of data in bytes
 */
void neon_whiten_entropy(uint8_t* data, size_t len) {
#if HAVE_NEON
    // Process data in 16-byte chunks
    size_t i = 0;
    for (; i + 16 <= len; i += 16) {
        // Load 16 bytes
        uint8x16_t chunk = vld1q_u8(data + i);
        
        // Apply transformations
        // 1. Byte-wise operations
        uint8x16_t shuffled = vrev64q_u8(chunk); // Reverse bytes within 64-bit elements
        
        // 2. XOR with rotated self
        uint8x16_t rotated = vextq_u8(chunk, chunk, 8); // Rotate by 8 bytes
        uint8x16_t mixed = veorq_u8(shuffled, rotated);
        
        // 3. Diffuse bits
        uint8x16_t diffused = mixed;
        for (int j = 0; j < 3; j++) {
            uint8x16_t shifted = vextq_u8(diffused, diffused, 1); // Rotate by 1 byte
            diffused = veorq_u8(diffused, shifted);
        }
        
        // Store result
        vst1q_u8(data + i, diffused);
    }
    
    // Handle remaining bytes
    for (; i < len; i++) {
        data[i] ^= data[(i + 7) % len];
    }
#else
    // Fallback implementation for non-NEON platforms
    // Simple whitening algorithm
    for (size_t i = 0; i < len; i++) {
        data[i] ^= data[(i + 7) % len];
    }
    
    // Additional mixing
    for (size_t i = 0; i < len; i++) {
        data[i] = (data[i] << 3) | (data[i] >> 5);
        data[i] ^= data[(i + len/2) % len];
    }
#endif
}
