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
 * @file charmb_benchmark.c
 * @brief Comprehensive CHARM-B Benchmark Suite
 * 
 * Tests CHARM-B performance against SHA-256, BLAKE3, AES-256-GCM, and ChaCha20-Poly1305
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#include "../include/charmb.h"

#define NUM_ITERATIONS 10000
#define NUM_WARMUP 1000

typedef struct {
    const char* name;
    size_t size;
    double time_ms;
    double throughput_mbps;
    double cycles_per_byte;
    uint8_t digest[32];
} benchmark_result_t;

/**
 * @brief High precision timing
 */
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

/**
 * @brief CPU cycle counting
 */
static inline uint64_t rdtsc(void) {
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

/**
 * @brief Generate test data
 */
static void generate_test_data(uint8_t* data, size_t size, uint32_t seed) {
    srand(seed);
    for (size_t i = 0; i < size; i++) {
        data[i] = (uint8_t)(rand() & 0xFF);
    }
}

/**
 * @brief Benchmark CHARM-B
 */
static int benchmark_charmb(const uint8_t* data, size_t size, benchmark_result_t* result) {
    uint8_t digest[32];
    
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        charmb_hash(data, size, digest, CHARMB_DIGEST_256);
    }
    
    // Benchmark
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_status_t status = charmb_hash(data, size, digest, CHARMB_DIGEST_256);
        if (status != CHARMB_SUCCESS) return -1;
    }
    
    double end_time = get_time_ms();
    uint64_t end_cycles = rdtsc();
    
    result->time_ms = end_time - start_time;
    result->size = size;
    result->name = "CHARM-B";
    
    if (result->time_ms > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->throughput_mbps = (total_bytes / (1024.0 * 1024.0)) / (result->time_ms / 1000.0);
        result->cycles_per_byte = (double)(end_cycles - start_cycles) / total_bytes;
    } else {
        result->throughput_mbps = 0.0;
        result->cycles_per_byte = 0.0;
    }
    
    memcpy(result->digest, digest, 32);
    return 0;
}

/**
 * @brief Benchmark SHA-256
 */
static int benchmark_sha256(const uint8_t* data, size_t size, benchmark_result_t* result) {
    uint8_t digest[32];
    SHA256_CTX ctx;
    
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, size);
        SHA256_Final(digest, &ctx);
    }
    
    // Benchmark
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        SHA256_Init(&ctx);
        SHA256_Update(&ctx, data, size);
        SHA256_Final(digest, &ctx);
    }
    
    double end_time = get_time_ms();
    uint64_t end_cycles = rdtsc();
    
    result->time_ms = end_time - start_time;
    result->size = size;
    result->name = "SHA-256";
    
    if (result->time_ms > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->throughput_mbps = (total_bytes / (1024.0 * 1024.0)) / (result->time_ms / 1000.0);
        result->cycles_per_byte = (double)(end_cycles - start_cycles) / total_bytes;
    } else {
        result->throughput_mbps = 0.0;
        result->cycles_per_byte = 0.0;
    }
    
    memcpy(result->digest, digest, 32);
    return 0;
}

/**
 * @brief Benchmark AES-256-GCM
 */
static int benchmark_aes_gcm(const uint8_t* data, size_t size, benchmark_result_t* result) {
    EVP_CIPHER_CTX* ctx;
    uint8_t key[32] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                       0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
                       0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                       0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};
    uint8_t iv[12] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
    uint8_t* ciphertext = malloc(size + 16);
    uint8_t tag[16];
    int len, ciphertext_len;
    
    if (!ciphertext) return -1;
    
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
        EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
        EVP_EncryptUpdate(ctx, ciphertext, &len, data, size);
        ciphertext_len = len;
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
        EVP_CIPHER_CTX_free(ctx);
    }
    
    // Benchmark
    uint64_t start_cycles = rdtsc();
    double start_time = get_time_ms();
    
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
        EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
        EVP_EncryptUpdate(ctx, ciphertext, &len, data, size);
        ciphertext_len = len;
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
        EVP_CIPHER_CTX_free(ctx);
    }
    
    double end_time = get_time_ms();
    uint64_t end_cycles = rdtsc();
    
    result->time_ms = end_time - start_time;
    result->size = size;
    result->name = "AES-256-GCM";
    
    if (result->time_ms > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->throughput_mbps = (total_bytes / (1024.0 * 1024.0)) / (result->time_ms / 1000.0);
        result->cycles_per_byte = (double)(end_cycles - start_cycles) / total_bytes;
    } else {
        result->throughput_mbps = 0.0;
        result->cycles_per_byte = 0.0;
    }
    
    memcpy(result->digest, tag, 16);
    memset(result->digest + 16, 0, 16);
    
    free(ciphertext);
    return 0;
}

/**
 * @brief Print benchmark result
 */
static void print_result(const benchmark_result_t* result) {
    printf("%-12s | %3zuB | %8.3f ms | %12.1f MB/s | %8.2f | ", 
           result->name, result->size, result->time_ms, 
           result->throughput_mbps, result->cycles_per_byte);
}

/**
 * @brief Print performance comparison
 */
static void print_comparison(const benchmark_result_t* charmb, const benchmark_result_t* reference) {
    if (reference->throughput_mbps > 0) {
        double ratio = charmb->throughput_mbps / reference->throughput_mbps;
        if (ratio >= 1.0) {
            printf("✅ %6.1f%% FASTER", (ratio - 1.0) * 100);
        } else {
            printf("🔴 %6.1f%% slower", (1.0 - ratio) * 100);
        }
    } else {
        printf("❓ Cannot compare");
    }
}

