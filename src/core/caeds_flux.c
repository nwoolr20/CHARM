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


// SPDX-License-Identifier: LicenseRef-CHARM-2025
// SPDX-FileCopyrightText: Copyright (c) 2025 Nicholas Woolridge & NOCTRL™ (Nô)

/**
 * @file caeds_flux.c
 * @brief Implementation of the CAEDS entropy flux acquisition system
 * 
 * This file implements the entropy flux acquisition system for the CAEDS
 * (Charm Advanced Entropy Diagnostic System) subsystem, providing mechanisms
 * to collect high-quality entropy from multiple sources.
 */

/* Define _GNU_SOURCE for M_PI, M_E and other extensions */
#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "caeds_flux.h"
#include "charm_status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Define math constants if not available */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.7182818284590452354
#endif

/**
 * @brief Maximum buffer size for batch operations
 */
#define CAEDS_FLUX_MAX_BUFFER_SIZE 4096

/**
 * @brief Default sampling window size for volatility metrics
 */
#define CAEDS_FLUX_DEFAULT_WINDOW_SIZE 256

/**
 * @brief Number of iterations for CPU latency measurement
 */
#define CAEDS_FLUX_CPU_ITERATIONS 1000

/**
 * @brief Path to thermal information in sysfs
 */
#define CAEDS_FLUX_THERMAL_PATH "/sys/class/thermal/thermal_zone0/temp"

/**
 * @brief Path to urandom device
 */
#define CAEDS_FLUX_URANDOM_PATH "/dev/urandom"

/**
 * @brief Internal source state
 */
typedef struct {
    caeds_flux_source_config_t config;
    bool available;
    caeds_flux_volatility_t volatility;
    uint8_t* window;
    size_t window_size;
    size_t window_index;
    pthread_mutex_t mutex;
} caeds_flux_source_state_t;

/**
 * @brief Internal context structure
 */
typedef struct caeds_flux_context_s {
    caeds_flux_source_state_t sources[CAEDS_FLUX_SOURCE_COUNT];
    uint32_t buffer_size;
    bool non_blocking;
    bool fingerprint_rdrand;
    uint32_t min_entropy_bits;
    int urandom_fd;
    bool rdrand_available;
} caeds_flux_context_s;

/**
 * @brief Default configuration
 */
const caeds_flux_config_t CAEDS_FLUX_DEFAULT_CONFIG = {
    .sources = {
        [CAEDS_FLUX_SOURCE_TIMESTAMP_JITTER] = {
            .enabled = true,
            .weight = 30,
            .batch_size = 64,
            .sampling_interval = 1000
        },
        [CAEDS_FLUX_SOURCE_CPU_LATENCY] = {
            .enabled = true,
            .weight = 25,
            .batch_size = 64,
            .sampling_interval = 2000
        },
        [CAEDS_FLUX_SOURCE_THERMAL] = {
            .enabled = true,
            .weight = 10,
            .batch_size = 32,
            .sampling_interval = 5000
        },
        [CAEDS_FLUX_SOURCE_NETWORK] = {
            .enabled = true,
            .weight = 15,
            .batch_size = 64,
            .sampling_interval = 3000
        },
        [CAEDS_FLUX_SOURCE_URANDOM] = {
            .enabled = true,
            .weight = 10,
            .batch_size = 128,
            .sampling_interval = 1000
        },
        [CAEDS_FLUX_SOURCE_RDRAND] = {
            .enabled = true,
            .weight = 10,
            .batch_size = 128,
            .sampling_interval = 1000
        }
    },
    .buffer_size = 1024,
    .non_blocking = true,
    .fingerprint_rdrand = true,
    .min_entropy_bits = 6
};

/**
 * @brief Source names
 */
static const char* caeds_flux_source_names[CAEDS_FLUX_SOURCE_COUNT] = {
    "Timestamp Jitter",
    "CPU Latency",
    "Thermal Variance",
    "Network Jitter",
    "System Entropy",
    "RDRAND"
};

/**
 * @brief Check if RDRAND is available
 * 
 * @return bool True if available
 */
