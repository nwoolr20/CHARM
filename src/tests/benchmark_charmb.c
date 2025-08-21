/**
 * @file benchmark_charmb.c
 * @brief Specialized benchmark for CHARM-B (CHARMbit) ultra-small inputs
 * 
 * This benchmark focuses on very small inputs (8B, 16B, 32B, 64B) and demonstrates
 * CHARM-B's superior performance compared to both regular CHARM and SHA-256.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <openssl/sha.h>
#include "blake3.h"

#include "ece_core.h"
#include "charmb_core.h"

#define NUM_ITERATIONS 5000  // More iterations for tiny inputs
#define NUM_WARMUP 500

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

// Benchmark CHARM-B
static void benchmark_charmb(const uint8_t* data, size_t size, benchmark_result_t* result, bool use_128bit) {
    charmb_digest_size_t digest_size = use_128bit ? CHARMB_DIGEST_128 : CHARMB_DIGEST_256;
    
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        charmb_hash(data, size, result->digest, digest_size);
    }
    
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_hash(data, size, result->digest, digest_size);
    }
    
    double end_time = get_time_ms();
    uint64_t end_cycles = rdtsc();
    
    result->time_ms = (end_time - start_time) / NUM_ITERATIONS;
    double seconds = result->time_ms / 1000.0;
    result->throughput_mbps = (size * NUM_ITERATIONS) / (seconds * 1024 * 1024);
    result->cycles_per_byte = (double)(end_cycles - start_cycles) / (size * NUM_ITERATIONS);
}

// Benchmark regular CHARM (optimized)
static void benchmark_charm_optimized(const uint8_t* data, size_t size, benchmark_result_t* result) {
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        ece_collapse_small_fast(data, size, result->digest);
    }
    
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ece_collapse_small_fast(data, size, result->digest);
    }
    
    double end_time = get_time_ms();
    uint64_t end_cycles = rdtsc();
    
    result->time_ms = (end_time - start_time) / NUM_ITERATIONS;
    double seconds = result->time_ms / 1000.0;
    result->throughput_mbps = (size * NUM_ITERATIONS) / (seconds * 1024 * 1024);
    result->cycles_per_byte = (double)(end_cycles - start_cycles) / (size * NUM_ITERATIONS);
}

// Benchmark SHA-256
static void benchmark_sha256(const uint8_t* data, size_t size, benchmark_result_t* result) {
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, size);
        SHA256_Final(result->digest, &ctx);
    }
    
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        SHA256_CTX ctx;
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, size);
        SHA256_Final(result->digest, &ctx);
    }
    
    double end_time = get_time_ms();
    uint64_t end_cycles = rdtsc();
    
    result->time_ms = (end_time - start_time) / NUM_ITERATIONS;
    double seconds = result->time_ms / 1000.0;
    result->throughput_mbps = (size * NUM_ITERATIONS) / (seconds * 1024 * 1024);
    result->cycles_per_byte = (double)(end_cycles - start_cycles) / (size * NUM_ITERATIONS);
}

// Calculate performance improvement
static double calc_improvement(double new_val, double old_val) {
    return ((new_val - old_val) / old_val) * 100.0;
}

// Print performance comparison
static void print_comparison(const char* size_str, benchmark_result_t* charmb, 
                           benchmark_result_t* charm, benchmark_result_t* sha256) {
    printf("%-8s | %-15s | %8.1f ms | %10.1f MB/s | %8.1f | ", 
           size_str, "CHARM-B", charmb->time_ms, charmb->throughput_mbps, charmb->cycles_per_byte);
    
    double vs_charm = calc_improvement(charmb->throughput_mbps, charm->throughput_mbps);
    double vs_sha = calc_improvement(charmb->throughput_mbps, sha256->throughput_mbps);
    
    if (vs_charm > 0) {
        printf("✅ +%.1f%% vs CHARM\n", vs_charm);
    } else {
        printf("🔴 %.1f%% vs CHARM\n", vs_charm);
    }
    
    printf("%-8s | %-15s | %8.1f ms | %10.1f MB/s | %8.1f | ", 
           "", "CHARM Optimized", charm->time_ms, charm->throughput_mbps, charm->cycles_per_byte);
    printf("Reference\n");
    
    printf("%-8s | %-15s | %8.1f ms | %10.1f MB/s | %8.1f | ", 
           "", "SHA-256", sha256->time_ms, sha256->throughput_mbps, sha256->cycles_per_byte);
    
    if (vs_sha > 0) {
        printf("✅ CHARM-B +%.1f%% faster\n", vs_sha);
    } else {
        printf("🔴 CHARM-B %.1f%% slower\n", -vs_sha);
    }
    
    printf("---------+-----------------+------------+--------------+----------+--------------\n");
}

int main() {
    printf("CHARM-B (CHARMbit) Ultra-Small Input Performance Analysis\n");
    printf("========================================================\n\n");
    
    printf("Configuration: %d iterations with %d warmup runs\n", NUM_ITERATIONS, NUM_WARMUP);
    printf("Focus on ultra-small inputs: 8B, 16B, 32B, 64B\n");
    printf("CHARM-B Version: %s\n", charmb_get_version());
    printf("CPU Support: %s\n\n", charmb_cpu_support() ? "✅ Optimized" : "⚠️ Fallback");
    
    printf("Size     | Algorithm       | Time (ms)  | Throughput   | Cycles/B | Performance \n");
    printf("---------+-----------------+------------+--------------+----------+--------------\n");
    
    // Test sizes optimized for CHARM-B
    size_t test_sizes[] = {8, 16, 32, 64};
    const char* size_names[] = {"8B", "16B", "32B", "64B"};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        uint8_t test_data[64];
        generate_test_data(test_data, size, 0x12345678 + i);
        
        benchmark_result_t charmb_result = {0};
        benchmark_result_t charm_result = {0};
        benchmark_result_t sha256_result = {0};
        
        charmb_result.name = "CHARM-B";
        charm_result.name = "CHARM Optimized";
        sha256_result.name = "SHA-256";
        
        // Run benchmarks
        benchmark_charmb(test_data, size, &charmb_result, false); // 256-bit output
        benchmark_charm_optimized(test_data, size, &charm_result);
        benchmark_sha256(test_data, size, &sha256_result);
        
        print_comparison(size_names[i], &charmb_result, &charm_result, &sha256_result);
    }
    
    // Test 128-bit output performance
    printf("\nCHARM-B 128-bit Digest Performance:\n");
    printf("Size     | Throughput   | vs 256-bit | Use Case\n");
    printf("---------+--------------+------------+----------\n");
    
    for (int i = 0; i < num_sizes; i++) {
        size_t size = test_sizes[i];
        uint8_t test_data[64];
        generate_test_data(test_data, size, 0x87654321 + i);
        
        benchmark_result_t charmb_128 = {0};
        benchmark_result_t charmb_256 = {0};
        
        benchmark_charmb(test_data, size, &charmb_128, true);  // 128-bit
        benchmark_charmb(test_data, size, &charmb_256, false); // 256-bit
        
        double improvement = calc_improvement(charmb_128.throughput_mbps, charmb_256.throughput_mbps);
        
        printf("%-8s | %10.1f MB/s | %+8.1f%% | Micro-hashing\n", 
               size_names[i], charmb_128.throughput_mbps, improvement);
    }
    
    printf("\nSpecialized Function Performance:\n");
    printf("Function        | Throughput   | Optimization\n");
    printf("----------------+--------------+--------------\n");
    
    // Test specialized functions
    uint8_t test_8b[8], test_16b[16], test_32b[32], test_64b[64];
    uint8_t digest[32];
    
    generate_test_data(test_8b, 8, 0x11111111);
    generate_test_data(test_16b, 16, 0x22222222);
    generate_test_data(test_32b, 32, 0x33333333);
    generate_test_data(test_64b, 64, 0x44444444);
    
    // Benchmark specialized functions
    double start_time, end_time;
    
    // 8B specialized
    start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_hash_8b(test_8b, digest, CHARMB_DIGEST_256);
    }
    end_time = get_time_ms();
    double throughput_8b = (8 * NUM_ITERATIONS) / ((end_time - start_time) / 1000.0 * 1024 * 1024);
    
    // 16B specialized
    start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_hash_16b(test_16b, digest, CHARMB_DIGEST_256);
    }
    end_time = get_time_ms();
    double throughput_16b = (16 * NUM_ITERATIONS) / ((end_time - start_time) / 1000.0 * 1024 * 1024);
    
    // 32B specialized
    start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_hash_32b(test_32b, digest, CHARMB_DIGEST_256);
    }
    end_time = get_time_ms();
    double throughput_32b = (32 * NUM_ITERATIONS) / ((end_time - start_time) / 1000.0 * 1024 * 1024);
    
    // 64B specialized
    start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_hash_64b(test_64b, digest, CHARMB_DIGEST_256);
    }
    end_time = get_time_ms();
    double throughput_64b = (64 * NUM_ITERATIONS) / ((end_time - start_time) / 1000.0 * 1024 * 1024);
    
    printf("charmb_hash_8b  | %10.1f MB/s | Direct 64-bit\n", throughput_8b);
    printf("charmb_hash_16b | %10.1f MB/s | SIMD 128-bit\n", throughput_16b);
    printf("charmb_hash_32b | %10.1f MB/s | Cross-SIMD\n", throughput_32b);
    printf("charmb_hash_64b | %10.1f MB/s | Parallel SIMD\n", throughput_64b);
    
    printf("\nCHARM-B Summary:\n");
    printf("- Specialized algorithms for exact sizes (8B, 16B, 32B, 64B)\n");
    printf("- Bit-level optimization for arbitrary small inputs\n");
    printf("- Zero malloc overhead with thread-local state\n");
    printf("- SIMD acceleration with AVX2 support\n");
    printf("- Optional 128-bit digest for micro-hashing\n");
    
    return 0;
}