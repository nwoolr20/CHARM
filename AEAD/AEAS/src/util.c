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
 * @file util.c
 * @brief Utility functions implementation for AEAS
 */

#include "util.h"
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h>
#else
#include <cpuid.h>
#endif

/**
 * @brief Constant-time memory comparison
 */
int util_ct_memcmp(const void* a, const void* b, size_t len) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    uint8_t result = 0;
    
    for (size_t i = 0; i < len; i++) {
        result |= pa[i] ^ pb[i];
    }
    
    return result;
}

/**
 * @brief Secure memory clear
 */
void util_secure_clear(void* ptr, size_t len) {
    volatile uint8_t* p = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < len; i++) {
        p[i] = 0;
    }
}

/**
 * @brief Little-endian 32-bit integer encoding
 */
void util_le32_encode(uint32_t value, uint8_t out[4]) {
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
    out[2] = (uint8_t)((value >> 16) & 0xFF);
    out[3] = (uint8_t)((value >> 24) & 0xFF);
}

/**
 * @brief Little-endian 64-bit integer encoding
 */
void util_le64_encode(uint64_t value, uint8_t out[8]) {
    out[0] = (uint8_t)(value & 0xFF);
    out[1] = (uint8_t)((value >> 8) & 0xFF);
    out[2] = (uint8_t)((value >> 16) & 0xFF);
    out[3] = (uint8_t)((value >> 24) & 0xFF);
    out[4] = (uint8_t)((value >> 32) & 0xFF);
    out[5] = (uint8_t)((value >> 40) & 0xFF);
    out[6] = (uint8_t)((value >> 48) & 0xFF);
    out[7] = (uint8_t)((value >> 56) & 0xFF);
}

/**
 * @brief Little-endian 32-bit integer decoding
 */
uint32_t util_le32_decode(const uint8_t in[4]) {
    return ((uint32_t)in[0]) |
           (((uint32_t)in[1]) << 8) |
           (((uint32_t)in[2]) << 16) |
           (((uint32_t)in[3]) << 24);
}

/**
 * @brief Little-endian 64-bit integer decoding
 */
uint64_t util_le64_decode(const uint8_t in[8]) {
    return ((uint64_t)in[0]) |
           (((uint64_t)in[1]) << 8) |
           (((uint64_t)in[2]) << 16) |
           (((uint64_t)in[3]) << 24) |
           (((uint64_t)in[4]) << 32) |
           (((uint64_t)in[5]) << 40) |
           (((uint64_t)in[6]) << 48) |
           (((uint64_t)in[7]) << 56);
}

/**
 * @brief Detect CPU features using CPUID
 */
uint32_t util_cpu_features(void) {
    uint32_t features = 0;
    
#ifdef _MSC_VER
    int cpuid_info[4];
    __cpuid(cpuid_info, 1);
    if (cpuid_info[2] & (1 << 25)) features |= UTIL_CPU_AES_NI;
    
    __cpuid(cpuid_info, 7);
    if (cpuid_info[1] & (1 << 5)) features |= UTIL_CPU_AVX2;
#else
    uint32_t eax, ebx, ecx, edx;
    
    // Check for AES-NI (CPUID.01H:ECX.AES[bit 25])
    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        if (ecx & (1U << 25)) features |= UTIL_CPU_AES_NI;
    }
    
    // Check for AVX2 (CPUID.07H:EBX.AVX2[bit 5])
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        if (ebx & (1U << 5)) features |= UTIL_CPU_AVX2;
    }
#endif
    
    return features;
}

/**
 * @brief Check if AVX2 is supported
 */
int util_has_avx2(void) {
    return (util_cpu_features() & UTIL_CPU_AVX2) ? 1 : 0;
}

/**
 * @brief Validate nonce parameters
 */
int util_validate_nonce(const uint8_t nonce[12]) {
    if (!nonce) return -1;
    // Nonce validation could check for known weak nonces, but for now just check non-NULL
    return 0;
}

/**
 * @brief Convert bytes to hex string
 */
void util_bytes_to_hex(const uint8_t* data, size_t data_len, char* hex_out) {
    static const char hex_chars[] = "0123456789abcdef";
    
    for (size_t i = 0; i < data_len; i++) {
        hex_out[i * 2] = hex_chars[data[i] >> 4];
        hex_out[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    hex_out[data_len * 2] = '\0';
}