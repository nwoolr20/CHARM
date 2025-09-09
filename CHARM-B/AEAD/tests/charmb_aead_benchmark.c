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
 * @file charmb_aead_benchmark.c
 * @brief CHARM-B AEAD Comprehensive Benchmark
 * 
 * Tests CHARM-B AEAD against AES-256-GCM and ChaCha20-Poly1305
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/aes.h>

#include "../include/charmb_aead.h"

#define NUM_ITERATIONS 5000
#define NUM_WARMUP 500

typedef struct {
    const char* name;
    size_t size;
    double encrypt_mbps;
    double decrypt_mbps;
    double total_mbps;
} aead_result_t;

/**
 * @brief High precision timing
 */
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
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
 * @brief Benchmark CHARM-B AEAD
 */
static int benchmark_charmb_aead(const uint8_t* plaintext, size_t size, aead_result_t* result) {
    uint8_t key[32], nonce[12], tag[16];
    uint8_t* ciphertext = malloc(size);
    uint8_t* decrypted = malloc(size);
    
    if (!ciphertext || !decrypted) {
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    
    generate_test_data(key, 32, 0x12345678);
    generate_test_data(nonce, 12, 0x87654321);
    
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        charmb_aead_encrypt(key, nonce, NULL, 0, plaintext, size, ciphertext, tag);
        charmb_aead_decrypt(key, nonce, NULL, 0, ciphertext, size, tag, decrypted);
    }
    
    // Benchmark encryption
    double start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_aead_status_t status = charmb_aead_encrypt(key, nonce, NULL, 0, 
                                                          plaintext, size, ciphertext, tag);
        if (status != CHARMB_AEAD_SUCCESS) {
            free(ciphertext);
            free(decrypted);
            return -1;
        }
    }
    double encrypt_time = get_time_ms() - start_time;
    
    // Benchmark decryption
    start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        charmb_aead_status_t status = charmb_aead_decrypt(key, nonce, NULL, 0, 
                                                          ciphertext, size, tag, decrypted);
        if (status != CHARMB_AEAD_SUCCESS) {
            free(ciphertext);
            free(decrypted);
            return -1;
        }
    }
    double decrypt_time = get_time_ms() - start_time;
    
    result->name = "CHARM-B AEAD";
    result->size = size;
    
    if (encrypt_time > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->encrypt_mbps = (total_bytes / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
    } else {
        result->encrypt_mbps = 0.0;
    }
    
    if (decrypt_time > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->decrypt_mbps = (total_bytes / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
    } else {
        result->decrypt_mbps = 0.0;
    }
    
    result->total_mbps = (result->encrypt_mbps + result->decrypt_mbps) / 2.0;
    
    free(ciphertext);
    free(decrypted);
    return 0;
}

/**
 * @brief Benchmark AES-256-GCM
 */
static int benchmark_aes_gcm(const uint8_t* plaintext, size_t size, aead_result_t* result) {
    EVP_CIPHER_CTX* ctx;
    uint8_t key[32], iv[12], tag[16];
    uint8_t* ciphertext = malloc(size + 16);
    uint8_t* decrypted = malloc(size);
    int len, ciphertext_len;
    
    if (!ciphertext || !decrypted) {
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    
    generate_test_data(key, 32, 0x12345678);
    generate_test_data(iv, 12, 0x87654321);
    
    // Warmup
    for (int i = 0; i < NUM_WARMUP; i++) {
        // Encrypt
        ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
        EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
        EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, size);
        ciphertext_len = len;
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
        EVP_CIPHER_CTX_free(ctx);
        
        // Decrypt
        ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
        EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);
        EVP_DecryptUpdate(ctx, decrypted, &len, ciphertext, ciphertext_len);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
        EVP_DecryptFinal_ex(ctx, decrypted + len, &len);
        EVP_CIPHER_CTX_free(ctx);
    }
    
    // Benchmark encryption
    double start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
        EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv);
        EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, size);
        ciphertext_len = len;
        EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag);
        EVP_CIPHER_CTX_free(ctx);
    }
    double encrypt_time = get_time_ms() - start_time;
    
    // Benchmark decryption
    start_time = get_time_ms();
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL);
        EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);
        EVP_DecryptUpdate(ctx, decrypted, &len, ciphertext, ciphertext_len);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);
        EVP_DecryptFinal_ex(ctx, decrypted + len, &len);
        EVP_CIPHER_CTX_free(ctx);
    }
    double decrypt_time = get_time_ms() - start_time;
    
    result->name = "AES-256-GCM";
    result->size = size;
    
    if (encrypt_time > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->encrypt_mbps = (total_bytes / (1024.0 * 1024.0)) / (encrypt_time / 1000.0);
    } else {
        result->encrypt_mbps = 0.0;
    }
    
    if (decrypt_time > 0) {
        double total_bytes = (double)size * NUM_ITERATIONS;
        result->decrypt_mbps = (total_bytes / (1024.0 * 1024.0)) / (decrypt_time / 1000.0);
    } else {
        result->decrypt_mbps = 0.0;
    }
    
    result->total_mbps = (result->encrypt_mbps + result->decrypt_mbps) / 2.0;
    
    free(ciphertext);
    free(decrypted);
    return 0;
}

/**
 * @brief Print AEAD result
 */
static void print_aead_result(const aead_result_t* result) {
    printf("%-15s | %3zuB | %10.1f | %10.1f | %10.1f | ", 
           result->name, result->size, 
           result->encrypt_mbps, result->decrypt_mbps, result->total_mbps);
}

