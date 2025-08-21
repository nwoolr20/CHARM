/**
 * @file charmb_core.c
 * @brief CHARM-B (CHARMbit) implementation - Ultra-optimized for small inputs ≤64B
 * 
 * CHARM-B uses bit-level operations, specialized SIMD instructions, and 
 * hyper-optimized algorithms for maximum performance on very small inputs.
 */

#include "charmb_core.h"
#include <string.h>
#include <immintrin.h>

// Compile-time feature detection
#ifdef __AVX2__
#define CHARMB_HAS_AVX2 1
#else
#define CHARMB_HAS_AVX2 0
#endif

#ifdef __BMI2__
#define CHARMB_HAS_BMI2 1
#else 
#define CHARMB_HAS_BMI2 0
#endif

// Ultra-fast entropy seeds optimized for bit operations
static const uint64_t CHARMB_ENTROPY_SEEDS[8] = {
    0x9e3779b97f4a7c15ULL, 0x85ebca6b6b976e52ULL,
    0xa5a5a5a5a5a5a5a5ULL, 0x3c6ef372fe94f82bULL,
    0xdea57ca5f1d6b7f8ULL, 0x6a09e667f3bcc908ULL,
    0x4e2d88d3e94f82b4ULL, 0x0123456789abcdefULL
};

// High-performance bit manipulation constants
static const uint32_t CHARMB_BIT_PATTERNS[16] = {
    0x55555555, 0xAAAAAAAA, 0x33333333, 0xCCCCCCCC,
    0x0F0F0F0F, 0xF0F0F0F0, 0x00FF00FF, 0xFF00FF00,
    0x0000FFFF, 0xFFFF0000, 0x01010101, 0xFEFEFEFE,
    0x80808080, 0x7F7F7F7F, 0x12345678, 0x87654321
};

/**
 * @brief High-precision CPU cycle counter for entropy
 */
static inline uint64_t charmb_rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/**
 * @brief Ultra-fast bit-level entropy mixing
 */
static inline uint64_t charmb_bit_mix(uint64_t x, uint64_t seed) {
    x ^= seed;
    x ^= x >> 31;
    x *= 0x7fb5d329728ea185ULL;
    x ^= x >> 27;
    x *= 0x81dadef4bc2dd44dULL;
    x ^= x >> 33;
    return x;
}

/**
 * @brief Hyper-optimized 64-bit rotation 
 */
static inline uint64_t charmb_rotl64(uint64_t x, int shift) {
    return (x << shift) | (x >> (64 - shift));
}

/**
 * @brief SIMD-accelerated bit shuffling for 128-bit blocks
 */
static inline void charmb_simd_shuffle_128(__m128i* block, uint64_t entropy) {
#if CHARMB_HAS_AVX2
    __m128i entropy_vec = _mm_set1_epi64x((long long)entropy);
    *block = _mm_xor_si128(*block, entropy_vec);
    
    // Ultra-fast bit permutation using SIMD
    __m128i shuffled = _mm_shuffle_epi8(*block, 
        _mm_set_epi8(0, 1, 14, 15, 4, 5, 10, 11, 8, 9, 6, 7, 12, 13, 2, 3));
    *block = _mm_xor_si128(*block, shuffled);
    
    // Fast avalanche diffusion
    *block = _mm_xor_si128(*block, _mm_srli_epi64(*block, 13));
    *block = _mm_xor_si128(*block, _mm_slli_epi64(*block, 7));
#else
    // Fallback for non-AVX2 systems
    uint64_t* data = (uint64_t*)block;
    data[0] = charmb_bit_mix(data[0], entropy);
    data[1] = charmb_bit_mix(data[1], entropy >> 8);
#endif
}

/**
 * @brief Ultra-fast 8-byte specialized hash implementation
 */
