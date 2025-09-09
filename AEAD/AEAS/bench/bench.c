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
 * @file bench.c
 * @brief Benchmarks CHARM-AEAS vs OpenSSL AES-256-GCM and ChaCha20-Poly1305
 */

#include "aeas.h"
#include "util.h"
#include "openssl_helpers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#ifdef WITH_OPENSSL
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/crypto.h>
#endif

/**
 * @brief Benchmark configuration
 */
typedef struct {
    size_t data_size;
    size_t aad_size;
    int iterations;
    const char* size_name;
} bench_config_t;

/**
 * @brief Benchmark results for one test
 */
typedef struct {
    const char* algorithm;
    size_t data_size;
    size_t aad_size;
    double throughput_mbps;
    double avg_time_us;
    int iterations;
} bench_result_t;

/**
 * @brief Get current time in microseconds
 */
static double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

/**
 * @brief Benchmark CHARM-AEAS
 */
static double benchmark_charm_aeas(const uint8_t* key, const uint8_t* nonce,
                                   const uint8_t* aad, size_t aad_len,
                                   const uint8_t* plaintext, size_t pt_len,
                                   int iterations) {
    uint8_t* ciphertext = malloc(pt_len);
    uint8_t* decrypted = malloc(pt_len);
    uint8_t tag[AEAS_TAG_SIZE];
    
    if (!ciphertext || !decrypted) {
        free(ciphertext);
        free(decrypted);
        return -1.0;
    }
    
    double start_time = get_time_us();
    
    for (int i = 0; i < iterations; i++) {
        // Encrypt
        if (aeas_encrypt(key, nonce, aad, aad_len, plaintext, pt_len, ciphertext, tag) != 0) {
            free(ciphertext);
            free(decrypted);
            return -1.0;
        }
        
        // Decrypt
        if (aeas_decrypt(key, nonce, aad, aad_len, ciphertext, pt_len, tag, decrypted) != 0) {
            free(ciphertext);
            free(decrypted);
            return -1.0;
        }
    }
    
    double end_time = get_time_us();
    
    free(ciphertext);
    free(decrypted);
    
    return (end_time - start_time) / iterations;
}

#ifdef WITH_OPENSSL

/**
 * @brief OpenSSL AES-256-GCM implementation
 */
int openssl_aes_gcm_encrypt(const uint8_t key[32],
                            const uint8_t nonce[12],
                            const uint8_t* aad, size_t aad_len,
                            const uint8_t* plaintext, size_t pt_len,
                            uint8_t* ciphertext,
                            uint8_t tag[16]) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len, ciphertext_len;
    
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) goto error;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL) != 1) goto error;
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) != 1) goto error;
    
    if (aad && aad_len > 0) {
        if (EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len) != 1) goto error;
    }
    
    if (pt_len > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, pt_len) != 1) goto error;
        ciphertext_len = len;
        if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) goto error;
        ciphertext_len += len;
    }
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1) goto error;
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
    
error:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/**
 * @brief OpenSSL AES-256-GCM decryption
 */
int openssl_aes_gcm_decrypt(const uint8_t key[32],
                            const uint8_t nonce[12],
                            const uint8_t* aad, size_t aad_len,
                            const uint8_t* ciphertext, size_t ct_len,
                            const uint8_t tag[16],
                            uint8_t* plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len, plaintext_len;
    
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) goto error;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, NULL) != 1) goto error;
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce) != 1) goto error;
    
    if (aad && aad_len > 0) {
        if (EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len) != 1) goto error;
    }
    
    if (ct_len > 0) {
        if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ct_len) != 1) goto error;
        plaintext_len = len;
    }
    
    uint8_t tag_copy[16];
    memcpy(tag_copy, tag, 16);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag_copy) != 1) goto error;
    
    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    
    EVP_CIPHER_CTX_free(ctx);
    return (ret > 0) ? 0 : -1;
    
error:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/**
 * @brief OpenSSL ChaCha20-Poly1305 encryption  
 */
int openssl_chacha20_poly1305_encrypt(const uint8_t key[32],
                                       const uint8_t nonce[12],
                                       const uint8_t* aad, size_t aad_len,
                                       const uint8_t* plaintext, size_t pt_len,
                                       uint8_t* ciphertext,
                                       uint8_t tag[16]) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len, ciphertext_len;
    
    if (EVP_EncryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL) != 1) goto error;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL) != 1) goto error;
    if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, nonce) != 1) goto error;
    
    if (aad && aad_len > 0) {
        if (EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len) != 1) goto error;
    }
    
    if (pt_len > 0) {
        if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, pt_len) != 1) goto error;
        ciphertext_len = len;
        if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) goto error;
        ciphertext_len += len;
    }
    
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, 16, tag) != 1) goto error;
    
    EVP_CIPHER_CTX_free(ctx);
    return 0;
    
error:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/**
 * @brief OpenSSL ChaCha20-Poly1305 decryption
 */
int openssl_chacha20_poly1305_decrypt(const uint8_t key[32],
                                       const uint8_t nonce[12],
                                       const uint8_t* aad, size_t aad_len,
                                       const uint8_t* ciphertext, size_t ct_len,
                                       const uint8_t tag[16],
                                       uint8_t* plaintext) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return -1;
    
    int len, plaintext_len;
    
    if (EVP_DecryptInit_ex(ctx, EVP_chacha20_poly1305(), NULL, NULL, NULL) != 1) goto error;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, 12, NULL) != 1) goto error;
    if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, nonce) != 1) goto error;
    
    if (aad && aad_len > 0) {
        if (EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len) != 1) goto error;
    }
    
    if (ct_len > 0) {
        if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ct_len) != 1) goto error;
        plaintext_len = len;
    }
    
    uint8_t tag_copy[16];
    memcpy(tag_copy, tag, 16);
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, 16, tag_copy) != 1) goto error;
    
    int ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);
    
    EVP_CIPHER_CTX_free(ctx);
    return (ret > 0) ? 0 : -1;
    
error:
    EVP_CIPHER_CTX_free(ctx);
    return -1;
}

/**
 * @brief Benchmark OpenSSL AES-256-GCM
 */
static double benchmark_aes_gcm(const uint8_t* key, const uint8_t* nonce,
                                const uint8_t* aad, size_t aad_len,
                                const uint8_t* plaintext, size_t pt_len,
                                int iterations) {
    uint8_t* ciphertext = malloc(pt_len);
    uint8_t* decrypted = malloc(pt_len);
    uint8_t tag[16];
    
    if (!ciphertext || !decrypted) {
        free(ciphertext);
        free(decrypted);
        return -1.0;
    }
    
    double start_time = get_time_us();
    
    for (int i = 0; i < iterations; i++) {
        // Encrypt
        if (openssl_aes_gcm_encrypt(key, nonce, aad, aad_len, plaintext, pt_len, ciphertext, tag) != 0) {
            free(ciphertext);
            free(decrypted);
            return -1.0;
        }
        
        // Decrypt
        if (openssl_aes_gcm_decrypt(key, nonce, aad, aad_len, ciphertext, pt_len, tag, decrypted) != 0) {
            free(ciphertext);
            free(decrypted);
            return -1.0;
        }
    }
    
    double end_time = get_time_us();
    
    free(ciphertext);
    free(decrypted);
    
    return (end_time - start_time) / iterations;
}

/**
 * @brief Benchmark OpenSSL ChaCha20-Poly1305
 */
static double benchmark_chacha20_poly1305(const uint8_t* key, const uint8_t* nonce,
                                           const uint8_t* aad, size_t aad_len,
                                           const uint8_t* plaintext, size_t pt_len,
                                           int iterations) {
    uint8_t* ciphertext = malloc(pt_len);
    uint8_t* decrypted = malloc(pt_len);
    uint8_t tag[16];
    
    if (!ciphertext || !decrypted) {
        free(ciphertext);
        free(decrypted);
        return -1.0;
    }
    
    double start_time = get_time_us();
    
    for (int i = 0; i < iterations; i++) {
        // Encrypt
        if (openssl_chacha20_poly1305_encrypt(key, nonce, aad, aad_len, plaintext, pt_len, ciphertext, tag) != 0) {
            free(ciphertext);
            free(decrypted);
            return -1.0;
        }
        
        // Decrypt
        if (openssl_chacha20_poly1305_decrypt(key, nonce, aad, aad_len, ciphertext, pt_len, tag, decrypted) != 0) {
            free(ciphertext);
            free(decrypted);
            return -1.0;
        }
    }
    
    double end_time = get_time_us();
    
    free(ciphertext);
    free(decrypted);
    
    return (end_time - start_time) / iterations;
}

#endif /* WITH_OPENSSL */

/**
 * @brief Run benchmark for specific configuration
 */