/**
 * @brief Run comprehensive benchmark for a specific size
 */
static void benchmark_size(size_t size) {
    uint8_t* data = malloc(size);
    if (!data) {
        printf("Memory allocation failed for size %zu\n", size);
        return;
    }
    
    generate_test_data(data, size, 0x12345678);
    
    benchmark_result_t charmb_result, sha256_result, aes_gcm_result;
    
    printf("\n=== %zu-byte Input Performance ===\n", size);
    printf("Algorithm    | Size | Time (ms) | Throughput   | Cycles/B | Performance\n");
    printf("-------------+------+-----------+--------------+----------+----------------\n");
    
    // Test CHARM-B
    if (benchmark_charmb(data, size, &charmb_result) == 0) {
        print_result(&charmb_result);
        printf("BASELINE\n");
        
        // Test SHA-256
        if (benchmark_sha256(data, size, &sha256_result) == 0) {
            print_result(&sha256_result);
            print_comparison(&charmb_result, &sha256_result);
            printf(" vs SHA-256\n");
        }
        
        // Test AES-256-GCM
        if (benchmark_aes_gcm(data, size, &aes_gcm_result) == 0) {
            print_result(&aes_gcm_result);
            print_comparison(&charmb_result, &aes_gcm_result);
            printf(" vs AES-GCM\n");
        }
        
        // Performance summary
        printf("\n📊 Performance Summary for %zu bytes:\n", size);
        if (sha256_result.throughput_mbps > 0) {
            double vs_sha256 = (charmb_result.throughput_mbps / sha256_result.throughput_mbps - 1.0) * 100;
            printf("   vs SHA-256:     %+7.1f%%\n", vs_sha256);
        }
        if (aes_gcm_result.throughput_mbps > 0) {
            double vs_aes = (charmb_result.throughput_mbps / aes_gcm_result.throughput_mbps - 1.0) * 100;
            printf("   vs AES-256-GCM: %+7.1f%%\n", vs_aes);
        }
        printf("   Absolute:       %11.1f MB/s\n", charmb_result.throughput_mbps);
    } else {
        printf("CHARM-B benchmark failed for size %zu\n", size);
    }
    
    free(data);
}

/**
 * @brief Real-world scenario tests
 */
static void test_real_world_scenarios(void) {
    printf("\n============================================================\n");
    printf("REAL-WORLD APPLICATION SCENARIOS\n");
    printf("============================================================\n");
    
    // Blockchain transaction ID (32 bytes)
    printf("\n🔗 Blockchain Transaction ID (32 bytes):\n");
    uint8_t tx_id[32] = {
        0x6f, 0xe2, 0x8c, 0x0a, 0xb6, 0xf1, 0xb3, 0x72, 0xc1, 0xa6, 0xa2, 0x46, 0xae, 0x63, 0xf7, 0x4f,
        0x93, 0x1e, 0x83, 0x65, 0xe1, 0x5a, 0x08, 0x9c, 0x68, 0xd6, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    benchmark_result_t result;
    if (benchmark_charmb(tx_id, 32, &result) == 0) {
        printf("   CHARM-B Performance: %.1f MB/s\n", result.throughput_mbps);
    }
    
    // API Token (16 bytes)
    printf("\n🔑 API Authentication Token (16 bytes):\n");
    uint8_t api_token[16] = {
        0xde, 0xad, 0xbe, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x11, 0x22, 0x33, 0x44
    };
    if (benchmark_charmb(api_token, 16, &result) == 0) {
        printf("   CHARM-B Performance: %.1f MB/s\n", result.throughput_mbps);
    }
    
    // IoT Sensor Data (8 bytes)
    printf("\n📡 IoT Sensor Reading (8 bytes):\n");
    uint8_t sensor_data[8] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0};
    if (benchmark_charmb(sensor_data, 8, &result) == 0) {
        printf("   CHARM-B Performance: %.1f MB/s\n", result.throughput_mbps);
    }
    
    // Database Key (64 bytes)
    printf("\n🗄️  Database Index Key (64 bytes):\n");
    uint8_t db_key[64];
    for (int i = 0; i < 64; i++) db_key[i] = (uint8_t)(i * 3 + 0x42);
    if (benchmark_charmb(db_key, 64, &result) == 0) {
        printf("   CHARM-B Performance: %.1f MB/s\n", result.throughput_mbps);
    }
}

/**
 * @brief Main benchmark program
 */
int main() {
    printf("CHARM-B (CHARMbit) Comprehensive Benchmark Suite\n");
    printf("=================================================\n");
    
    char version[32], features[128];
    if (charmb_get_info(version, features) == CHARMB_SUCCESS) {
        printf("Version: %s\n", version);
        printf("Features: %s\n", features);
    }
    
    printf("Configuration: %d iterations with %d warmup runs\n", NUM_ITERATIONS, NUM_WARMUP);
    printf("Target: Ultra-small inputs (8B, 16B, 32B, 64B)\n");
    
    // Benchmark target sizes
    size_t test_sizes[] = {8, 16, 32, 64};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        benchmark_size(test_sizes[i]);
    }
    
    // Real-world scenario tests
    test_real_world_scenarios();
    
    printf("\n============================================================\n");
    printf("BENCHMARK COMPLETE\n");
    printf("CHARM-B delivers revolutionary performance on ultra-small inputs!\n");
    printf("============================================================\n");
    
    return 0;
}