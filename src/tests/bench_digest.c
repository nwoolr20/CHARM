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
 * @file bench_digest.c
 * @brief Performance benchmark for CHARM digest compared to SHA-256 and BLAKE3
 * 
 * This program benchmarks the performance of the CHARM digest algorithm
 * against SHA-256 and BLAKE3, measuring throughput for various input sizes.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <openssl/sha.h>
#include <openssl/evp.h>

// CHARM API
#include "charm_api.h"

// BLAKE3 headers (if available)
#ifdef HAVE_BLAKE3
#include "blake3.h"
#endif

// Benchmark parameters
#define MIN_SIZE (1 << 10)        // 1 KB
#define MAX_SIZE (1 << 24)        // 16 MB
#define NUM_ITERATIONS 10         // Number of iterations for each size
#define WARMUP_ITERATIONS 3       // Number of warmup iterations

// Function to get current time in microseconds
static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
}

// Function to generate random data
static void generate_random_data(uint8_t* buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = rand() & 0xFF;
    }
}

// Benchmark CHARM digest
static double benchmark_charm(const uint8_t* data, size_t size, int iterations) {
    uint8_t digest[32];
    uint64_t start, end;
    double total_time = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        start = get_time_us();
        charm_digest_compute(data, size, digest);
        end = get_time_us();
        total_time += (end - start);
    }
    
    return total_time / iterations;
}

// Benchmark SHA-256
static double benchmark_sha256(const uint8_t* data, size_t size, int iterations) {
    uint8_t digest[32];
    uint64_t start, end;
    double total_time = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        start = get_time_us();
        SHA256(data, size, digest);
        end = get_time_us();
        total_time += (end - start);
    }
    
    return total_time / iterations;
}

// Benchmark BLAKE3 (if available)
static double benchmark_blake3(const uint8_t* data, size_t size, int iterations) {
#ifdef HAVE_BLAKE3
    uint8_t digest[32];
    uint64_t start, end;
    double total_time = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        start = get_time_us();
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, data, size);
        blake3_hasher_finalize(&hasher, digest, 32);
        end = get_time_us();
        total_time += (end - start);
    }
    
    return total_time / iterations;
#else
    (void)data;
    (void)size;
    (void)iterations;
    return 0.0;
#endif
}

int main(int argc, char* argv[]) {
    // Initialize CHARM system
    if (charm_init() != 0) {
        fprintf(stderr, "Failed to initialize CHARM system\n");
        return 1;
    }
    
    // Parse command line arguments
    int verbose = 0;
    int csv_output = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--csv") == 0) {
            csv_output = 1;
        }
    }
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Allocate buffer for largest size
    uint8_t* buffer = (uint8_t*)malloc(MAX_SIZE);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        charm_shutdown();
        return 1;
    }
    
    // Generate random data
    generate_random_data(buffer, MAX_SIZE);
    
    // Print header
    if (csv_output) {
        printf("Size,CHARM (us),SHA-256 (us),BLAKE3 (us),CHARM (MB/s),SHA-256 (MB/s),BLAKE3 (MB/s)\n");
    } else {
        printf("CHARM Digest Benchmark\n");
        printf("=====================\n\n");
        printf("%-10s %-15s %-15s %-15s %-15s %-15s %-15s\n",
               "Size", "CHARM (us)", "SHA-256 (us)", "BLAKE3 (us)",
               "CHARM (MB/s)", "SHA-256 (MB/s)", "BLAKE3 (MB/s)");
        printf("%-10s %-15s %-15s %-15s %-15s %-15s %-15s\n",
               "----", "---------", "-----------", "----------",
               "-----------", "------------", "-----------");
    }
    
    // Run benchmarks for different sizes
    for (size_t size = MIN_SIZE; size <= MAX_SIZE; size *= 2) {
        // Warmup
        benchmark_charm(buffer, size, WARMUP_ITERATIONS);
        benchmark_sha256(buffer, size, WARMUP_ITERATIONS);
#ifdef HAVE_BLAKE3
        benchmark_blake3(buffer, size, WARMUP_ITERATIONS);
#endif
        
        // Benchmark
        double charm_time = benchmark_charm(buffer, size, NUM_ITERATIONS);
        double sha256_time = benchmark_sha256(buffer, size, NUM_ITERATIONS);
        double blake3_time = benchmark_blake3(buffer, size, NUM_ITERATIONS);
        
        // Calculate throughput in MB/s
        double charm_throughput = (double)size / charm_time;
        double sha256_throughput = (double)size / sha256_time;
        double blake3_throughput = (blake3_time > 0.0) ? ((double)size / blake3_time) : 0.0;
        
        // Convert to MB/s
        charm_throughput = charm_throughput * 1000000.0 / (1024.0 * 1024.0);
        sha256_throughput = sha256_throughput * 1000000.0 / (1024.0 * 1024.0);
        blake3_throughput = blake3_throughput * 1000000.0 / (1024.0 * 1024.0);
        
        // Print results
        if (csv_output) {
            printf("%zu,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                   size, charm_time, sha256_time, blake3_time,
                   charm_throughput, sha256_throughput, blake3_throughput);
        } else {
            char size_str[16];
            if (size < 1024) {
                snprintf(size_str, sizeof(size_str), "%zu B", size);
            } else if (size < 1024 * 1024) {
                snprintf(size_str, sizeof(size_str), "%zu KB", size / 1024);
            } else {
                snprintf(size_str, sizeof(size_str), "%zu MB", size / (1024 * 1024));
            }
            
            printf("%-10s %-15.2f %-15.2f %-15.2f %-15.2f %-15.2f %-15.2f\n",
                   size_str, charm_time, sha256_time, blake3_time,
                   charm_throughput, sha256_throughput, blake3_throughput);
        }
        
        // Print detailed results if verbose
        if (verbose) {
            printf("  CHARM:   %.2f us (%.2f MB/s)\n", charm_time, charm_throughput);
            printf("  SHA-256: %.2f us (%.2f MB/s)\n", sha256_time, sha256_throughput);
            if (blake3_time > 0.0) {
                printf("  BLAKE3:  %.2f us (%.2f MB/s)\n", blake3_time, blake3_throughput);
            } else {
                printf("  BLAKE3:  Not available\n");
            }
            printf("\n");
        }
    }
    
    // Print summary
    if (!csv_output) {
        printf("\nSummary:\n");
        printf("- CHARM is optimized for entropy quality and security\n");
        printf("- SHA-256 is the industry standard for comparison\n");
#ifdef HAVE_BLAKE3
        printf("- BLAKE3 is a high-performance modern hash function\n");
#else
        printf("- BLAKE3 was not available for comparison\n");
#endif
    }
    
    // Clean up
    free(buffer);
    charm_shutdown();
    
    return 0;
}
