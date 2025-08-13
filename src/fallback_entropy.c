/**
 * @file fallback_entropy.c
 * @brief RDRAND/CPU jitter backup seeder
 * 
 * This module implements a fallback entropy source that can be activated
 * when the primary entropy sources are degraded or unavailable. It uses
 * CPU jitter and hardware RNG instructions (when available) to generate
 * entropy for the system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

// Forward declarations for external functions
extern int rng_linux_init(int source);
extern int rng_linux_get_bytes(uint8_t* buffer, size_t size);
extern bool rng_linux_is_ready(void);
extern const char* rng_linux_get_source_name(void);

extern int entropy_bus_register_source(const char* name, double initial_quality);
extern int entropy_bus_push(int source_id, const uint8_t* data, size_t size, double quality);

// Fallback entropy state
typedef struct {
    bool active;
    bool initialized;
    pthread_t thread_id;
    int source_id;
    uint64_t bytes_generated;
} fallback_entropy_state_t;

// Global fallback entropy state
static fallback_entropy_state_t g_fallback = {
    .active = false,
    .initialized = false,
    .source_id = -1,
    .bytes_generated = 0
};

/**
 * @brief Generate entropy from CPU jitter
 * 
 * This function generates entropy by measuring timing variations
 * in CPU operations, which can be a source of true randomness.
 * 
 * @param buffer Buffer to store entropy
 * @param size Size of buffer in bytes
 * @return int Number of bytes generated, negative on failure
 */
static int generate_jitter_entropy(uint8_t* buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return -1;
    }
    
    // Clear buffer
    memset(buffer, 0, size);
    
    // Use high-resolution timer for jitter measurement
    struct timespec start, end;
    uint64_t delta;
    
    for (size_t i = 0; i < size; i++) {
        uint8_t entropy_byte = 0;
        
        // Generate 8 bits of entropy
        for (int bit = 0; bit < 8; bit++) {
            // Perform some CPU-intensive operations
            volatile int sum = 0;
            for (volatile int j = 0; j < 100; j++) {
                sum += j * j;
            }
            
            // Measure time before and after a short sleep
            clock_gettime(CLOCK_MONOTONIC, &start);
            usleep(1); // Very short sleep
            clock_gettime(CLOCK_MONOTONIC, &end);
            
            // Calculate time delta in nanoseconds
            delta = (end.tv_sec - start.tv_sec) * 1000000000ULL + 
                   (end.tv_nsec - start.tv_nsec);
            
            // Extract least significant bit of timing measurement
            entropy_byte |= ((delta & 1) << bit);
        }
        
        buffer[i] = entropy_byte;
    }
    
    return size;
}

/**
 * @brief Fallback entropy generation thread
 * 
 * @param arg Thread argument (unused)
 * @return void* Return value (unused)
 */
static void* fallback_entropy_thread(void* arg) {
    (void)arg; // Unused
    
    const size_t BUFFER_SIZE = 256;
    uint8_t buffer[BUFFER_SIZE];
    
    while (g_fallback.active) {
        // Try to get entropy from hardware RNG first
        int bytes = 0;
        if (rng_linux_is_ready()) {
            bytes = rng_linux_get_bytes(buffer, BUFFER_SIZE);
        }
        
        // If hardware RNG failed or returned fewer bytes than requested,
        // supplement with jitter entropy
        if (bytes < BUFFER_SIZE) {
            int jitter_bytes = generate_jitter_entropy(buffer + bytes, BUFFER_SIZE - bytes);
            if (jitter_bytes > 0) {
                bytes += jitter_bytes;
            }
        }
        
        // Push entropy to the bus if we got any
        if (bytes > 0 && g_fallback.source_id >= 0) {
            entropy_bus_push(g_fallback.source_id, buffer, bytes, 0.8);
            g_fallback.bytes_generated += bytes;
        }
        
        // Sleep for a short time
        usleep(100000); // 100ms
    }
    
    return NULL;
}

/**
 * @brief Initialize the fallback entropy source
 * 
 * @return int 0 on success, non-zero on failure
 */
int fallback_entropy_init(void) {
    if (g_fallback.initialized) {
        return 0; // Already initialized
    }
    
    // Initialize hardware RNG
    if (rng_linux_init(0) != 0) {
        fprintf(stderr, "Warning: Failed to initialize hardware RNG\n");
        // Non-fatal, we can still use jitter entropy
    }
    
    // Register as entropy source
    g_fallback.source_id = entropy_bus_register_source("fallback", 0.8);
    if (g_fallback.source_id < 0) {
        fprintf(stderr, "Failed to register fallback entropy source\n");
        return -1;
    }
    
    g_fallback.initialized = true;
    return 0;
}

/**
 * @brief Activate the fallback entropy source
 * 
 * @return int 0 on success, non-zero on failure
 */
int fallback_entropy_activate(void) {
    if (!g_fallback.initialized) {
        if (fallback_entropy_init() != 0) {
            return -1;
        }
    }
    
    if (g_fallback.active) {
        return 0; // Already active
    }
    
    g_fallback.active = true;
    
    // Start entropy generation thread
    if (pthread_create(&g_fallback.thread_id, NULL, fallback_entropy_thread, NULL) != 0) {
        g_fallback.active = false;
        return -1;
    }
    
    return 0;
}

/**
 * @brief Deactivate the fallback entropy source
 * 
 * @return int 0 on success, non-zero on failure
 */
int fallback_entropy_deactivate(void) {
    if (!g_fallback.active) {
        return 0; // Already inactive
    }
    
    g_fallback.active = false;
    
    // Wait for thread to exit
    pthread_join(g_fallback.thread_id, NULL);
    
    return 0;
}

/**
 * @brief Check if the fallback entropy source is active
 * 
 * @return bool true if active, false otherwise
 */
bool fallback_entropy_is_active(void) {
    return g_fallback.active;
}

/**
 * @brief Get the number of bytes generated by the fallback source
 * 
 * @return uint64_t Number of bytes generated
 */
uint64_t fallback_entropy_get_bytes_generated(void) {
    return g_fallback.bytes_generated;
}

/**
 * @brief Generate entropy on demand
 * 
 * This function generates entropy immediately and returns it,
 * without pushing it to the entropy bus.
 * 
 * @param buffer Buffer to store entropy
 * @param size Size of buffer in bytes
 * @return int Number of bytes generated, negative on failure
 */
int fallback_entropy_generate(uint8_t* buffer, size_t size) {
    if (!g_fallback.initialized) {
        if (fallback_entropy_init() != 0) {
            return -1;
        }
    }
    
    // Try to get entropy from hardware RNG first
    int bytes = 0;
    if (rng_linux_is_ready()) {
        bytes = rng_linux_get_bytes(buffer, size);
    }
    
    // If hardware RNG failed or returned fewer bytes than requested,
    // supplement with jitter entropy
    if (bytes < (int)size) {
        int jitter_bytes = generate_jitter_entropy(buffer + bytes, size - bytes);
        if (jitter_bytes > 0) {
            bytes += jitter_bytes;
        }
    }
    
    return bytes;
}
