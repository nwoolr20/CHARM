/**
 * @file ece_seed.c
 * @brief Implementation of seed generation and management for the Entropic Collapse Engine
 * 
 * This file implements the seed generation and management functionality for the ECE
 * (Entropic Collapse Engine) subsystem, providing mechanisms for creating and
 * managing high-quality entropy seeds for the collapse function.
 */

#include "ece_seed.h"
#include "ece_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>

/**
 * @brief Default seed size (32 bytes / 256 bits)
 */
#define ECE_SEED_DEFAULT_SIZE 32

/**
 * @brief Maximum seed size (128 bytes / 1024 bits)
 */
#define ECE_SEED_MAX_SIZE 128

/**
 * @brief Minimum seed size (16 bytes / 128 bits)
 */
#define ECE_SEED_MIN_SIZE 16

/**
 * @brief Default mixing rounds
 */
#define ECE_SEED_DEFAULT_MIXING_ROUNDS 16

/**
 * @brief System entropy source path
 */
#define ECE_SEED_SYSTEM_SOURCE "/dev/urandom"

/**
 * @brief Default persistence file path
 */
#define ECE_SEED_DEFAULT_PERSISTENCE_PATH ".ece_seed"

/**
 * @brief Internal seed context structure
 */
struct ece_seed {
    ece_seed_source_t source;      /**< Seed source type */
    uint8_t* seed;                 /**< Seed data */
    size_t seed_size;              /**< Seed size in bytes */
    uint32_t mixing_rounds;        /**< Number of mixing rounds */
    bool persistent;               /**< Persistence flag */
    char* custom_source;           /**< Custom source path */
    ece_seed_quality_t quality;    /**< Seed quality metrics */
    bool initialized;              /**< Initialization flag */
};

/**
 * @brief Default seed configuration
 */
static const ece_seed_config_t ECE_SEED_DEFAULT_CONFIG = {
    .source = ECE_SEED_SOURCE_MIXED,
    .seed_size = 0,  /* Use default */
    .mixing_rounds = 0,  /* Use default */
    .persistent = false,
    .custom_source = NULL
};

/**
 * @brief Check if RDRAND is available
 * 
 * @return bool True if available
 */
static bool ece_seed_check_rdrand(void) {
    // Check for RDRAND support using CPUID
    // This is a simplified check - in production, use proper CPUID intrinsics
    #if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    uint32_t eax, ebx, ecx, edx;
    eax = 1;
    
    #if defined(__GNUC__) || defined(__clang__)
    __asm__ __volatile__("cpuid"
                        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                        : "a"(eax));
    #else
    // Fallback for non-GCC/Clang compilers
    return false;
    #endif
    
    return (ecx & (1 << 30)) != 0;
    #else
    // Non-x86 platforms don't have RDRAND
    return false;
    #endif
}

/**
 * @brief Get RDRAND value
 * 
 * @param value Pointer to receive value
 * @return bool True if successful
 */
static bool ece_seed_get_rdrand(uint64_t* value) {
    #if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    uint64_t rand_val;
    unsigned char success;
    
    #if defined(__GNUC__) || defined(__clang__)
    // Try up to 10 times
    for (int i = 0; i < 10; i++) {
        #if defined(__x86_64__) || defined(_M_X64)
        __asm__ __volatile__("rdrand %0; setc %1"
                            : "=r"(rand_val), "=qm"(success));
        #else
        // 32-bit x86
        uint32_t lo, hi;
        __asm__ __volatile__("rdrand %0; setc %2"
                            : "=r"(lo), "=qm"(success));
        if (!success) continue;
        
        __asm__ __volatile__("rdrand %0; setc %2"
                            : "=r"(hi), "=qm"(success));
        if (!success) continue;
        
        rand_val = ((uint64_t)hi << 32) | lo;
        #endif
        
        if (success) {
            *value = rand_val;
            return true;
        }
    }
    #endif
    #endif
    
    return false;
}

/**
 * @brief Get high-resolution timestamp
 * 
 * @return uint64_t Timestamp in nanoseconds
 */
