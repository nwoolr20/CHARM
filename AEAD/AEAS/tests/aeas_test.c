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
 * @file aeas_test.c
 * @brief Unit tests for CHARM-AEAS AEAD implementation
 */

#include "aeas.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Test vectors and helper functions
 */

static int test_count = 0;
static int test_passed = 0;

#define ASSERT(condition, message) do { \
    test_count++; \
    if (condition) { \
        test_passed++; \
        printf("✓ %s\n", message); \
    } else { \
        printf("✗ %s\n", message); \
    } \
} while(0)

/**
 * @brief Test basic encrypt/decrypt functionality
 */
static void test_basic_encrypt_decrypt(void) {
    printf("\n=== Basic Encrypt/Decrypt Test ===\n");
    
    uint8_t key[AEAS_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    uint8_t nonce[AEAS_NONCE_SIZE] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01
    };
    
    const char* plaintext = "Hello, CHARM-AEAS!";
    size_t pt_len = strlen(plaintext);
    
    uint8_t ciphertext[64];
    uint8_t tag[AEAS_TAG_SIZE];
    uint8_t decrypted[64];
    
    // Test encryption
    int result = aeas_encrypt(key, nonce, NULL, 0,
                              (const uint8_t*)plaintext, pt_len,
                              ciphertext, tag);
    ASSERT(result == AEAS_SUCCESS, "Encryption succeeds");
    
    // Test decryption
    result = aeas_decrypt(key, nonce, NULL, 0,
                          ciphertext, pt_len, tag, decrypted);
    ASSERT(result == AEAS_SUCCESS, "Decryption succeeds");
    
    // Verify plaintext
    ASSERT(memcmp(plaintext, decrypted, pt_len) == 0, "Decrypted plaintext matches original");
}

/**
 * @brief Test with additional authenticated data (AAD)
 */
static void test_aad_handling(void) {
    printf("\n=== AAD Handling Test ===\n");
    
    uint8_t key[AEAS_KEY_SIZE];
    uint8_t nonce[AEAS_NONCE_SIZE];
    
    // Fill key and nonce with test pattern
    for (int i = 0; i < AEAS_KEY_SIZE; i++) key[i] = i;
    for (int i = 0; i < AEAS_NONCE_SIZE; i++) nonce[i] = i;
    
    const char* plaintext = "Test message";
    const char* aad = "Additional authenticated data";
    size_t pt_len = strlen(plaintext);
    size_t aad_len = strlen(aad);
    
    uint8_t ciphertext[64];
    uint8_t tag[AEAS_TAG_SIZE];
    uint8_t decrypted[64];
    
    // Encrypt with AAD
    int result = aeas_encrypt(key, nonce, (const uint8_t*)aad, aad_len,
                              (const uint8_t*)plaintext, pt_len,
                              ciphertext, tag);
    ASSERT(result == AEAS_SUCCESS, "Encryption with AAD succeeds");
    
    // Decrypt with correct AAD
    result = aeas_decrypt(key, nonce, (const uint8_t*)aad, aad_len,
                          ciphertext, pt_len, tag, decrypted);
    ASSERT(result == AEAS_SUCCESS, "Decryption with correct AAD succeeds");
    
    // Verify plaintext
    ASSERT(memcmp(plaintext, decrypted, pt_len) == 0, "Decrypted plaintext matches with AAD");
    
    // Try with wrong AAD
    const char* wrong_aad = "Wrong AAD data";
    result = aeas_decrypt(key, nonce, (const uint8_t*)wrong_aad, strlen(wrong_aad),
                          ciphertext, pt_len, tag, decrypted);
    ASSERT(result == AEAS_ERROR_AUTH, "Decryption with wrong AAD fails");
}

/**
 * @brief Test tampering detection
 */
static void test_tampering_detection(void) {
    printf("\n=== Tampering Detection Test ===\n");
    
    uint8_t key[AEAS_KEY_SIZE];
    uint8_t nonce[AEAS_NONCE_SIZE];
    
    // Fill with pattern
    for (int i = 0; i < AEAS_KEY_SIZE; i++) key[i] = 0x42;
    for (int i = 0; i < AEAS_NONCE_SIZE; i++) nonce[i] = 0x24;
    
    const char* plaintext = "Important message";
    size_t pt_len = strlen(plaintext);
    
    uint8_t ciphertext[64];
    uint8_t tag[AEAS_TAG_SIZE];
    uint8_t decrypted[64];
    
    // Encrypt
    int result = aeas_encrypt(key, nonce, NULL, 0,
                              (const uint8_t*)plaintext, pt_len,
                              ciphertext, tag);
    ASSERT(result == AEAS_SUCCESS, "Encryption for tampering test succeeds");
    
    // Test with tampered ciphertext
    uint8_t tampered_ct[64];
    memcpy(tampered_ct, ciphertext, pt_len);
    tampered_ct[0] ^= 0x01;  // Flip one bit
    
    result = aeas_decrypt(key, nonce, NULL, 0,
                          tampered_ct, pt_len, tag, decrypted);
    ASSERT(result == AEAS_ERROR_AUTH, "Tampered ciphertext detected");
    
    // Test with tampered tag
    uint8_t tampered_tag[AEAS_TAG_SIZE];
    memcpy(tampered_tag, tag, AEAS_TAG_SIZE);
    tampered_tag[0] ^= 0x01;  // Flip one bit
    
    result = aeas_decrypt(key, nonce, NULL, 0,
                          ciphertext, pt_len, tampered_tag, decrypted);
    ASSERT(result == AEAS_ERROR_AUTH, "Tampered tag detected");
}

/**
 * @brief Test empty inputs
 */