static void run_benchmark_set(const bench_config_t* config, bench_result_t* results, int* result_count) {
    printf("Benchmarking %s (AAD: %zu bytes)...\n", config->size_name, config->aad_size);
    
    // Prepare test data
    uint8_t key[32];
    uint8_t nonce[12];
    uint8_t* plaintext = malloc(config->data_size);
    uint8_t* aad = malloc(config->aad_size);
    
    if (!plaintext || (!aad && config->aad_size > 0)) {
        printf("Memory allocation failed\n");
        free(plaintext);
        free(aad);
        return;
    }
    
    // Fill with test pattern
    for (size_t i = 0; i < 32; i++) key[i] = i;
    for (size_t i = 0; i < 12; i++) nonce[i] = i;
    for (size_t i = 0; i < config->data_size; i++) plaintext[i] = i & 0xFF;
    if (config->aad_size > 0) {
        for (size_t i = 0; i < config->aad_size; i++) aad[i] = (i + 128) & 0xFF;
    }
    
    // Benchmark CHARM-AEAS
    double charm_time = benchmark_charm_aeas(key, nonce, aad, config->aad_size, 
                                            plaintext, config->data_size, config->iterations);
    if (charm_time > 0) {
        double throughput = (config->data_size * 2.0 * config->iterations) / (charm_time * 1e-6) / (1024 * 1024);
        results[*result_count] = (bench_result_t){
            .algorithm = "CHARM-AEAS",
            .data_size = config->data_size,
            .aad_size = config->aad_size,
            .throughput_mbps = throughput,
            .avg_time_us = charm_time,
            .iterations = config->iterations
        };
        (*result_count)++;
        printf("  CHARM-AEAS: %.2f MB/s (%.2f μs/op)\n", throughput, charm_time);
    } else {
        printf("  CHARM-AEAS: FAILED\n");
    }
    
#ifdef WITH_OPENSSL
    // Benchmark AES-256-GCM
    double aes_time = benchmark_aes_gcm(key, nonce, aad, config->aad_size,
                                       plaintext, config->data_size, config->iterations);
    if (aes_time > 0) {
        double throughput = (config->data_size * 2.0 * config->iterations) / (aes_time * 1e-6) / (1024 * 1024);
        results[*result_count] = (bench_result_t){
            .algorithm = "AES-256-GCM",
            .data_size = config->data_size,
            .aad_size = config->aad_size,
            .throughput_mbps = throughput,
            .avg_time_us = aes_time,
            .iterations = config->iterations
        };
        (*result_count)++;
        printf("  AES-256-GCM: %.2f MB/s (%.2f μs/op)\n", throughput, aes_time);
    } else {
        printf("  AES-256-GCM: FAILED\n");
    }
    
    // Benchmark ChaCha20-Poly1305
    double chacha_time = benchmark_chacha20_poly1305(key, nonce, aad, config->aad_size,
                                                    plaintext, config->data_size, config->iterations);
    if (chacha_time > 0) {
        double throughput = (config->data_size * 2.0 * config->iterations) / (chacha_time * 1e-6) / (1024 * 1024);
        results[*result_count] = (bench_result_t){
            .algorithm = "ChaCha20-Poly1305",
            .data_size = config->data_size,
            .aad_size = config->aad_size,
            .throughput_mbps = throughput,
            .avg_time_us = chacha_time,
            .iterations = config->iterations
        };
        (*result_count)++;
        printf("  ChaCha20-Poly1305: %.2f MB/s (%.2f μs/op)\n", throughput, chacha_time);
    } else {
        printf("  ChaCha20-Poly1305: FAILED\n");
    }
#else
    printf("  OpenSSL not available - skipping AES-GCM and ChaCha20-Poly1305\n");
#endif
    
    free(plaintext);
    free(aad);
}

/**
 * @brief Write benchmark results to files
 */