static uint64_t ece_seed_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * @brief Mix bytes using a simple non-cryptographic algorithm
 * 
 * @param seed Seed buffer
 * @param seed_size Seed size in bytes
 * @param data Input data
 * @param data_size Input data size in bytes
 * @param rounds Number of mixing rounds
 */
static void ece_seed_mix_bytes(uint8_t* seed, size_t seed_size, 
                             const uint8_t* data, size_t data_size,
                             uint32_t rounds) {
    if (!seed || seed_size == 0 || !data || data_size == 0 || rounds == 0) {
        return;
    }
    
    // Create a temporary buffer for mixing
    uint8_t* temp = (uint8_t*)malloc(seed_size);
    if (!temp) {
        return;
    }
    
    // Copy seed to temp buffer
    memcpy(temp, seed, seed_size);
    
    // Perform mixing rounds
    for (uint32_t round = 0; round < rounds; round++) {
        // Phase 1: Mix with input data
        for (size_t i = 0; i < seed_size; i++) {
            temp[i] ^= data[i % data_size];
            temp[i] = (temp[i] << 1) | (temp[i] >> 7); // Rotate left by 1
        }
        
        // Phase 2: Diffuse changes
        for (size_t i = 0; i < seed_size; i++) {
            size_t j = (i + round) % seed_size;
            temp[i] ^= temp[j];
            temp[i] = (temp[i] << 3) | (temp[i] >> 5); // Rotate left by 3
        }
        
        // Phase 3: Apply non-linear transformation
        for (size_t i = 0; i < seed_size; i++) {
            temp[i] ^= (temp[(i + 1) % seed_size] & temp[(i + 2) % seed_size]);
            temp[i] ^= (temp[(i + 3) % seed_size] | temp[(i + 4) % seed_size]);
        }
    }
    
    // Copy temp buffer back to seed
    memcpy(seed, temp, seed_size);
    
    // Clean up
    free(temp);
}

/**
 * @brief Calculate entropy estimation
 * 
 * @param data Input data
 * @param size Data size in bytes
 * @return double Estimated entropy bits per byte (0.0-8.0)
 */
static double ece_seed_calculate_entropy(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        return 0.0;
    }
    
    // Count byte frequencies
    uint32_t counts[256] = {0};
    for (size_t i = 0; i < size; i++) {
        counts[data[i]]++;
    }
    
    // Calculate Shannon entropy
    double entropy = 0.0;
    for (int i = 0; i < 256; i++) {
        if (counts[i] > 0) {
            double p = (double)counts[i] / size;
            entropy -= p * log2(p);
        }
    }
    
    return entropy;
}

/**
 * @brief Calculate uniformity score
 * 
 * @param data Input data
 * @param size Data size in bytes
 * @return double Uniformity score (0.0-1.0)
 */
static double ece_seed_calculate_uniformity(const uint8_t* data, size_t size) {
    if (!data || size == 0) {
        return 0.0;
    }
    
    // Count byte frequencies
    uint32_t counts[256] = {0};
    for (size_t i = 0; i < size; i++) {
        counts[data[i]]++;
    }
    
    // Calculate chi-square statistic
    double expected = (double)size / 256.0;
    double chi_square = 0.0;
    
    for (int i = 0; i < 256; i++) {
        double diff = counts[i] - expected;
        chi_square += (diff * diff) / expected;
    }
    
    // Convert chi-square to uniformity score (0.0-1.0)
    // Lower chi-square means better uniformity
    double max_chi_square = size; // Theoretical maximum for worst case
    double uniformity = 1.0 - (chi_square / max_chi_square);
    
    // Clamp to valid range
    if (uniformity < 0.0) uniformity = 0.0;
    if (uniformity > 1.0) uniformity = 1.0;
    
    return uniformity;
}

/**
 * @brief Calculate independence score
 * 
 * @param data Input data
 * @param size Data size in bytes
 * @return double Independence score (0.0-1.0)
 */