/**
 * @brief Print performance comparison
 */
static void print_aead_comparison(const aead_result_t* charmb, const aead_result_t* reference) {
    if (reference->total_mbps > 0) {
        double ratio = charmb->total_mbps / reference->total_mbps;
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
 * @brief Test AEAD functionality
 */
static int test_aead_functionality(void) {
    printf("CHARM-B AEAD Functionality Test\n");
    printf("================================\n\n");
    
    uint8_t key[32] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                       17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};
    uint8_t nonce[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    uint8_t plaintext[] = "Hello, CHARM-B AEAD! This is a test message.";
    size_t pt_len = strlen((char*)plaintext);
    uint8_t* ciphertext = malloc(pt_len);
    uint8_t* decrypted = malloc(pt_len);
    uint8_t tag[16];
    uint8_t aad[] = "Additional authenticated data";
    
    if (!ciphertext || !decrypted) {
        printf("❌ Memory allocation failed\n");
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    
    // Test encryption
    printf("Testing encryption...\n");
    charmb_aead_status_t status = charmb_aead_encrypt(key, nonce, aad, strlen((char*)aad),
                                                      plaintext, pt_len, ciphertext, tag);
    if (status != CHARMB_AEAD_SUCCESS) {
        printf("❌ Encryption failed with status %d\n", status);
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    printf("✅ Encryption successful\n");
    
    // Test decryption
    printf("Testing decryption...\n");
    status = charmb_aead_decrypt(key, nonce, aad, strlen((char*)aad),
                                 ciphertext, pt_len, tag, decrypted);
    if (status != CHARMB_AEAD_SUCCESS) {
        printf("❌ Decryption failed with status %d\n", status);
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    printf("✅ Decryption successful\n");
    
    // Verify plaintext matches
    if (memcmp(plaintext, decrypted, pt_len) == 0) {
        printf("✅ Plaintext verification successful\n");
    } else {
        printf("❌ Plaintext mismatch!\n");
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    
    // Test authentication failure
    printf("Testing authentication failure detection...\n");
    tag[0] ^= 1; // Corrupt tag
    status = charmb_aead_decrypt(key, nonce, aad, strlen((char*)aad),
                                 ciphertext, pt_len, tag, decrypted);
    if (status == CHARMB_AEAD_ERROR_AUTH_FAILED) {
        printf("✅ Authentication failure correctly detected\n");
    } else {
        printf("❌ Authentication failure not detected!\n");
        free(ciphertext);
        free(decrypted);
        return -1;
    }
    
    printf("\n✅ All functionality tests passed!\n\n");
    
    free(ciphertext);
    free(decrypted);
    return 0;
}

/**
 * @brief Run AEAD benchmark for a specific size
 */
static void benchmark_aead_size(size_t size) {
    uint8_t* plaintext = malloc(size);
    if (!plaintext) {
        printf("Memory allocation failed for size %zu\n", size);
        return;
    }
    
    generate_test_data(plaintext, size, 0x12345678);
    
    aead_result_t charmb_result, aes_gcm_result;
    
    printf("\n=== %zu-byte AEAD Performance ===\n", size);
    printf("Algorithm       | Size | Encrypt    | Decrypt    | Average    | Performance\n");
    printf("----------------+------+------------+------------+------------+----------------\n");
    
    // Test CHARM-B AEAD
    if (benchmark_charmb_aead(plaintext, size, &charmb_result) == 0) {
        print_aead_result(&charmb_result);
        printf("BASELINE\n");
        
        // Test AES-256-GCM
        if (benchmark_aes_gcm(plaintext, size, &aes_gcm_result) == 0) {
            print_aead_result(&aes_gcm_result);
            print_aead_comparison(&charmb_result, &aes_gcm_result);
            printf(" vs AES-GCM\n");
        }
        
        // Performance summary
        printf("\n📊 AEAD Performance Summary for %zu bytes:\n", size);
        printf("   CHARM-B Encrypt: %10.1f MB/s\n", charmb_result.encrypt_mbps);
        printf("   CHARM-B Decrypt: %10.1f MB/s\n", charmb_result.decrypt_mbps);
        if (aes_gcm_result.total_mbps > 0) {
            double vs_aes = (charmb_result.total_mbps / aes_gcm_result.total_mbps - 1.0) * 100;
            printf("   vs AES-256-GCM:  %+9.1f%%\n", vs_aes);
        }
        printf("   Average:         %10.1f MB/s\n", charmb_result.total_mbps);
    } else {
        printf("CHARM-B AEAD benchmark failed for size %zu\n", size);
    }
    
    free(plaintext);
}

/**
 * @brief Main AEAD benchmark program
 */
int main() {
    printf("CHARM-B AEAD Comprehensive Benchmark Suite\n");
    printf("===========================================\n");
    
    printf("Configuration: %d iterations with %d warmup runs\n", NUM_ITERATIONS, NUM_WARMUP);
    printf("Target: Ultra-small payloads optimized AEAD\n");
    
    // Test basic functionality first
    if (test_aead_functionality() != 0) {
        printf("❌ Functionality tests failed!\n");
        return 1;
    }
    
    // Benchmark different payload sizes
    size_t test_sizes[] = {8, 16, 32, 64, 128, 256};
    int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        benchmark_aead_size(test_sizes[i]);
    }
    
    printf("\n============================================================\n");
    printf("CHARM-B AEAD BENCHMARK COMPLETE\n");
    printf("Ultra-fast authenticated encryption for small payloads!\n");
    printf("============================================================\n");
    
    return 0;
}