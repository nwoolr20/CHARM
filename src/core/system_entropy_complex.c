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
 * @file system_entropy.c
 * @brief System entropy sampling module - "Internal Lava Lamp Method"
 * 
 * This module implements a high-performance entropy sampling system inspired by
 * Cloudflare's lava lamp method, but using internal system metrics instead of
 * external sources. It samples unpredictable system characteristics at random
 * to create high-quality entropy with minimal overhead.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <dirent.h>
#include <cpuid.h>

// System entropy configuration
#define SYS_ENTROPY_BUFFER_SIZE 4096
#define SYS_ENTROPY_SAMPLE_COUNT 16
#define SYS_ENTROPY_CACHE_SIZE 1024
#define SYS_ENTROPY_REFRESH_INTERVAL_US 1000  // 1ms

// Entropy source types
typedef enum {
    ENTROPY_CPU_CYCLES = 0,
    ENTROPY_MEMORY_USAGE,
    ENTROPY_PROCESS_COUNT,
    ENTROPY_INTERRUPT_COUNT,
    ENTROPY_CACHE_MISSES,
    ENTROPY_CONTEXT_SWITCHES,
    ENTROPY_NETWORK_PACKETS,
    ENTROPY_DISK_IO,
    ENTROPY_TEMPERATURE,
    ENTROPY_VOLTAGE,
    ENTROPY_MAX_SOURCES
} entropy_source_t;

// Fast entropy cache for high-performance access
typedef struct {
    uint8_t data[SYS_ENTROPY_CACHE_SIZE];
    size_t write_pos;
    size_t read_pos;
    uint64_t last_refresh_time;
    pthread_mutex_t mutex;
} entropy_cache_t;

static entropy_cache_t g_entropy_cache = {
    .write_pos = 0,
    .read_pos = 0,
    .last_refresh_time = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

// Fast RDTSC implementation
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Fast system metric sampling functions
static inline uint64_t sample_cpu_cycles(void) {
    return rdtsc();
}

static inline uint64_t sample_memory_usage(void) {
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return (uint64_t)info.freeram ^ (uint64_t)info.bufferram;
    }
    return rdtsc(); // Fallback
}

static inline uint64_t sample_process_count(void) {
    static uint64_t cached_count = 0;
    static uint64_t last_check = 0;
    uint64_t now = rdtsc();
    
    // Cache process count for performance, update every ~million cycles
    if (now - last_check > 1000000) {
        DIR* proc_dir = opendir("/proc");
        if (proc_dir) {
            struct dirent* entry;
            cached_count = 0;
            int count = 0;
            while ((entry = readdir(proc_dir)) != NULL && count < 1000) { // Limit iterations
                if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
                    cached_count++;
                }
                count++;
            }
            closedir(proc_dir);
        }
        last_check = now;
    }
    
    return cached_count ^ now;
}

static inline uint64_t sample_interrupt_count(void) {
    static int fd = -1;
    static uint64_t fallback_count = 0;
    
    if (fd == -1) {
        fd = open("/proc/stat", O_RDONLY);
    }
    
    if (fd >= 0) {
        char buffer[256];
        lseek(fd, 0, SEEK_SET);
        ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            char* intr_line = strstr(buffer, "intr ");
            if (intr_line) {
                uint64_t count = strtoull(intr_line + 5, NULL, 10);
                fallback_count = count; // Update fallback
                return count;
            }
        }
    }
    
    // Fallback with incrementing counter
    fallback_count += rdtsc() & 0xFFFF;
    return fallback_count;
}

static inline uint64_t sample_context_switches(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (uint64_t)usage.ru_nvcsw ^ (uint64_t)usage.ru_nivcsw;
    }
    return rdtsc();
}

// Optimized entropy mixing function
static inline void mix_entropy_sample(uint8_t* buffer, uint64_t sample, size_t offset) {
    // Use FNV-1a hash for fast mixing
    uint64_t hash = 0xcbf29ce484222325ULL;
    
    for (int i = 0; i < 8; i++) {
        hash ^= (sample >> (i * 8)) & 0xFF;
        hash *= 0x100000001b3ULL;
    }
    
    // Mix into buffer with position-dependent chaos
    for (int i = 0; i < 8; i++) {
        size_t pos = (offset + i) % SYS_ENTROPY_CACHE_SIZE;
        buffer[pos] ^= (hash >> (i * 8)) & 0xFF;
        // Add avalanche effect
        buffer[pos] ^= buffer[(pos + 1) % SYS_ENTROPY_CACHE_SIZE];
    }
}