static void test_empty_inputs(void) {
    printf("\n=== Empty Inputs Test ===\n");
    
    uint8_t key[AEAS_KEY_SIZE];
    uint8_t nonce[AEAS_NONCE_SIZE];
    uint8_t tag[AEAS_TAG_SIZE];
    
    // Fill with pattern
    for (int i = 0; i < AEAS_KEY_SIZE; i++) key[i] = i * 3;
    for (int i = 0; i < AEAS_NONCE_SIZE; i++) nonce[i] = i * 7;
    
    // Test empty plaintext
    int result = aeas_encrypt(key, nonce, NULL, 0, NULL, 0, NULL, tag);
    ASSERT(result == AEAS_SUCCESS, "Empty plaintext encryption succeeds");
    
    result = aeas_decrypt(key, nonce, NULL, 0, NULL, 0, tag, NULL);
    ASSERT(result == AEAS_SUCCESS, "Empty plaintext decryption succeeds");
}

/**
 * @brief Test nonce context helpers
 */
static void test_nonce_context(void) {
    printf("\n=== Nonce Context Test ===\n");
    
    aeas_nonce_ctx_t ctx;
    uint8_t prefix[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t nonce1[AEAS_NONCE_SIZE];
    uint8_t nonce2[AEAS_NONCE_SIZE];
    
    // Initialize context
    int result = aeas_nonce_init(&ctx, prefix);
    ASSERT(result == AEAS_SUCCESS, "Nonce context initialization succeeds");
    
    // Generate first nonce
    result = aeas_nonce_next(&ctx, nonce1);
    ASSERT(result == AEAS_SUCCESS, "First nonce generation succeeds");
    
    // Generate second nonce
    result = aeas_nonce_next(&ctx, nonce2);
    ASSERT(result == AEAS_SUCCESS, "Second nonce generation succeeds");
    
    // Verify nonces are different
    ASSERT(memcmp(nonce1, nonce2, AEAS_NONCE_SIZE) != 0, "Generated nonces are different");
    
    // Verify prefix is correct
    ASSERT(memcmp(nonce1, prefix, 4) == 0, "First nonce has correct prefix");
    ASSERT(memcmp(nonce2, prefix, 4) == 0, "Second nonce has correct prefix");
}

/**
 * @brief Test parameter validation
 */
static void test_parameter_validation(void) {
    printf("\n=== Parameter Validation Test ===\n");
    
    uint8_t key[AEAS_KEY_SIZE];
    uint8_t nonce[AEAS_NONCE_SIZE];
    uint8_t plaintext[16] = "test message";
    uint8_t ciphertext[16];
    uint8_t tag[AEAS_TAG_SIZE];
    
    // Test NULL key
    int result = aeas_encrypt(NULL, nonce, NULL, 0, plaintext, 12, ciphertext, tag);
    ASSERT(result == AEAS_ERROR_INVALID, "NULL key rejected");
    
    // Test NULL nonce
    result = aeas_encrypt(key, NULL, NULL, 0, plaintext, 12, ciphertext, tag);
    ASSERT(result == AEAS_ERROR_INVALID, "NULL nonce rejected");
    
    // Test NULL tag
    result = aeas_encrypt(key, nonce, NULL, 0, plaintext, 12, ciphertext, NULL);
    ASSERT(result == AEAS_ERROR_INVALID, "NULL tag rejected");
}

/**
 * @brief Write test results to file
 */
static void write_test_results(void) {
    FILE* fp = fopen("results/test_report.md", "w");
    if (fp) {
        fprintf(fp, "# CHARM-AEAS Test Report\n\n");
        fprintf(fp, "**Test Date:** %s\n", __DATE__);
        fprintf(fp, "**Test Time:** %s\n", __TIME__);
        fprintf(fp, "\n## Summary\n\n");
        fprintf(fp, "- **Total Tests:** %d\n", test_count);
        fprintf(fp, "- **Passed:** %d\n", test_passed);
        fprintf(fp, "- **Failed:** %d\n", test_count - test_passed);
        fprintf(fp, "- **Success Rate:** %.1f%%\n", (100.0 * test_passed) / test_count);
        fprintf(fp, "\n## Status\n\n");
        if (test_passed == test_count) {
            fprintf(fp, "✅ **ALL TESTS PASSED**\n");
        } else {
            fprintf(fp, "❌ **SOME TESTS FAILED**\n");
        }
        fclose(fp);
    }
    
    // JSON results
    fp = fopen("results/test_results.json", "w");
    if (fp) {
        fprintf(fp, "{\n");
        fprintf(fp, "  \"total_tests\": %d,\n", test_count);
        fprintf(fp, "  \"passed\": %d,\n", test_passed);
        fprintf(fp, "  \"failed\": %d,\n", test_count - test_passed);
        fprintf(fp, "  \"success_rate\": %.1f,\n", (100.0 * test_passed) / test_count);
        fprintf(fp, "  \"timestamp\": \"%s %s\"\n", __DATE__, __TIME__);
        fprintf(fp, "}\n");
        fclose(fp);
    }
}

/**
 * @brief Main test function
 */
int main(void) {
    printf("CHARM-AEAS Unit Tests\n");
    printf("====================\n");
    
    // Run all tests
    test_basic_encrypt_decrypt();
    test_aad_handling();
    test_tampering_detection();
    test_empty_inputs();
    test_nonce_context();
    test_parameter_validation();
    
    // Print summary
    printf("\n=== Test Summary ===\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", test_passed);
    printf("Failed: %d\n", test_count - test_passed);
    printf("Success rate: %.1f%%\n", (100.0 * test_passed) / test_count);
    
    // Write results
    write_test_results();
    
    if (test_passed == test_count) {
        printf("\n✅ ALL TESTS PASSED!\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED!\n");
        return 1;
    }
}