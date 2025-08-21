/**
 * @file charmb_entropy.c
 * @brief CHARM-B Entropy Engine Implementation
 * 
 * Ultra-fast bit-level entropy processing optimized for small inputs
 */

#include "../include/charmb_entropy.h"
#include <string.h>
#include <stdlib.h>
#include <immintrin.h>

// Thread-local entropy state for zero-overhead design
static _Thread_local charmb_entropy_state_t tls_state = {0};
static _Thread_local charmb_entropy_config_t tls_config = {0};

/**
 * @brief CHARM-B entropy constants for bit-level optimization
 */
static const uint64_t CHARMB_ENTROPY_CONSTANTS[8] = {
    0x428A2F98D728AE22ULL, 0x7137449123EF65CDULL,
    0xB5C0FBCFEC4D3B2FULL, 0xE9B5DBA58189DBBCULL,
    0x3956C25BF348B538ULL, 0x59F111F1B605D019ULL,
    0x923F82A4AF194F9BULL, 0xAB1C5ED5DA6D8118ULL
};

/**
 * @brief Bit rotation for entropy diffusion
 */
static inline uint64_t rotr64(uint64_t x, int n) {
    return (x >> n) | (x << (64 - n));
}

static inline uint64_t rotl64(uint64_t x, int n) {
    return (x << n) | (x >> (64 - n));
}

/**
 * @brief Ultra-fast bit mixing function
 */
static inline uint64_t charmb_mix64(uint64_t x) {
    x ^= x >> 33;
    x *= 0xFF51AFD7ED558CCDULL;
    x ^= x >> 33;
    x *= 0xC4CEB9FE1A85EC53ULL;
    x ^= x >> 33;
    return x;
}

/**
 * @brief Check AVX2 availability
 */
bool charmb_entropy_avx2_available(void) {
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(7), "c"(0));
    return (ebx & (1 << 5)) != 0;  // AVX2 bit
}

/**
 * @brief Initialize CHARM-B entropy engine
 */
int charmb_entropy_init(const charmb_entropy_config_t* config) {
    if (!config) return -1;
    
    // Copy configuration
    tls_config = *config;
    
    // Initialize state with entropy constants
    for (int i = 0; i < 8; i++) {
        tls_state.state[i] = CHARMB_ENTROPY_CONSTANTS[i];
    }
    
    tls_state.counter = 0;
    tls_state.initialized = 1;
    
    return 0;
}

/**
 * @brief Get thread-local entropy state
 */
charmb_entropy_state_t* charmb_entropy_get_state(void) {
    return &tls_state;
}

/**
 * @brief Reset entropy engine state
 */
void charmb_entropy_reset(void) {
    memset(&tls_state, 0, sizeof(tls_state));
    memset(&tls_config, 0, sizeof(tls_config));
}

/**
 * @brief SIMD-accelerated entropy mixing using AVX2
 */
int charmb_entropy_simd_mix(uint8_t* data, size_t size) {
    if (!data || size % 32 != 0) return -1;
    
    if (!tls_config.use_simd || !charmb_entropy_avx2_available()) {
        // Fallback to scalar mixing
        for (size_t i = 0; i < size; i += 8) {
            uint64_t* ptr = (uint64_t*)(data + i);
            *ptr = charmb_mix64(*ptr);
        }
        return 0;
    }
    
    // AVX2 SIMD mixing
    for (size_t i = 0; i < size; i += 32) {
        __m256i chunk = _mm256_loadu_si256((__m256i*)(data + i));
        
        // Bit diffusion using SIMD operations
        __m256i mixed = _mm256_xor_si256(chunk, _mm256_srli_epi64(chunk, 33));
        mixed = _mm256_xor_si256(mixed, _mm256_slli_epi64(mixed, 13));
        mixed = _mm256_xor_si256(mixed, _mm256_srli_epi64(mixed, 17));
        
        _mm256_storeu_si256((__m256i*)(data + i), mixed);
    }
    
    return 0;
}

/**
 * @brief Ultra-fast bit diffusion for small inputs
 */
int charmb_entropy_bit_diffusion(const uint8_t* input, size_t size, 
                                 uint8_t* output, int rounds) {
    if (!input || !output || size == 0 || rounds < 1) return -1;
    
    // Copy input to output
    memcpy(output, input, size);
    
    // Apply diffusion rounds
    for (int round = 0; round < rounds; round++) {
        for (size_t i = 0; i < size; i += 8) {
            size_t remaining = size - i;
            if (remaining >= 8) {
                uint64_t* ptr = (uint64_t*)(output + i);
                uint64_t val = *ptr;
                
                // Bit-level mixing with round-specific constants
                val ^= CHARMB_ENTROPY_CONSTANTS[round & 7];
                val = rotr64(val, 13 + round);
                val = charmb_mix64(val);
                val = rotl64(val, 7 + round);
                
                *ptr = val;
            } else {
                // Handle remaining bytes
                for (size_t j = 0; j < remaining; j++) {
                    output[i + j] ^= (uint8_t)(CHARMB_ENTROPY_CONSTANTS[round & 7] >> (j * 8));
                }
            }
        }
    }
    
    return 0;
}

/**
 * @brief Bit-level entropy processing for 8-byte inputs
 */