static double ece_seed_calculate_independence(const uint8_t* data, size_t size) {
    if (!data || size < 2) {
        return 0.0;
    }
    
    // Calculate autocorrelation for lag 1
    double sum = 0.0;
    double sum_sq = 0.0;
    double sum_lag = 0.0;
    
    for (size_t i = 0; i < size - 1; i++) {
        sum += data[i];
        sum_sq += data[i] * data[i];
        sum_lag += data[i] * data[i+1];
    }
    
    sum += data[size-1];
    sum_sq += data[size-1] * data[size-1];
    
    double mean = sum / size;
    double variance = (sum_sq / size) - (mean * mean);
    
    if (variance <= 0.0) {
        return 0.0; // No variance means no independence
    }
    
    double autocorr = (sum_lag / (size - 1) - mean * mean) / variance;
    
    // Convert autocorrelation to independence score (0.0-1.0)
    // Lower absolute autocorrelation means better independence
    double independence = 1.0 - fabs(autocorr);
    
    // Clamp to valid range
    if (independence < 0.0) independence = 0.0;
    if (independence > 1.0) independence = 1.0;
    
    return independence;
}

/**
 * @brief Generate seed from system entropy source
 * 
 * @param handle Seed handle
 * @return ece_status_t Status code
 */
static ece_status_t ece_seed_generate_system(ece_seed_handle_t handle) {
    if (!handle || !handle->seed) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Open system entropy source
    int fd = open(ECE_SEED_SYSTEM_SOURCE, O_RDONLY);
    if (fd < 0) {
        return ECE_STATUS_ERROR;
    }
    
    // Read entropy
    ssize_t bytes_read = read(fd, handle->seed, handle->seed_size);
    close(fd);
    
    if (bytes_read != (ssize_t)handle->seed_size) {
        return ECE_STATUS_ERROR;
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Generate seed from hardware entropy source
 * 
 * @param handle Seed handle
 * @return ece_status_t Status code
 */
static ece_status_t ece_seed_generate_hardware(ece_seed_handle_t handle) {
    if (!handle || !handle->seed) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Check if RDRAND is available
    if (!ece_seed_check_rdrand()) {
        return ECE_STATUS_NO_ENTROPY;
    }
    
    // Generate seed using RDRAND
    size_t bytes_generated = 0;
    while (bytes_generated < handle->seed_size) {
        uint64_t value;
        if (!ece_seed_get_rdrand(&value)) {
            return ECE_STATUS_ERROR;
        }
        
        // Copy bytes to seed buffer
        size_t bytes_to_copy = handle->seed_size - bytes_generated;
        if (bytes_to_copy > sizeof(value)) {
            bytes_to_copy = sizeof(value);
        }
        
        memcpy(handle->seed + bytes_generated, &value, bytes_to_copy);
        bytes_generated += bytes_to_copy;
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Generate seed from timer jitter
 * 
 * @param handle Seed handle
 * @return ece_status_t Status code
 */
static ece_status_t ece_seed_generate_timer(ece_seed_handle_t handle) {
    if (!handle || !handle->seed) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Initialize seed with zeros
    memset(handle->seed, 0, handle->seed_size);
    
    // Collect entropy from timer jitter
    for (size_t i = 0; i < handle->seed_size * 8; i++) {
        // Get high-resolution timestamp
        uint64_t timestamp = ece_seed_get_timestamp();
        
        // Extract entropy from timestamp
        uint8_t entropy_byte = (timestamp & 0xFF) ^ 
                              ((timestamp >> 8) & 0xFF) ^ 
                              ((timestamp >> 16) & 0xFF) ^ 
                              ((timestamp >> 24) & 0xFF) ^
                              ((timestamp >> 32) & 0xFF) ^ 
                              ((timestamp >> 40) & 0xFF) ^
                              ((timestamp >> 48) & 0xFF) ^ 
                              ((timestamp >> 56) & 0xFF);
        
        // Mix into seed
        handle->seed[i % handle->seed_size] ^= entropy_byte;
        
        // Introduce small delay to ensure timestamp difference
        for (volatile int j = 0; j < 1000; j++) {
            // Busy-wait to create jitter
        }
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Generate seed from custom entropy source
 * 
 * @param handle Seed handle
 * @return ece_status_t Status code
 */
static ece_status_t ece_seed_generate_custom(ece_seed_handle_t handle) {
    if (!handle || !handle->seed || !handle->custom_source) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Open custom entropy source
    int fd = open(handle->custom_source, O_RDONLY);
    if (fd < 0) {
        return ECE_STATUS_ERROR;
    }
    
    // Read entropy
    ssize_t bytes_read = read(fd, handle->seed, handle->seed_size);
    close(fd);
    
    if (bytes_read != (ssize_t)handle->seed_size) {
        return ECE_STATUS_ERROR;
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Generate seed from mixed entropy sources
 * 
 * @param handle Seed handle
 * @return ece_status_t Status code
 */
static ece_status_t ece_seed_generate_mixed(ece_seed_handle_t handle) {
    if (!handle || !handle->seed) {
        return ECE_STATUS_INVALID_ARG;
    }
    
    // Initialize seed with zeros
    memset(handle->seed, 0, handle->seed_size);
    
    // Try hardware entropy first
    uint8_t* hardware_seed = (uint8_t*)malloc(handle->seed_size);
    if (hardware_seed) {
        if (ece_seed_generate_hardware(handle) == ECE_STATUS_OK) {
            memcpy(hardware_seed, handle->seed, handle->seed_size);
        } else {
            // If hardware entropy fails, use zeros
            memset(hardware_seed, 0, handle->seed_size);
        }
    }
    
    // Try system entropy
    if (ece_seed_generate_system(handle) != ECE_STATUS_OK) {
        // If system entropy fails, use timer entropy
        if (ece_seed_generate_timer(handle) != ECE_STATUS_OK) {
            // If all sources fail, return error
            if (hardware_seed) {
                free(hardware_seed);
            }
            return ECE_STATUS_NO_ENTROPY;
        }
    }
    
    // Mix in hardware entropy if available
    if (hardware_seed) {
        ece_seed_mix_bytes(handle->seed, handle->seed_size, 
                          hardware_seed, handle->seed_size, 
                          handle->mixing_rounds);
        free(hardware_seed);
    }
    
    // Mix in additional entropy from timer
    for (int i = 0; i < 16; i++) {
        uint64_t timestamp = ece_seed_get_timestamp();
        ece_seed_mix_bytes(handle->seed, handle->seed_size, 
                          (uint8_t*)&timestamp, sizeof(timestamp), 
                          handle->mixing_rounds / 4);
        
        // Introduce small delay
        usleep(1000);
    }
    
    return ECE_STATUS_OK;
}

/**
 * @brief Initialize a seed context
 * 
 * @param config Configuration structure (NULL for defaults)
 * @return ece_seed_handle_t Seed handle or NULL on failure
 */
ece_seed_handle_t ece_seed_init(const ece_seed_config_t* config) {
    // Use default config if none provided
    ece_seed_config_t effective_config;
    if (config) {
        effective_config = *config;
    } else {
        effective_config = ECE_SEED_DEFAULT_CONFIG;
    }
    
    // Validate and adjust seed size
    if (effective_config.seed_size == 0) {
        effective_config.seed_size = ECE_SEED_DEFAULT_SIZE;
    } else if (effective_config.seed_size < ECE_SEED_MIN_SIZE) {
        effective_config.seed_size = ECE_SEED_MIN_SIZE;
    } else if (effective_config.seed_size > ECE_SEED_MAX_SIZE) {
        effective_config.seed_size = ECE_SEED_MAX_SIZE;
    }
    
    // Validate and adjust mixing rounds
    if (effective_config.mixing_rounds == 0) {
        effective_config.mixing_rounds = ECE_SEED_DEFAULT_MIXING_ROUNDS;
    }
    
    // Allocate seed context
    struct ece_seed* handle = (struct ece_seed*)calloc(1, sizeof(struct ece_seed));
    if (!handle) {
        return NULL;
    }
    
    // Allocate seed buffer
    handle->seed = (uint8_t*)malloc(effective_config.seed_size);
    if (!handle->seed) {
        free(handle);
        return NULL;
    }
    
    // Copy custom source if provided
    if (effective_config.custom_source && effective_config.so
(Content truncated due to size limit. Use line ranges to read in chunks)