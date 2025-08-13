/**
 * @file avx2_detect.h
 * @brief AVX2 detection and SIMD capabilities for the CHARM system
 * 
 * This header defines the AVX2 detection and SIMD capability interfaces
 * for the CHARM system, providing runtime detection of CPU features.
 */

#ifndef AVX2_DETECT_H
#define AVX2_DETECT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief SIMD feature flags
 */
typedef enum {
    SIMD_FEATURE_NONE = 0,          /**< No SIMD features */
    SIMD_FEATURE_SSE = (1 << 0),     /**< SSE */
    SIMD_FEATURE_SSE2 = (1 << 1),    /**< SSE2 */
    SIMD_FEATURE_SSE3 = (1 << 2),    /**< SSE3 */
    SIMD_FEATURE_SSSE3 = (1 << 3),   /**< SSSE3 */
    SIMD_FEATURE_SSE41 = (1 << 4),   /**< SSE4.1 */
    SIMD_FEATURE_SSE42 = (1 << 5),   /**< SSE4.2 */
    SIMD_FEATURE_AVX = (1 << 6),     /**< AVX */
    SIMD_FEATURE_AVX2 = (1 << 7),    /**< AVX2 */
    SIMD_FEATURE_AVX512F = (1 << 8), /**< AVX-512 Foundation */
    SIMD_FEATURE_AVX512BW = (1 << 9), /**< AVX-512 Byte and Word */
    SIMD_FEATURE_AVX512DQ = (1 << 10), /**< AVX-512 Doubleword and Quadword */
    SIMD_FEATURE_AVX512VL = (1 << 11), /**< AVX-512 Vector Length Extensions */
    SIMD_FEATURE_FMA = (1 << 12),    /**< FMA */
    SIMD_FEATURE_BMI1 = (1 << 13),   /**< BMI1 */
    SIMD_FEATURE_BMI2 = (1 << 14),   /**< BMI2 */
    SIMD_FEATURE_POPCNT = (1 << 15), /**< POPCNT */
    SIMD_FEATURE_RDRAND = (1 << 16), /**< RDRAND */
    SIMD_FEATURE_RDSEED = (1 << 17)  /**< RDSEED */
} simd_feature_t;

/**
 * @brief SIMD architecture
 */
typedef enum {
    SIMD_ARCH_UNKNOWN = 0,  /**< Unknown architecture */
    SIMD_ARCH_X86,          /**< x86 architecture */
    SIMD_ARCH_X86_64,       /**< x86-64 architecture */
    SIMD_ARCH_ARM,          /**< ARM architecture */
    SIMD_ARCH_ARM64,        /**< ARM64 architecture */
    SIMD_ARCH_PPC,          /**< PowerPC architecture */
    SIMD_ARCH_PPC64,        /**< PowerPC64 architecture */
    SIMD_ARCH_MIPS,         /**< MIPS architecture */
    SIMD_ARCH_RISCV         /**< RISC-V architecture */
} simd_arch_t;

/**
 * @brief SIMD capabilities
 */
typedef struct {
    simd_arch_t arch;               /**< CPU architecture */
    uint32_t features;              /**< Feature flags */
    uint32_t vector_size;           /**< Vector size in bytes */
    uint32_t cache_line_size;       /**< Cache line size in bytes */
    char vendor[16];                /**< CPU vendor string */
    char brand[64];                 /**< CPU brand string */
    uint32_t family;                /**< CPU family */
    uint32_t model;                 /**< CPU model */
    uint32_t stepping;              /**< CPU stepping */
} simd_capabilities_t;

/**
 * @brief Check if AVX2 is supported
 * 
 * @return bool True if AVX2 is supported
 */
bool avx2_is_supported(void);

/**
 * @brief Check if AVX512F is supported
 * 
 * @return bool True if AVX512F is supported
 */
bool avx512_is_supported(void);

/**
 * @brief Check if NEON is supported
 * 
 * @return bool True if NEON is supported  
 */
bool neon_is_supported(void);

/**
 * @brief Detect SIMD capabilities
 * 
 * @param capabilities Capabilities structure to fill
 * @return bool True if detection succeeded
 */
bool simd_detect_capabilities(simd_capabilities_t* capabilities);

/**
 * @brief Check if a specific SIMD feature is supported
 * 
 * @param feature Feature to check
 * @return bool True if supported
 */
bool simd_is_feature_supported(simd_feature_t feature);

/**
 * @brief Get optimal SIMD implementation for current CPU
 * 
 * @return const char* Implementation name (e.g., "avx2", "neon", "fallback")
 */
const char* simd_get_optimal_implementation(void);

/**
 * @brief Get the best available SIMD feature (legacy)
 * 
 * @return const char* Name of the best available SIMD feature
 */
const char* get_best_simd_feature(void);

/**
 * @brief Print CPU feature information
 */
void print_cpu_features(void);

/**
 * @brief Get string representation of SIMD architecture
 * 
 * @param arch Architecture
 * @return const char* String representation
 */
const char* simd_arch_to_string(simd_arch_t arch);

/**
 * @brief Get string representation of SIMD feature
 * 
 * @param feature Feature
 * @return const char* String representation
 */
const char* simd_feature_to_string(simd_feature_t feature);

/**
 * @brief Print SIMD capabilities to stdout
 * 
 * @param capabilities Capabilities structure
 */
void simd_print_capabilities(const simd_capabilities_t* capabilities);

#ifdef __cplusplus
}
#endif

#endif /* AVX2_DETECT_H */
