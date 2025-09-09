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
 * @file charm_aead_tests.c
 * @brief Comprehensive test suite for CHARM AEAD
 * 
 * Tests both regular nonce-based AEAD and misuse-resistant SIV variant
 */

#include "../include/charm_aead.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

// Test utilities
static void print_hex(const char* label, const uint8_t* data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

static int test_basic_aead() {
    printf("Testing basic CHARM AEAD encryption/decryption...\n");
    
    uint8_t key[CHARM_AEAD_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    uint8_t nonce[CHARM_AEAD_NONCE_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
    };
    
    const char* plaintext_str = "Hello, CHARM AEAD!";
    size_t plaintext_len = strlen(plaintext_str);
    const uint8_t* plaintext = (const uint8_t*)plaintext_str;
    
    const char* aad_str = "Additional data";
    size_t aad_len = strlen(aad_str);
    const uint8_t* aad = (const uint8_t*)aad_str;
    
    uint8_t* ciphertext = malloc(plaintext_len);
    uint8_t tag[CHARM_AEAD_TAG_SIZE];
    uint8_t* recovered = malloc(plaintext_len);
    
    if (!ciphertext || !recovered) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    // Test encryption
    charm_aead_status_t status = charm_aead_encrypt(
        key, nonce, aad, aad_len, plaintext, plaintext_len, ciphertext, tag
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("Encryption failed with status %d\n", status);
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    print_hex("Ciphertext", ciphertext, plaintext_len);
    print_hex("Tag", tag, CHARM_AEAD_TAG_SIZE);
    
    // Test decryption
    status = charm_aead_decrypt(
        key, nonce, aad, aad_len, ciphertext, plaintext_len, tag, recovered
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("Decryption failed with status %d\n", status);
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    // Verify recovered plaintext
    if (memcmp(plaintext, recovered, plaintext_len) != 0) {
        printf("Plaintext verification failed\n");
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    printf("✅ Basic AEAD test passed\n");
    free(ciphertext);
    free(recovered);
    return 1;
}

static int test_siv_deterministic() {
    printf("Testing CHARM AEAD SIV deterministic behavior...\n");
    
    uint8_t key[CHARM_AEAD_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    const char* plaintext_str = "SIV test message";
    size_t plaintext_len = strlen(plaintext_str);
    const uint8_t* plaintext = (const uint8_t*)plaintext_str;
    
    const char* aad_str = "SIV AAD";
    size_t aad_len = strlen(aad_str);
    const uint8_t* aad = (const uint8_t*)aad_str;
    
    uint8_t* ciphertext1 = malloc(plaintext_len);
    uint8_t tag1[CHARM_AEAD_TAG_SIZE];
    uint8_t* ciphertext2 = malloc(plaintext_len);
    uint8_t tag2[CHARM_AEAD_TAG_SIZE];
    uint8_t* recovered = malloc(plaintext_len);
    
    if (!ciphertext1 || !ciphertext2 || !recovered) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    // First encryption
    charm_aead_status_t status = charm_aead_siv_encrypt(
        key, aad, aad_len, plaintext, plaintext_len, ciphertext1, tag1
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("First SIV encryption failed with status %d\n", status);
        free(ciphertext1);
        free(ciphertext2);
        free(recovered);
        return 0;
    }
    
    // Second encryption (should be identical)
    status = charm_aead_siv_encrypt(
        key, aad, aad_len, plaintext, plaintext_len, ciphertext2, tag2
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("Second SIV encryption failed with status %d\n", status);
        free(ciphertext1);
        free(ciphertext2);
        free(recovered);
        return 0;
    }
    
    // Verify deterministic behavior
    if (memcmp(ciphertext1, ciphertext2, plaintext_len) != 0) {
        printf("SIV ciphertexts are not identical (not deterministic)\n");
        free(ciphertext1);
        free(ciphertext2);
        free(recovered);
        return 0;
    }
    
    if (memcmp(tag1, tag2, CHARM_AEAD_TAG_SIZE) != 0) {
        printf("SIV tags are not identical (not deterministic)\n");
        free(ciphertext1);
        free(ciphertext2);
        free(recovered);
        return 0;
    }
    
    print_hex("SIV Ciphertext", ciphertext1, plaintext_len);
    print_hex("SIV Tag", tag1, CHARM_AEAD_TAG_SIZE);
    
    // Test decryption
    status = charm_aead_siv_decrypt(
        key, aad, aad_len, ciphertext1, plaintext_len, tag1, recovered
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("SIV decryption failed with status %d\n", status);
        free(ciphertext1);
        free(ciphertext2);
        free(recovered);
        return 0;
    }
    
    // Verify recovered plaintext
    if (memcmp(plaintext, recovered, plaintext_len) != 0) {
        printf("SIV plaintext verification failed\n");
        free(ciphertext1);
        free(ciphertext2);
        free(recovered);
        return 0;
    }
    
    printf("✅ SIV deterministic test passed\n");
    free(ciphertext1);
    free(ciphertext2);
    free(recovered);
    return 1;
}

static int test_siv_different_inputs() {
    printf("Testing CHARM AEAD SIV with different inputs...\n");
    
    uint8_t key[CHARM_AEAD_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    const char* plaintext1_str = "Message 1";
    const char* plaintext2_str = "Message 2";
    size_t plaintext1_len = strlen(plaintext1_str);
    size_t plaintext2_len = strlen(plaintext2_str);
    const uint8_t* plaintext1 = (const uint8_t*)plaintext1_str;
    const uint8_t* plaintext2 = (const uint8_t*)plaintext2_str;
    
    const char* aad_str = "Common AAD";
    size_t aad_len = strlen(aad_str);
    const uint8_t* aad = (const uint8_t*)aad_str;
    
    uint8_t* ciphertext1 = malloc(plaintext1_len);
    uint8_t tag1[CHARM_AEAD_TAG_SIZE];
    uint8_t* ciphertext2 = malloc(plaintext2_len);
    uint8_t tag2[CHARM_AEAD_TAG_SIZE];
    
    if (!ciphertext1 || !ciphertext2) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    // Encrypt first message
    charm_aead_status_t status = charm_aead_siv_encrypt(
        key, aad, aad_len, plaintext1, plaintext1_len, ciphertext1, tag1
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("First message SIV encryption failed with status %d\n", status);
        free(ciphertext1);
        free(ciphertext2);
        return 0;
    }
    
    // Encrypt second message
    status = charm_aead_siv_encrypt(
        key, aad, aad_len, plaintext2, plaintext2_len, ciphertext2, tag2
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("Second message SIV encryption failed with status %d\n", status);
        free(ciphertext1);
        free(ciphertext2);
        return 0;
    }
    
    // Verify that different messages produce different outputs
    if (plaintext1_len == plaintext2_len && 
        memcmp(ciphertext1, ciphertext2, plaintext1_len) == 0) {
        printf("Different plaintexts produced identical ciphertexts\n");
        free(ciphertext1);
        free(ciphertext2);
        return 0;
    }
    
    if (memcmp(tag1, tag2, CHARM_AEAD_TAG_SIZE) == 0) {
        printf("Different plaintexts produced identical tags\n");
        print_hex("Plaintext1", plaintext1, plaintext1_len);
        print_hex("Plaintext2", plaintext2, plaintext2_len);
        print_hex("Tag1", tag1, CHARM_AEAD_TAG_SIZE);
        print_hex("Tag2", tag2, CHARM_AEAD_TAG_SIZE);
        free(ciphertext1);
        free(ciphertext2);
        return 0;
    }
    
    printf("✅ SIV different inputs test passed\n");
    free(ciphertext1);
    free(ciphertext2);
    return 1;
}

static int test_authentication_failure() {
    printf("Testing CHARM AEAD authentication failure detection...\n");
    
    uint8_t key[CHARM_AEAD_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    const char* plaintext_str = "Authentication test";
    size_t plaintext_len = strlen(plaintext_str);
    const uint8_t* plaintext = (const uint8_t*)plaintext_str;
    
    const char* aad_str = "Auth AAD";
    size_t aad_len = strlen(aad_str);
    const uint8_t* aad = (const uint8_t*)aad_str;
    
    uint8_t* ciphertext = malloc(plaintext_len);
    uint8_t tag[CHARM_AEAD_TAG_SIZE];
    uint8_t* recovered = malloc(plaintext_len);
    
    if (!ciphertext || !recovered) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    // Encrypt with SIV mode
    charm_aead_status_t status = charm_aead_siv_encrypt(
        key, aad, aad_len, plaintext, plaintext_len, ciphertext, tag
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("SIV encryption failed with status %d\n", status);
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    // Corrupt the tag
    tag[0] ^= 0x01;
    
    // Try to decrypt (should fail)
    status = charm_aead_siv_decrypt(
        key, aad, aad_len, ciphertext, plaintext_len, tag, recovered
    );
    
    if (status != CHARM_AEAD_ERROR_AUTH_FAILED) {
        printf("Expected authentication failure, got status %d\n", status);
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    printf("✅ Authentication failure test passed\n");
    free(ciphertext);
    free(recovered);
    return 1;
}

static int test_no_aad() {
    printf("Testing CHARM AEAD SIV with no AAD...\n");
    
    uint8_t key[CHARM_AEAD_KEY_SIZE] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    
    const char* plaintext_str = "No AAD test";
    size_t plaintext_len = strlen(plaintext_str);
    const uint8_t* plaintext = (const uint8_t*)plaintext_str;
    
    uint8_t* ciphertext = malloc(plaintext_len);
    uint8_t tag[CHARM_AEAD_TAG_SIZE];
    uint8_t* recovered = malloc(plaintext_len);
    
    if (!ciphertext || !recovered) {
        printf("Memory allocation failed\n");
        return 0;
    }
    
    // Test encryption with no AAD
    charm_aead_status_t status = charm_aead_siv_encrypt(
        key, NULL, 0, plaintext, plaintext_len, ciphertext, tag
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("SIV encryption with no AAD failed with status %d\n", status);
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    // Test decryption with no AAD
    status = charm_aead_siv_decrypt(
        key, NULL, 0, ciphertext, plaintext_len, tag, recovered
    );
    
    if (status != CHARM_AEAD_SUCCESS) {
        printf("SIV decryption with no AAD failed with status %d\n", status);
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    // Verify recovered plaintext
    if (memcmp(plaintext, recovered, plaintext_len) != 0) {
        printf("No AAD plaintext verification failed\n");
        free(ciphertext);
        free(recovered);
        return 0;
    }
    
    printf("✅ No AAD test passed\n");
    free(ciphertext);
    free(recovered);
    return 1;
}

static int test_performance() {
    printf("Testing CHARM AEAD performance...\n");
    
    const size_t test_sizes[] = {16, 64, 256, 1024, 4096};
    const int num_sizes = sizeof(test_sizes) / sizeof(test_sizes[0]);
    const int iterations = 1000;
    
    printf("\n=== CHARM AEAD Performance Results ===\n");
    printf("%-10s %-15s %-15s %-15s %-15s\n", 
           "Size", "Regular Enc", "Regular Dec", "SIV Enc", "SIV Dec");
    printf("%-10s %-15s %-15s %-15s %-15s\n", 
           "(bytes)", "(MB/s)", "(MB/s)", "(MB/s)", "(MB/s)");
    printf("--------------------------------------------------------\n");
    
    for (int i = 0; i < num_sizes; i++) {
        double reg_enc_mbps, reg_dec_mbps, siv_enc_mbps, siv_dec_mbps;
        
        // Test regular mode
        charm_aead_status_t status = charm_aead_benchmark(
            false, test_sizes[i], iterations, &reg_enc_mbps, &reg_dec_mbps
        );
        if (status != CHARM_AEAD_SUCCESS) {
            printf("Regular mode benchmark failed for size %zu\n", test_sizes[i]);
            continue;
        }
        
        // Test SIV mode
        status = charm_aead_benchmark(
            true, test_sizes[i], iterations, &siv_enc_mbps, &siv_dec_mbps
        );
        if (status != CHARM_AEAD_SUCCESS) {
            printf("SIV mode benchmark failed for size %zu\n", test_sizes[i]);
            continue;
        }
        
        printf("%-10zu %-15.2f %-15.2f %-15.2f %-15.2f\n",
               test_sizes[i], reg_enc_mbps, reg_dec_mbps, siv_enc_mbps, siv_dec_mbps);
    }
    
    printf("\n✅ Performance test completed\n");
    return 1;
}

int main() {
    printf("CHARM AEAD Test Suite\n");
    printf("=====================\n\n");
    
    int tests_passed = 0;
    int total_tests = 6;
    
    if (test_basic_aead()) tests_passed++;
    printf("\n");
    
    if (test_siv_deterministic()) tests_passed++;
    printf("\n");
    
    if (test_siv_different_inputs()) tests_passed++;
    printf("\n");
    
    if (test_authentication_failure()) tests_passed++;
    printf("\n");
    
    if (test_no_aad()) tests_passed++;
    printf("\n");
    
    if (test_performance()) tests_passed++;
    printf("\n");
    
    printf("=== Test Summary ===\n");
    printf("Tests passed: %d/%d\n", tests_passed, total_tests);
    
    if (tests_passed == total_tests) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed\n");
        return 1;
    }
}