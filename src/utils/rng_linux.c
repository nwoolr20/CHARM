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
 * @file rng_linux.c
 * @brief Linux-specific random number generator driver
 * 
 * This module implements the Linux-specific random number generator driver
 * for accessing hardware random number generators and system entropy sources
 * like /dev/urandom and RDRAND CPU instructions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/random.h>
#include <cpuid.h>
#include <x86intrin.h>

// RNG source types
typedef enum {
    RNG_SOURCE_URANDOM,    // /dev/urandom
    RNG_SOURCE_RANDOM,     // /dev/random
    RNG_SOURCE_RDRAND,     // RDRAND CPU instruction
    RNG_SOURCE_RDSEED,     // RDSEED CPU instruction
    RNG_SOURCE_HWRNG       // Hardware RNG device
} rng_source_t;

// RNG state
typedef struct {
    rng_source_t source;
    int fd;                // File descriptor for /dev/urandom or /dev/random
    bool has_rdrand;       // Whether RDRAND is available
    bool has_rdseed;       // Whether RDSEED is available
    bool initialized;
} rng_state_t;

// Global RNG state
static rng_state_t g_rng = {
    .source = RNG_SOURCE_URANDOM,
    .fd = -1,
    .has_rdrand = false,
    .has_rdseed = false,
    .initialized = false
};

/**
 * @brief Check if CPU supports RDRAND/RDSEED
 */
static void check_cpu_features(void) {
    unsigned int eax, ebx, ecx, edx;
    
    // Check RDRAND (bit 30 of ECX in leaf 1)
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        g_rng.has_rdrand = (ecx & (1 << 30)) != 0;
    }
    
    // Check RDSEED (bit 18 of EBX in leaf 7)
    if (__get_cpuid(7, &eax, &ebx, &ecx, &edx)) {
        g_rng.has_rdseed = (ebx & (1 << 18)) != 0;
    }
}

/**
 * @brief Initialize the RNG driver
 * 
 * @param source RNG source to use
 * @return int 0 on success, non-zero on failure
 */
int rng_linux_init(rng_source_t source) {
    if (g_rng.initialized) {
        // Already initialized, close existing resources
        if (g_rng.fd >= 0) {
            close(g_rng.fd);
            g_rng.fd = -1;
        }
    }
    
    // Check CPU features
    check_cpu_features();
    
    // Set source
    g_rng.source = source;
    
    // Initialize based on source
    switch (source) {
        case RNG_SOURCE_URANDOM:
            g_rng.fd = open("/dev/urandom", O_RDONLY);
            if (g_rng.fd < 0) {
                return -1;
            }
            break;
            
        case RNG_SOURCE_RANDOM:
            g_rng.fd = open("/dev/random", O_RDONLY);
            if (g_rng.fd < 0) {
                return -1;
            }
            break;
            
        case RNG_SOURCE_RDRAND:
            if (!g_rng.has_rdrand) {
                return -1; // RDRAND not supported
            }
            break;
            
        case RNG_SOURCE_RDSEED:
            if (!g_rng.has_rdseed) {
                return -1; // RDSEED not supported
            }
            break;
            
        case RNG_SOURCE_HWRNG:
            g_rng.fd = open("/dev/hwrng", O_RDONLY);
            if (g_rng.fd < 0) {
                return -1;
            }
            break;
    }
    
    g_rng.initialized = true;
    return 0;
}

/**
 * @brief Clean up the RNG driver
 */
void rng_linux_cleanup(void) {
    if (!g_rng.initialized) {
        return;
    }
    
    if (g_rng.fd >= 0) {
        close(g_rng.fd);
        g_rng.fd = -1;
    }
    
    g_rng.initialized = false;
}

/**
 * @brief Get random bytes from the RNG
 * 
 * @param buffer Buffer to store random bytes
 * @param size Number of bytes to generate
 * @return int Number of bytes generated, negative on failure
 */