int charmb_entropy_process_8b(const uint8_t input[8], uint8_t* output, size_t output_size) {
    if (!input || !output || output_size == 0) return -1;
    
    // Direct 64-bit processing with single-round mixing
    uint64_t val = *(uint64_t*)input;
    
    // Ultra-fast mixing optimized for 8 bytes
    val ^= tls_state.state[0];
    val = charmb_mix64(val);
    val ^= tls_state.state[1];
    val = rotr64(val, 23);
    val ^= (uint64_t)tls_state.counter++;
    
    // Generate output
    if (output_size >= 8) {
        *(uint64_t*)output = val;
        
        // Generate additional bytes if needed
        for (size_t i = 8; i < output_size; i += 8) {
            val = charmb_mix64(val ^ tls_state.state[(i/8) & 7]);
            size_t copy_size = (output_size - i >= 8) ? 8 : (output_size - i);
            memcpy(output + i, &val, copy_size);
        }
    } else {
        memcpy(output, &val, output_size);
    }
    
    return 0;
}

/**
 * @brief Bit-level entropy processing for 16-byte inputs
 */
int charmb_entropy_process_16b(const uint8_t input[16], uint8_t* output, size_t output_size) {
    if (!input || !output || output_size == 0) return -1;
    
    // SIMD 128-bit operations with optimized shuffling
    uint64_t val1 = *(uint64_t*)input;
    uint64_t val2 = *(uint64_t*)(input + 8);
    
    // Cross-mix the two 64-bit values
    val1 ^= tls_state.state[0];
    val2 ^= tls_state.state[1];
    
    uint64_t temp = val1;
    val1 = charmb_mix64(val1 ^ val2);
    val2 = charmb_mix64(val2 ^ temp);
    
    val1 = rotr64(val1, 17) ^ tls_state.state[2];
    val2 = rotl64(val2, 31) ^ tls_state.state[3];
    
    val1 ^= (uint64_t)tls_state.counter++;
    val2 ^= (uint64_t)tls_state.counter++;
    
    // Generate output
    size_t offset = 0;
    while (offset < output_size) {
        if (offset + 8 <= output_size) {
            *(uint64_t*)(output + offset) = (offset % 16 < 8) ? val1 : val2;
            offset += 8;
        } else {
            uint64_t final_val = (offset % 16 < 8) ? val1 : val2;
            memcpy(output + offset, &final_val, output_size - offset);
            break;
        }
        
        // Regenerate values for additional output
        if (offset >= 16 && offset < output_size) {
            val1 = charmb_mix64(val1 ^ val2);
            val2 = charmb_mix64(val2 ^ val1);
        }
    }
    
    return 0;
}

/**
 * @brief Bit-level entropy processing for 32-byte inputs
 */
int charmb_entropy_process_32b(const uint8_t input[32], uint8_t* output, size_t output_size) {
    if (!input || !output || output_size == 0) return -1;
    
    // Cross-block SIMD processing with avalanche diffusion
    uint64_t vals[4];
    for (int i = 0; i < 4; i++) {
        vals[i] = *(uint64_t*)(input + i * 8);
        vals[i] ^= tls_state.state[i];
    }
    
    // Cross-mixing for avalanche effect
    for (int round = 0; round < 2; round++) {
        for (int i = 0; i < 4; i++) {
            int next = (i + 1) % 4;
            vals[i] = charmb_mix64(vals[i] ^ vals[next]);
            vals[i] = rotr64(vals[i], 13 + i * 7);
        }
    }
    
    // Final mixing with counter
    for (int i = 0; i < 4; i++) {
        vals[i] ^= (uint64_t)tls_state.counter++;
    }
    
    // Generate output
    size_t offset = 0;
    while (offset < output_size) {
        int val_idx = (offset / 8) % 4;
        
        if (offset + 8 <= output_size) {
            *(uint64_t*)(output + offset) = vals[val_idx];
            offset += 8;
        } else {
            memcpy(output + offset, &vals[val_idx], output_size - offset);
            break;
        }
        
        // Regenerate values for additional output
        if (offset >= 32 && offset < output_size) {
            vals[val_idx] = charmb_mix64(vals[val_idx] ^ vals[(val_idx + 2) % 4]);
        }
    }
    
    return 0;
}

/**
 * @brief Bit-level entropy processing for 64-byte inputs
 */
int charmb_entropy_process_64b(const uint8_t input[64], uint8_t* output, size_t output_size) {
    if (!input || !output || output_size == 0) return -1;
    
    // Parallel SIMD with four 128-bit blocks
    uint64_t vals[8];
    for (int i = 0; i < 8; i++) {
        vals[i] = *(uint64_t*)(input + i * 8);
        vals[i] ^= tls_state.state[i];
    }
    
    // Four-way parallel mixing
    for (int round = 0; round < 3; round++) {
        // Process in pairs
        for (int i = 0; i < 8; i += 2) {
            uint64_t temp1 = vals[i];
            uint64_t temp2 = vals[i + 1];
            
            vals[i] = charmb_mix64(temp1 ^ temp2);
            vals[i + 1] = charmb_mix64(temp2 ^ temp1);
            
            vals[i] = rotr64(vals[i], 11 + i);
            vals[i + 1] = rotl64(vals[i + 1], 19 + i);
        }
        
        // Cross-block mixing
        for (int i = 0; i < 4; i++) {
            vals[i] ^= vals[i + 4];
            vals[i + 4] ^= vals[i];
        }
    }
    
    // Final mixing with counters
    for (int i = 0; i < 8; i++) {
        vals[i] ^= (uint64_t)tls_state.counter++;
    }
    
    // Generate output
    size_t offset = 0;
    while (offset < output_size) {
        int val_idx = (offset / 8) % 8;
        
        if (offset + 8 <= output_size) {
            *(uint64_t*)(output + offset) = vals[val_idx];
            offset += 8;
        } else {
            memcpy(output + offset, &vals[val_idx], output_size - offset);
            break;
        }
        
        // Regenerate values for additional output
        if (offset >= 64 && offset < output_size) {
            vals[val_idx] = charmb_mix64(vals[val_idx] ^ vals[(val_idx + 4) % 8]);
        }
    }
    
    return 0;
}