charmb_status_t charmb_hash_8b(const uint8_t* data, uint8_t* digest, 
                              charmb_digest_size_t digest_size) {
    if (data == NULL || digest == NULL) {
        return CHARMB_STATUS_INVALID_ARG;
    }
    
    // Direct 64-bit processing with hyper-optimization
    uint64_t input = *(const uint64_t*)data;
    uint64_t entropy = charmb_rdtsc() ^ CHARMB_ENTROPY_SEEDS[0];
    
    // Ultra-fast single-round bit mixing optimized for 8 bytes
    uint64_t state = charmb_bit_mix(input, entropy);
    
    // Additional bit-level mixing for security
    state = charmb_rotl64(state, 21) ^ charmb_rotl64(state, 35);
    state ^= CHARMB_BIT_PATTERNS[state & 0xF];
    
    if (digest_size == CHARMB_DIGEST_128) {
        // Generate 128-bit output
        uint64_t state2 = charmb_bit_mix(state, entropy >> 16);
        *(uint64_t*)digest = state;
        *(uint64_t*)(digest + 8) = state2;
    } else {
        // Generate 256-bit output
        uint64_t states[4];
        states[0] = state;
        states[1] = charmb_bit_mix(state, entropy >> 8);
        states[2] = charmb_bit_mix(state, entropy >> 16);
        states[3] = charmb_bit_mix(state, entropy >> 24);
        memcpy(digest, states, 32);
    }
    
    return CHARMB_STATUS_OK;
}

/**
 * @brief Ultra-fast 16-byte specialized hash implementation
 */
charmb_status_t charmb_hash_16b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size) {
    if (data == NULL || digest == NULL) {
        return CHARMB_STATUS_INVALID_ARG;
    }
    
    // Use SIMD for maximum 16-byte performance
    __m128i input = _mm_loadu_si128((const __m128i*)data);
    uint64_t entropy = charmb_rdtsc() ^ CHARMB_ENTROPY_SEEDS[1];
    
    // Ultra-fast SIMD bit mixing
    charmb_simd_shuffle_128(&input, entropy);
    
    if (digest_size == CHARMB_DIGEST_128) {
        _mm_storeu_si128((__m128i*)digest, input);
    } else {
        // Expand to 256-bit with additional mixing
        __m128i second_half = input;
        charmb_simd_shuffle_128(&second_half, entropy >> 16);
        
        _mm_storeu_si128((__m128i*)digest, input);
        _mm_storeu_si128((__m128i*)(digest + 16), second_half);
    }
    
    return CHARMB_STATUS_OK;
}

/**
 * @brief Ultra-fast 32-byte specialized hash implementation
 */
charmb_status_t charmb_hash_32b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size) {
    if (data == NULL || digest == NULL) {
        return CHARMB_STATUS_INVALID_ARG;
    }
    
    // Process as two 128-bit blocks with cross-mixing
    __m128i block1 = _mm_loadu_si128((const __m128i*)data);
    __m128i block2 = _mm_loadu_si128((const __m128i*)(data + 16));
    
    uint64_t entropy = charmb_rdtsc() ^ CHARMB_ENTROPY_SEEDS[2];
    
    // Ultra-fast cross-block mixing
    charmb_simd_shuffle_128(&block1, entropy);
    charmb_simd_shuffle_128(&block2, entropy >> 8);
    
    // Cross-block avalanche
    block1 = _mm_xor_si128(block1, block2);
    block2 = _mm_xor_si128(block2, block1);
    
    if (digest_size == CHARMB_DIGEST_128) {
        _mm_storeu_si128((__m128i*)digest, block1);
    } else {
        _mm_storeu_si128((__m128i*)digest, block1);
        _mm_storeu_si128((__m128i*)(digest + 16), block2);
    }
    
    return CHARMB_STATUS_OK;
}

/**
 * @brief Ultra-fast 64-byte specialized hash implementation
 */
charmb_status_t charmb_hash_64b(const uint8_t* data, uint8_t* digest, 
                               charmb_digest_size_t digest_size) {
    if (data == NULL || digest == NULL) {
        return CHARMB_STATUS_INVALID_ARG;
    }
    
    // Process as four 128-bit blocks with maximum SIMD efficiency
    __m128i blocks[4];
    for (int i = 0; i < 4; i++) {
        blocks[i] = _mm_loadu_si128((const __m128i*)(data + i * 16));
    }
    
    uint64_t entropy = charmb_rdtsc() ^ CHARMB_ENTROPY_SEEDS[3];
    
    // Ultra-fast parallel processing with cross-block mixing
    for (int i = 0; i < 4; i++) {
        charmb_simd_shuffle_128(&blocks[i], entropy >> (i * 4));
    }
    
    // Cross-block avalanche diffusion
    blocks[0] = _mm_xor_si128(blocks[0], blocks[2]);
    blocks[1] = _mm_xor_si128(blocks[1], blocks[3]);
    blocks[2] = _mm_xor_si128(blocks[2], blocks[0]);
    blocks[3] = _mm_xor_si128(blocks[3], blocks[1]);
    
    if (digest_size == CHARMB_DIGEST_128) {
        // Collapse to 128-bit
        __m128i result = _mm_xor_si128(blocks[0], blocks[1]);
        result = _mm_xor_si128(result, blocks[2]);
        result = _mm_xor_si128(result, blocks[3]);
        _mm_storeu_si128((__m128i*)digest, result);
    } else {
        // Generate 256-bit output
        __m128i result1 = _mm_xor_si128(blocks[0], blocks[1]);
        __m128i result2 = _mm_xor_si128(blocks[2], blocks[3]);
        _mm_storeu_si128((__m128i*)digest, result1);
        _mm_storeu_si128((__m128i*)(digest + 16), result2);
    }
    
    return CHARMB_STATUS_OK;
}

