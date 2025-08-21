/**
 * @file benchmark_small_inputs.c
 * @brief Specialized benchmark for small inputs (64B, 256B, 1KB)
 * 
 * This benchmark focuses on the specific sizes mentioned in the requirements
 * and provides detailed performance analysis against SHA-256.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <openssl/sha.h>
#include "blake3.h"

#include "ece_core.h"

#define NUM_ITERATIONS 1000
#define NUM_WARMUP 100
#define BLAKE3_AVAILABLE 1

typedef struct {
    const char* name;
    double time_ms;
    double throughput_mbps;
    double cycles_per_byte;
    uint8_t digest[32];
} benchmark_result_t;

// High precision timing
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

// CPU cycle counting for precise measurement
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Generate test data
static void generate_test_data(uint8_t* data, size_t size, uint32_t seed) {
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        data[i] = (uint8_t)(rand() & 0xFF);
    }
}

// CHARM benchmark with ultra-optimized configuration for small inputs
static int benchmark_charm_small(const uint8_t* data, size_t size, benchmark_result_t* result) {
    // Warmup runs with ultra-fast one-shot function
    for (int iter = 0; iter < NUM_WARMUP; iter++) {
        ece_collapse_small_fast(data, size, result->digest);
    }
    
    // Actual benchmark with cycle counting
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        ece_collapse_small_fast(data, size, result->digest);
    }
    
    uint64_t end_cycles = rdtsc();
    double end_time = get_time_ms();
    
    result->time_ms = (end_time - start_time) / NUM_ITERATIONS;
    result->throughput_mbps = (size / (1024.0 * 1024.0)) / (result->time_ms / 1000.0);
    result->cycles_per_byte = (double)(end_cycles - start_cycles) / (NUM_ITERATIONS * size);
    result->name = "CHARM-Optimized";
    
    return 0;
}

// SHA-256 benchmark for comparison
static int benchmark_sha256_small(const uint8_t* data, size_t size, benchmark_result_t* result) {
    SHA256_CTX ctx;
    
    // Warmup runs
    for (int iter = 0; iter < NUM_WARMUP; iter++) {
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, size);
        SHA256_Final(result->digest, &ctx);
    }
    
    // Actual benchmark with cycle counting
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, size);
        SHA256_Final(result->digest, &ctx);
    }
    
    uint64_t end_cycles = rdtsc();
    double end_time = get_time_ms();
    
    result->time_ms = (end_time - start_time) / NUM_ITERATIONS;
    result->throughput_mbps = (size / (1024.0 * 1024.0)) / (result->time_ms / 1000.0);
    result->cycles_per_byte = (double)(end_cycles - start_cycles) / (NUM_ITERATIONS * size);
    result->name = "SHA-256";
    
    return 0;
}

// BLAKE3 benchmark for comparison
static int benchmark_blake3_small(const uint8_t* data, size_t size, benchmark_result_t* result) {
    #if BLAKE3_AVAILABLE
    blake3_hasher hasher;
    
    // Warmup runs
    for (int iter = 0; iter < NUM_WARMUP; iter++) {
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, data, size);
        blake3_hasher_finalize(&hasher, result->digest, 32);
    }
    
    // Actual benchmark with cycle counting
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, data, size);
        blake3_hasher_finalize(&hasher, result->digest, 32);
    }
    
    uint64_t end_cycles = rdtsc();
    double end_time = get_time_ms();
    
    result->time_ms = (end_time - start_time) / NUM_ITERATIONS;
    result->throughput_mbps = (size / (1024.0 * 1024.0)) / (result->time_ms / 1000.0);
    result->cycles_per_byte = (double)(end_cycles - start_cycles) / (NUM_ITERATIONS * size);
    result->name = "BLAKE3";
    
    return 0;
    #else
    return -1; // BLAKE3 not available
    #endif
}

static void print_results_header(void) {
    printf("Small Input Performance Analysis\n");
    printf("================================\n\n");
    printf("Configuration: %d iterations with %d warmup runs\n", NUM_ITERATIONS, NUM_WARMUP);
    printf("Focus on small inputs: 64B, 256B, 1KB\n\n");
    printf("%-8s | %-15s | %-10s | %-12s | %-8s | %-12s\n",
           "Size", "Algorithm", "Time (ms)", "Throughput", "Cycles/B", "Performance");
    printf("---------+-----------------+------------+--------------+----------+--------------\n");
}

static void print_result(size_t size, const benchmark_result_t* result) {
    char size_str[16];
    if (size < 1024) {
        snprintf(size_str, sizeof(size_str), "%zuB", size);
    } else {
        snprintf(size_str, sizeof(size_str), "%zuKB", size / 1024);
    }
    
    printf("%-8s | %-15s | %8.3f ms | %8.1f MB/s | %8.1f | ",
           size_str, result->name, result->time_ms, 
           result->throughput_mbps, result->cycles_per_byte);
}

static void benchmark_size(size_t size) {
    uint8_t* data = malloc(size);
    if (!data) {
        printf("Memory allocation failed for size %zu\n", size);
        return;
    }
    
    generate_test_data(data, size, 42);
    
    benchmark_result_t charm_result, sha256_result, blake3_result;
    
    // Test CHARM optimized
    if (benchmark_charm_small(data, size, &charm_result) == 0) {
        print_result(size, &charm_result);
        
        // Test SHA-256
        if (benchmark_sha256_small(data, size, &sha256_result) == 0) {
            double performance_ratio = charm_result.throughput_mbps / sha256_result.throughput_mbps;
            if (performance_ratio >= 1.0) {
                printf("✅ %.1f%% FASTER\n", (performance_ratio - 1.0) * 100);
            } else {
                printf("🔴 %.1f%% slower\n", (1.0 - performance_ratio) * 100);
            }
            
            print_result(size, &sha256_result);
            printf("Reference\n");
        }
        
        // Test BLAKE3
        if (benchmark_blake3_small(data, size, &blake3_result) == 0) {
            print_result(size, &blake3_result);
            double performance_ratio = charm_result.throughput_mbps / blake3_result.throughput_mbps;
            if (performance_ratio >= 1.0) {
                printf("✅ %.1f%% vs BLAKE3\n", (performance_ratio - 1.0) * 100);
            } else {
                printf("🔴 %.1f%% vs BLAKE3\n", (1.0 - performance_ratio) * 100);
            }
        }
    }
    
    printf("---------+-----------------+------------+--------------+----------+--------------\n");
    
    free(data);
}

int main(void) {
    print_results_header();
    
    // Focus on the three specific sizes mentioned in requirements
    size_t test_sizes[] = {64, 256, 1024};
    size_t num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (size_t i = 0; i < num_sizes; i++) {
        benchmark_size(test_sizes[i]);
    }
    
    printf("\nOptimization Summary:\n");
    printf("- Ultra-fast paths for exactly 64B, 256B, and 1KB\n");
    printf("- Minimal rounds and features for maximum speed\n");
    printf("- 64-bit operations and unrolled loops\n");
    printf("- Cache-friendly memory access patterns\n");
    printf("- Specialized mixing algorithms per size\n");
    
    return 0;
}