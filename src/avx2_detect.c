/**
 * @file avx2_detect.c
 * @brief CPU feature detection for AVX2 and NEON
 * 
 * This module implements CPU feature detection for AVX2 on x86/x64 platforms
 * and NEON on ARM platforms, allowing the system to use optimized code paths
 * when available.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <cpuid.h>
#endif

/**
 * @brief Check if AVX2 is supported
 * 
 * @return bool true if AVX2 is supported, false otherwise
 */
bool avx2_is_supported(void) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    unsigned int eax, ebx, ecx, edx;
    
    // Check if CPUID supports extended features (leaf 7)
    if (__get_cpuid(0, &eax, &ebx, &ecx, &edx)) {
        if (eax >= 7) {
            // Get extended features
            __cpuid_count(7, 0, eax, ebx, ecx, edx);
            
            // Check AVX2 bit (bit 5 of EBX)
            return (ebx & (1 << 5)) != 0;
        }
    }
#endif
    
    return false;
}

/**
 * @brief Check if NEON is supported
 * 
 * @return bool true if NEON is supported, false otherwise
 */
bool neon_is_supported(void) {
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    // NEON is enabled at compile time on ARM platforms that support it
    return true;
#elif defined(__aarch64__)
    // All AArch64 (ARM64) processors support NEON
    return true;
#else
    // Not an ARM platform or NEON not enabled
    return false;
#endif
}

/**
 * @brief Get the best available SIMD feature
 * 
 * @return const char* Name of the best available SIMD feature
 */
const char* get_best_simd_feature(void) {
    if (avx2_is_supported()) {
        return "AVX2";
    } else if (neon_is_supported()) {
        return "NEON";
    } else {
        return "None";
    }
}

/**
 * @brief Print CPU feature information
 */
void print_cpu_features(void) {
    printf("CPU Features:\n");
    printf("  AVX2: %s\n", avx2_is_supported() ? "Supported" : "Not supported");
    printf("  NEON: %s\n", neon_is_supported() ? "Supported" : "Not supported");
    printf("  Best SIMD: %s\n", get_best_simd_feature());
}