/**
 * @brief Bit-level CHARM-B for arbitrary small inputs
 */
charmb_status_t charmb_hash_bitlevel(const uint8_t* data, size_t data_size,
                                    uint8_t* digest, charmb_digest_size_t digest_size) {
    if (data == NULL || data_size == 0 || digest == NULL) {
        return CHARMB_STATUS_INVALID_ARG;
    }
    
    if (data_size > 64) {
        return CHARMB_STATUS_TOO_LARGE;
    }
    
    // Use static state for zero malloc overhead
    static __thread uint8_t bit_state[32] = {
        0x6a, 0x09, 0xe6, 0x67, 0xf3, 0xbc, 0xc9, 0x08,
        0xbb, 0x67, 0xae, 0x85, 0x84, 0xca, 0xa7, 0x3b,
        0x3c, 0x6e, 0xf3, 0x72, 0xfe, 0x94, 0xf8, 0x2b,
        0xa5, 0x4f, 0xf5, 0x3a, 0x5f, 0x1d, 0x36, 0xf1
    };
    
    uint64_t entropy = charmb_rdtsc() ^ CHARMB_ENTROPY_SEEDS[data_size & 7];
    
    // Bit-level processing with maximum efficiency
    uint64_t* state64 = (uint64_t*)bit_state;
    
    // Process input bit by bit for maximum mixing
    for (size_t byte_idx = 0; byte_idx < data_size; byte_idx++) {
        uint8_t input_byte = data[byte_idx];
        
        // Bit-level mixing - each bit affects state differently
        for (int bit = 0; bit < 8; bit++) {
            if (input_byte & (1 << bit)) {
                int state_idx = (byte_idx * 8 + bit) % 4;
                state64[state_idx] = charmb_bit_mix(state64[state_idx], 
                    entropy >> ((byte_idx + bit) & 31));
            }
        }
    }
    
    // Final bit-level avalanche
    for (int i = 0; i < 4; i++) {
        state64[i] = charmb_rotl64(state64[i], 13 + i * 7);
        state64[i] ^= CHARMB_BIT_PATTERNS[(i * data_size) & 15];
    }
    
    // Output generation
    if (digest_size == CHARMB_DIGEST_128) {
        memcpy(digest, bit_state, 16);
    } else {
        memcpy(digest, bit_state, 32);
    }
    
    return CHARMB_STATUS_OK;
}

/**
 * @brief Main CHARM-B hash function with automatic size optimization
 */
charmb_status_t charmb_hash(const uint8_t* data, size_t data_size, 
                           uint8_t* digest, charmb_digest_size_t digest_size) {
    if (data == NULL || data_size == 0 || digest == NULL) {
        return CHARMB_STATUS_INVALID_ARG;
    }
    
    if (data_size > 64) {
        return CHARMB_STATUS_TOO_LARGE;
    }
    
    // Automatic dispatch to specialized functions for maximum performance
    switch (data_size) {
        case 8:
            return charmb_hash_8b(data, digest, digest_size);
        case 16:
            return charmb_hash_16b(data, digest, digest_size);
        case 32:
            return charmb_hash_32b(data, digest, digest_size);
        case 64:
            return charmb_hash_64b(data, digest, digest_size);
        default:
            return charmb_hash_bitlevel(data, data_size, digest, digest_size);
    }
}

/**
 * @brief Get CHARM-B version and features
 */
const char* charmb_get_version(void) {
    return "CHARM-B 1.0 (CHARMbit) - "
#if CHARMB_HAS_AVX2
           "AVX2 "
#endif
#if CHARMB_HAS_BMI2
           "BMI2 "
#endif
           "Optimized";
}

/**
 * @brief Check CPU support for CHARM-B optimizations
 */
bool charmb_cpu_support(void) {
#if CHARMB_HAS_AVX2
    return true;
#else
    return false;
#endif
}