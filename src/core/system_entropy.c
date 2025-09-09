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
 * @brief Simplified high-performance system entropy sampling
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

// Simple entropy cache for high-performance access
#define SYS_ENTROPY_CACHE_SIZE 1024

typedef struct {
    uint8_t data[SYS_ENTROPY_CACHE_SIZE];
    size_t write_pos;
    size_t read_pos;
    uint64_t counter;
} entropy_cache_t;

static entropy_cache_t g_entropy_cache = {0};

// Fast RDTSC implementation
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Simple but effective entropy mixing
static inline void mix_entropy(uint8_t* buffer, uint64_t sample, size_t offset) {
    for (int i = 0; i < 8 && offset + i < SYS_ENTROPY_CACHE_SIZE; i++) {
        buffer[offset + i] ^= (sample >> (i * 8)) & 0xFF;
    }
}

// Fast entropy generation using multiple sources
static void refresh_entropy_cache(void) {
    // Use multiple fast entropy sources
    uint64_t samples[8];
    samples[0] = rdtsc();
    samples[1] = (uint64_t)getpid() ^ samples[0];
    samples[2] = (uint64_t)pthread_self() ^ samples[1];
    samples[3] = g_entropy_cache.counter++;
    
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    samples[4] = (uint64_t)ts.tv_nsec ^ samples[3];
    
    // Add some memory address entropy
    volatile void* stack_addr = &samples;
    samples[5] = (uint64_t)(uintptr_t)stack_addr ^ samples[4];
    
    // Final mixing
    samples[6] = samples[0] ^ samples[2] ^ samples[4];
    samples[7] = samples[1] ^ samples[3] ^ samples[5];
    
    // Mix all samples into cache
    for (int i = 0; i < 8; i++) {
        size_t offset = (g_entropy_cache.write_pos + i * 64) % SYS_ENTROPY_CACHE_SIZE;
        mix_entropy(g_entropy_cache.data, samples[i], offset);
    }
    
    g_entropy_cache.write_pos = (g_entropy_cache.write_pos + 64) % SYS_ENTROPY_CACHE_SIZE;
}

void system_entropy_init(void) {
    // Initialize cache with some entropy
    uint64_t seed = rdtsc();
    for (size_t i = 0; i < SYS_ENTROPY_CACHE_SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        g_entropy_cache.data[i] = (uint8_t)(seed >> 8);
    }
    refresh_entropy_cache();
}

size_t system_entropy_extract_fast(uint8_t* buffer, size_t size) {
    if (!buffer || size == 0) return 0;
    
    size_t to_copy = size > SYS_ENTROPY_CACHE_SIZE ? SYS_ENTROPY_CACHE_SIZE : size;
    
    // Copy from cache with mixing
    for (size_t i = 0; i < to_copy; i++) {
        size_t pos = (g_entropy_cache.read_pos + i) % SYS_ENTROPY_CACHE_SIZE;
        buffer[i] = g_entropy_cache.data[pos] ^ (uint8_t)(rdtsc() >> (i % 8));
    }
    
    g_entropy_cache.read_pos = (g_entropy_cache.read_pos + to_copy) % SYS_ENTROPY_CACHE_SIZE;
    
    // Refresh occasionally
    if ((g_entropy_cache.counter++ % 100) == 0) {
        refresh_entropy_cache();
    }
    
    return to_copy;
}

void system_entropy_cleanup(void) {
    memset(&g_entropy_cache, 0, sizeof(g_entropy_cache));
}