static bool caeds_flux_check_rdrand(void) {
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
static bool caeds_flux_get_rdrand(uint64_t* value) {
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
static uint64_t caeds_flux_get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * @brief Update volatility metrics
 * 
 * @param state Source state
 * @param value New value
 */
static void caeds_flux_update_volatility(caeds_flux_source_state_t* state, uint8_t value) {
    pthread_mutex_lock(&state->mutex);
    
    // Add to window
    if (state->window != NULL) {
        state->window[state->window_index] = value;
        state->window_index = (state->window_index + 1) % state->window_size;
    }
    
    // Update metrics
    if (state->volatility.sample_count == 0) {
        state->volatility.min = value;
        state->volatility.max = value;
        state->volatility.mean = value;
        state->volatility.variance = 0;
    } else {
        // Update min/max
        if (value < state->volatility.min) {
            state->volatility.min = value;
        }
        if (value > state->volatility.max) {
            state->volatility.max = value;
        }
        
        // Update mean and variance using Welford's online algorithm
        double old_mean = state->volatility.mean;
        double delta = value - old_mean;
        state->volatility.mean += delta / (state->volatility.sample_count + 1);
        double delta2 = value - state->volatility.mean;
        state->volatility.variance += delta * delta2;
    }
    
    state->volatility.sample_count++;
    
    // Estimate entropy bits per byte
    if (state->volatility.sample_count > 1) {
        double normalized_variance = state->volatility.variance / state->volatility.sample_count;
        double range = state->volatility.max - state->volatility.min + 1;
        
        // Shannon entropy estimate based on variance
        // This is a simplified model - in production, use more sophisticated entropy estimation
        if (normalized_variance > 0 && range > 1) {
            double entropy_estimate = log2(sqrt(2 * M_PI * M_E * normalized_variance));
            if (entropy_estimate > 8.0) entropy_estimate = 8.0;
            if (entropy_estimate < 0.0) entropy_estimate = 0.0;
            state->volatility.entropy_bits = entropy_estimate;
        } else {
            state->volatility.entropy_bits = 0.0;
        }
    }
    
    pthread_mutex_unlock(&state->mutex);
}

/**
 * @brief Collect entropy from timestamp jitter
 * 
 * @param context Context handle
 * @param buffer Buffer to receive entropy
 * @param size Buffer size
 * @param collected Pointer to receive number of bytes collected
 * @return charm_status_t Status code
 */
static charm_status_t caeds_flux_collect_timestamp_jitter(caeds_flux_context_t context,
                                                        uint8_t* buffer,
                                                        size_t size,
                                                        size_t* collected) {
    if (!context || !buffer || !collected) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_flux_source_state_t* state = &context->sources[CAEDS_FLUX_SOURCE_TIMESTAMP_JITTER];
    if (!state->available || !state->config.enabled) {
        return CHARM_STATUS_ERROR_NOT_SUPPORTED;
    }
    
    size_t bytes_collected = 0;
    uint64_t last_timestamp = 0;
    
    while (bytes_collected < size) {
        // Get high-resolution timestamp
        uint64_t timestamp = caeds_flux_get_timestamp();
        
        // Calculate jitter
        if (last_timestamp > 0) {
            uint64_t jitter = timestamp - last_timestamp;
            
            // Extract entropy from jitter
            uint8_t entropy_byte = (jitter & 0xFF) ^ ((jitter >> 8) & 0xFF) ^ 
                                  ((jitter >> 16) & 0xFF) ^ ((jitter >> 24) & 0xFF) ^
                                  ((jitter >> 32) & 0xFF) ^ ((jitter >> 40) & 0xFF) ^
                                  ((jitter >> 48) & 0xFF) ^ ((jitter >> 56) & 0xFF);
            
            buffer[bytes_collected++] = entropy_byte;
            
            // Update volatility metrics
            caeds_flux_update_volatility(state, entropy_byte);
        }
        
        last_timestamp = timestamp;
        
        // Introduce small delay to ensure timestamp difference
        usleep(1);
        
        // Check if we should stop (non-blocking mode)
        if (context->non_blocking && bytes_collected > 0) {
            break;
        }
    }
    
    *collected = bytes_collected;
    return CHARM_STATUS_SUCCESS;
}

/**
 * @brief Collect entropy from CPU latency drift
 * 
 * @param context Context handle
 * @param buffer Buffer to receive entropy
 * @param size Buffer size
 * @param collected Pointer to receive number of bytes collected
 * @return charm_status_t Status code
 */
static charm_status_t caeds_flux_collect_cpu_latency(caeds_flux_context_t context,
                                                   uint8_t* buffer,
                                                   size_t size,
                                                   size_t* collected) {
    if (!context || !buffer || !collected) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_flux_source_state_t* state = &context->sources[CAEDS_FLUX_SOURCE_CPU_LATENCY];
    if (!state->available || !state->config.enabled) {
        return CHARM_STATUS_ERROR_NOT_SUPPORTED;
    }
    
    size_t bytes_collected = 0;
    
    while (bytes_collected < size) {
        // Measure CPU loop latency
        uint64_t start_time = caeds_flux_get_timestamp();
        
        // Variable-length instruction execution
        volatile uint64_t counter = 0;
        for (int i = 0; i < CAEDS_FLUX_CPU_ITERATIONS; i++) {
            counter += i * i;
        }
        
        uint64_t end_time = caeds_flux_get_timestamp();
        uint64_t latency = end_time - start_time;
        
        // Extract entropy from latency
        uint8_t entropy_byte = (latency & 0xFF) ^ ((latency >> 8) & 0xFF) ^ 
                              ((latency >> 16) & 0xFF) ^ ((latency >> 24) & 0xFF) ^
                              ((counter & 0xFF) ^ ((counter >> 8) & 0xFF));
        
        buffer[bytes_collected++] = entropy_byte;
        
        // Update volatility metrics
        caeds_flux_update_volatility(state, entropy_byte);
        
        // Check if we should stop (non-blocking mode)
        if (context->non_blocking && bytes_collected > 0) {
            break;
        }
    }
    
    *collected = bytes_collected;
    return CHARM_STATUS_SUCCESS;
}

/**
 * @brief Collect entropy from thermal variance
 * 
 * @param context Context handle
 * @param buffer Buffer to receive entropy
 * @param size Buffer size
 * @param collected Pointer to receive number of bytes collected
 * @return charm_status_t Status code
 */
static charm_status_t caeds_flux_collect_thermal(caeds_flux_context_t context,
                                               uint8_t* buffer,
                                               size_t size,
                                               size_t* collected) {
    if (!context || !buffer || !collected) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_flux_source_state_t* state = &context->sources[CAEDS_FLUX_SOURCE_THERMAL];
    if (!state->available || !state->config.enabled) {
        return CHARM_STATUS_ERROR_NOT_SUPPORTED;
    }
    
    size_t bytes_collected = 0;
    
    while (bytes_collected < size) {
        // Read thermal information
        FILE* thermal_file = fopen(CAEDS_FLUX_THERMAL_PATH, "r");
        if (!thermal_file) {
            // Try alternative thermal zones if the first one fails
            for (int i = 1; i < 10; i++) {
                char path[128];
                snprintf(path, sizeof(path), "/sys/class/thermal/thermal_zone%d/temp", i);
                thermal_file = fopen(path, "r");
                if (thermal_file) break;
            }
            
            if (!thermal_file) {
                state->available = false;
                return CHARM_STATUS_ERROR_NOT_SUPPORTED;
            }
        }
        
        int temp;
        if (fscanf(thermal_file, "%d", &temp) != 1) {
            fclose(thermal_file);
            return CHARM_STATUS_ERROR_IO;
        }
        
        fclose(thermal_file);
        
        // Get timestamp for additional entropy
        uint64_t timestamp = caeds_flux_get_timestamp();
        
        // Combine thermal reading with timestamp
        uint8_t entropy_byte = (temp & 0xFF) ^ ((temp >> 8) & 0xFF) ^ 
                              (timestamp & 0xFF) ^ ((timestamp >> 8) & 0xFF);
        
        buffer[bytes_collected++] = entropy_byte;
        
        // Update volatility metrics
        caeds_flux_update_volatility(state, entropy_byte);
        
        // Thermal readings don't change rapidly, so add a delay
        usleep(5000);
        
        // Check if we should stop (non-blocking mode)
        if (context->non_blocking && bytes_collected > 0) {
            break;
        }
    }
    
    *collected = bytes_collected;
    return CHARM_STATUS_SUCCESS;
}

/**
 * @brief Collect entropy from network jitter
 * 
 * @param context Context handle
 * @param buffer Buffer to receive entropy
 * @param size Buffer size
 * @param collected Pointer to receive number of bytes collected
 * @return charm_status_t Status code
 */
static charm_status_t caeds_flux_collect_network(caeds_flux_context_t context,
                                               uint8_t* buffer,
                                               size_t size,
                                               size_t* collected) {
    if (!context || !buffer || !collected) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_flux_source_state_t* state = &context->sources[CAEDS_FLUX_SOURCE_NETWORK];
    if (!state->available || !state->config.enabled) {
        return CHARM_STATUS_ERROR_NOT_SUPPORTED;
    }
    
    size_t bytes_collected = 0;
    
    // Create a socket for loopback ping
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        state->available = false;
        return CHARM_STATUS_ERROR_NOT_SUPPORTED;
    }
    
    // Set non-blocking mode
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    close(sock);
    state->available = true;
    return CHARM_SUCCESS;
}