int rng_linux_get_bytes(uint8_t* buffer, size_t size) {
    if (!g_rng.initialized || buffer == NULL || size == 0) {
        return -1;
    }
    
    switch (g_rng.source) {
        case RNG_SOURCE_URANDOM:
        case RNG_SOURCE_RANDOM:
        case RNG_SOURCE_HWRNG:
            if (g_rng.fd < 0) {
                return -1;
            }
            return read(g_rng.fd, buffer, size);
            
        case RNG_SOURCE_RDRAND:
            if (!g_rng.has_rdrand) {
                return -1;
            }
            {
                size_t bytes_generated = 0;
                unsigned int rand_val;
                
                while (bytes_generated < size) {
                    // Try to get a random value (retry up to 10 times if it fails)
                    int success = 0;
                    for (int i = 0; i < 10; i++) {
                        success = _rdrand32_step(&rand_val);
                        if (success) break;
                    }
                    
                    if (!success) {
                        return bytes_generated > 0 ? bytes_generated : -1;
                    }
                    
                    // Copy as many bytes as we can
                    size_t bytes_to_copy = (size - bytes_generated < sizeof(rand_val)) ? 
                                          (size - bytes_generated) : sizeof(rand_val);
                    memcpy(buffer + bytes_generated, &rand_val, bytes_to_copy);
                    bytes_generated += bytes_to_copy;
                }
                
                return bytes_generated;
            }
            
        case RNG_SOURCE_RDSEED:
            if (!g_rng.has_rdseed) {
                return -1;
            }
            {
                size_t bytes_generated = 0;
                unsigned int seed_val;
                
                while (bytes_generated < size) {
                    // Try to get a seed value (retry up to 100 times if it fails)
                    int success = 0;
                    for (int i = 0; i < 100; i++) {
                        success = _rdseed32_step(&seed_val);
                        if (success) break;
                    }
                    
                    if (!success) {
                        return bytes_generated > 0 ? bytes_generated : -1;
                    }
                    
                    // Copy as many bytes as we can
                    size_t bytes_to_copy = (size - bytes_generated < sizeof(seed_val)) ? 
                                          (size - bytes_generated) : sizeof(seed_val);
                    memcpy(buffer + bytes_generated, &seed_val, bytes_to_copy);
                    bytes_generated += bytes_to_copy;
                }
                
                return bytes_generated;
            }
    }
    
    return -1;
}

/**
 * @brief Get entropy estimate from the RNG
 * 
 * @return int Entropy estimate in bits, negative on failure
 */
int rng_linux_get_entropy(void) {
    if (!g_rng.initialized) {
        return -1;
    }
    
    switch (g_rng.source) {
        case RNG_SOURCE_URANDOM:
        case RNG_SOURCE_RANDOM:
            if (g_rng.fd < 0) {
                return -1;
            }
            {
                int entropy = 0;
                if (ioctl(g_rng.fd, RNDGETENTCNT, &entropy) < 0) {
                    return -1;
                }
                return entropy;
            }
            
        case RNG_SOURCE_RDRAND:
            // RDRAND is considered to have full entropy
            return 32;
            
        case RNG_SOURCE_RDSEED:
            // RDSEED is considered to have full entropy
            return 32;
            
        case RNG_SOURCE_HWRNG:
            // Hardware RNG is assumed to have good entropy
            return 32;
    }
    
    return -1;
}

/**
 * @brief Check if the RNG is ready
 * 
 * @return bool true if ready, false otherwise
 */
bool rng_linux_is_ready(void) {
    if (!g_rng.initialized) {
        return false;
    }
    
    switch (g_rng.source) {
        case RNG_SOURCE_URANDOM:
            // /dev/urandom is always ready
            return g_rng.fd >= 0;
            
        case RNG_SOURCE_RANDOM:
            if (g_rng.fd < 0) {
                return false;
            }
            {
                // Check if there's at least 64 bits of entropy
                int entropy = rng_linux_get_entropy();
                return entropy >= 64;
            }
            
        case RNG_SOURCE_RDRAND:
            return g_rng.has_rdrand;
            
        case RNG_SOURCE_RDSEED:
            return g_rng.has_rdseed;
            
        case RNG_SOURCE_HWRNG:
            return g_rng.fd >= 0;
    }
    
    return false;
}

/**
 * @brief Get the name of the current RNG source
 * 
 * @return const char* Source name
 */
const char* rng_linux_get_source_name(void) {
    switch (g_rng.source) {
        case RNG_SOURCE_URANDOM:
            return "urandom";
        case RNG_SOURCE_RANDOM:
            return "random";
        case RNG_SOURCE_RDRAND:
            return "rdrand";
        case RNG_SOURCE_RDSEED:
            return "rdseed";
        case RNG_SOURCE_HWRNG:
            return "hwrng";
        default:
            return "unknown";
    }
}