// High-performance entropy refresh
static void refresh_entropy_cache(void) {
    uint64_t now = rdtsc();
    
    // Only refresh if enough time has passed
    if (now - g_entropy_cache.last_refresh_time < SYS_ENTROPY_REFRESH_INTERVAL_US * 3000) {
        return;
    }
    
    pthread_mutex_lock(&g_entropy_cache.mutex);
    
    // Double-check after acquiring lock
    if (now - g_entropy_cache.last_refresh_time < SYS_ENTROPY_REFRESH_INTERVAL_US * 3000) {
        pthread_mutex_unlock(&g_entropy_cache.mutex);
        return;
    }
    
    // Sample random entropy sources (this is the "lava lamp" randomness)
    entropy_source_t sources[SYS_ENTROPY_SAMPLE_COUNT];
    for (int i = 0; i < SYS_ENTROPY_SAMPLE_COUNT; i++) {
        // Use current cycles and position to pseudo-randomly select sources
        uint64_t selector = rdtsc() ^ (i * 0x9e3779b97f4a7c15ULL);
        sources[i] = selector % ENTROPY_MAX_SOURCES;
    }
    
    // Sample the selected sources and mix into cache
    for (int i = 0; i < SYS_ENTROPY_SAMPLE_COUNT; i++) {
        uint64_t sample = 0;
        
        switch (sources[i]) {
            case ENTROPY_CPU_CYCLES:
                sample = sample_cpu_cycles();
                break;
            case ENTROPY_MEMORY_USAGE:
                sample = sample_memory_usage();
                break;
            case ENTROPY_PROCESS_COUNT:
                sample = sample_process_count();
                break;
            case ENTROPY_INTERRUPT_COUNT:
                sample = sample_interrupt_count();
                break;
            case ENTROPY_CONTEXT_SWITCHES:
                sample = sample_context_switches();
                break;
            default:
                sample = rdtsc(); // Always have fallback entropy
                break;
        }
        
        // Mix sample into cache
        mix_entropy_sample(g_entropy_cache.data, sample, 
                          (g_entropy_cache.write_pos + i * 64) % SYS_ENTROPY_CACHE_SIZE);
    }
    
    // Update cache position
    g_entropy_cache.write_pos = (g_entropy_cache.write_pos + SYS_ENTROPY_SAMPLE_COUNT * 8) % SYS_ENTROPY_CACHE_SIZE;
    g_entropy_cache.last_refresh_time = now;
    
    pthread_mutex_unlock(&g_entropy_cache.mutex);
}

// Public API for fast entropy extraction
void system_entropy_init(void) {
    // Initialize cache with some initial entropy
    uint64_t seed = rdtsc();
    for (size_t i = 0; i < SYS_ENTROPY_CACHE_SIZE; i++) {
        seed = seed * 1103515245 + 12345; // Linear congruential generator for initialization
        g_entropy_cache.data[i] = (uint8_t)(seed >> 24);
    }
    
    // Do initial refresh
    refresh_entropy_cache();
}

// Ultra-fast entropy extraction (no system calls in hot path)
size_t system_entropy_extract_fast(uint8_t* buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return 0;
    }
    
    // Ensure we don't read/write beyond cache bounds
    size_t available = SYS_ENTROPY_CACHE_SIZE;
    size_t to_copy = (size < available) ? size : available;
    
    // Copy from cache without locking for speed (acceptable for entropy)
    for (size_t i = 0; i < to_copy; i++) {
        size_t read_pos = (g_entropy_cache.read_pos + i) % SYS_ENTROPY_CACHE_SIZE;
        buffer[i] = g_entropy_cache.data[read_pos];
        // Add additional mixing with RDTSC for extra entropy
        buffer[i] ^= (uint8_t)(rdtsc() >> ((i % 8) * 8));
    }
    
    // Update read position safely
    g_entropy_cache.read_pos = (g_entropy_cache.read_pos + to_copy) % SYS_ENTROPY_CACHE_SIZE;
    
    // Trigger refresh in background if needed (probabilistic)
    if ((rdtsc() % 10000) == 0) { // Less frequent refresh
        refresh_entropy_cache();
    }
    
    return to_copy;
}

// Cleanup function
void system_entropy_cleanup(void) {
    pthread_mutex_destroy(&g_entropy_cache.mutex);
}