static void write_benchmark_results(const bench_result_t* results, int result_count) {
    // JSON results
    FILE* fp = fopen("results/bench_results.json", "w");
    if (fp) {
        fprintf(fp, "{\n");
        fprintf(fp, "  \"benchmark_date\": \"%s\",\n", __DATE__);
        fprintf(fp, "  \"benchmark_time\": \"%s\",\n", __TIME__);
#ifdef WITH_OPENSSL
        fprintf(fp, "  \"openssl_version\": \"%s\",\n", OPENSSL_VERSION_TEXT);
#else
        fprintf(fp, "  \"openssl_version\": \"Not available\",\n");
#endif
        fprintf(fp, "  \"results\": [\n");
        
        for (int i = 0; i < result_count; i++) {
            const bench_result_t* r = &results[i];
            fprintf(fp, "    {\n");
            fprintf(fp, "      \"algorithm\": \"%s\",\n", r->algorithm);
            fprintf(fp, "      \"data_size\": %zu,\n", r->data_size);
            fprintf(fp, "      \"aad_size\": %zu,\n", r->aad_size);
            fprintf(fp, "      \"throughput_mbps\": %.2f,\n", r->throughput_mbps);
            fprintf(fp, "      \"avg_time_us\": %.2f,\n", r->avg_time_us);
            fprintf(fp, "      \"iterations\": %d\n", r->iterations);
            fprintf(fp, "    }%s\n", (i < result_count - 1) ? "," : "");
        }
        
        fprintf(fp, "  ]\n");
        fprintf(fp, "}\n");
        fclose(fp);
    }
    
    // Markdown summary
    fp = fopen("results/bench_summary.md", "w");
    if (fp) {
        fprintf(fp, "# CHARM-AEAS Benchmark Results\n\n");
        fprintf(fp, "**Benchmark Date:** %s %s\n\n", __DATE__, __TIME__);
        
#ifdef WITH_OPENSSL
        fprintf(fp, "**OpenSSL Version:** %s\n\n", OPENSSL_VERSION_TEXT);
        fprintf(fp, "## Performance Comparison\n\n");
        fprintf(fp, "| Data Size | AAD Size | Algorithm | Throughput (MB/s) | Avg Time (μs) |\n");
        fprintf(fp, "|-----------|----------|-----------|-------------------|---------------|\n");
        
        for (int i = 0; i < result_count; i++) {
            const bench_result_t* r = &results[i];
            const char* size_str;
            if (r->data_size < 1024) {
                fprintf(fp, "| %zu B", r->data_size);
            } else if (r->data_size < 1024 * 1024) {
                fprintf(fp, "| %zu KB", r->data_size / 1024);
            } else {
                fprintf(fp, "| %zu MB", r->data_size / (1024 * 1024));
            }
            
            fprintf(fp, " | %zu B | %s | %.2f | %.2f |\n",
                   r->aad_size, r->algorithm, r->throughput_mbps, r->avg_time_us);
        }
#else
        fprintf(fp, "**OpenSSL:** Not available - only CHARM-AEAS results shown\n\n");
        fprintf(fp, "## CHARM-AEAS Performance\n\n");
        fprintf(fp, "| Data Size | AAD Size | Throughput (MB/s) | Avg Time (μs) |\n");
        fprintf(fp, "|-----------|----------|-------------------|---------------|\n");
        
        for (int i = 0; i < result_count; i++) {
            const bench_result_t* r = &results[i];
            if (strcmp(r->algorithm, "CHARM-AEAS") == 0) {
                if (r->data_size < 1024) {
                    fprintf(fp, "| %zu B", r->data_size);
                } else if (r->data_size < 1024 * 1024) {
                    fprintf(fp, "| %zu KB", r->data_size / 1024);
                } else {
                    fprintf(fp, "| %zu MB", r->data_size / (1024 * 1024));
                }
                
                fprintf(fp, " | %zu B | %.2f | %.2f |\n",
                       r->aad_size, r->throughput_mbps, r->avg_time_us);
            }
        }
#endif
        
        fclose(fp);
    }
}

/**
 * @brief Main benchmark function
 */
int main(void) {
    printf("CHARM-AEAS Benchmark Suite\n");
    printf("==========================\n");
    
#ifdef WITH_OPENSSL
    printf("OpenSSL version: %s\n", OPENSSL_VERSION_TEXT);
#else
    printf("OpenSSL: Not available\n");
#endif
    printf("\n");
    
    // Benchmark configurations
    bench_config_t configs[] = {
        {64, 0, 10000, "64B"},
        {64, 16, 10000, "64B"},
        {256, 0, 5000, "256B"},
        {256, 16, 5000, "256B"},
        {1024, 0, 2000, "1KB"},
        {1024, 64, 2000, "1KB"},
        {4096, 0, 1000, "4KB"},
        {4096, 64, 1000, "4KB"},
        {16384, 0, 500, "16KB"},
        {16384, 64, 500, "16KB"},
        {65536, 0, 200, "64KB"},
        {65536, 64, 200, "64KB"},
        {1048576, 0, 50, "1MB"},
        {1048576, 64, 50, "1MB"}
    };
    
    int num_configs = sizeof(configs) / sizeof(configs[0]);
    bench_result_t results[64];  // Enough space for all results
    int result_count = 0;
    
    // Run benchmarks
    for (int i = 0; i < num_configs; i++) {
        run_benchmark_set(&configs[i], results, &result_count);
        printf("\n");
    }
    
    // Write results
    write_benchmark_results(results, result_count);
    
    printf("Benchmark complete. Results saved to results/\n");
    return 0;
}