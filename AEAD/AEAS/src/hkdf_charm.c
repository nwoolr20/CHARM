/**
 * @file hkdf_charm.c
 * @brief HKDF implementation using HMAC-CHARM
 */

#include "hkdf_charm.h"
#include "hmac_charm.h"
#include "util.h"
#include <string.h>

/**
 * @brief HKDF-Extract: Extract a pseudorandom key from input keying material
 */
int hkdf_charm_extract(const uint8_t* salt, size_t salt_len,
                       const uint8_t* ikm, size_t ikm_len,
                       uint8_t prk[32]) {
    
    if (!ikm || !prk) return -1;
    
    // If salt is NULL or empty, use zero-filled salt of hash length
    uint8_t zero_salt[32] = {0};
    const uint8_t* actual_salt = salt;
    size_t actual_salt_len = salt_len;
    
    if (!salt || salt_len == 0) {
        actual_salt = zero_salt;
        actual_salt_len = sizeof(zero_salt);
    }
    
    // PRK = HMAC-CHARM(salt, IKM)
    return hmac_charm(actual_salt, actual_salt_len, ikm, ikm_len, prk);
}

/**
 * @brief HKDF-Expand: Expand a pseudorandom key to desired length
 */
int hkdf_charm_expand(const uint8_t prk[32],
                      const uint8_t* info, size_t info_len,
                      uint8_t* okm, size_t okm_len) {
    
    if (!prk || !okm) return -1;
    
    // Maximum output length is 255 * hash_length (32 bytes for CHARM-256)
    if (okm_len > 255 * 32) return -1;
    
    size_t hash_len = 32;  // HMAC-CHARM output size
    size_t n = (okm_len + hash_len - 1) / hash_len;  // Number of iterations needed
    
    uint8_t* output_ptr = okm;
    size_t remaining = okm_len;
    
    uint8_t t_prev[32] = {0};  // T(i-1), initially empty
    uint8_t t_curr[32];        // T(i)
    
    for (size_t i = 1; i <= n; i++) {
        hmac_charm_ctx_t ctx;
        
        // Initialize HMAC context with PRK as key
        if (hmac_charm_init(&ctx, prk, hash_len) != 0) {
            util_secure_clear(t_prev, sizeof(t_prev));
            util_secure_clear(t_curr, sizeof(t_curr));
            return -1;
        }
        
        // Update with T(i-1) if i > 1
        if (i > 1) {
            if (hmac_charm_update(&ctx, t_prev, hash_len) != 0) {
                hmac_charm_clear(&ctx);
                util_secure_clear(t_prev, sizeof(t_prev));
                util_secure_clear(t_curr, sizeof(t_curr));
                return -1;
            }
        }
        
        // Update with info if present
        if (info && info_len > 0) {
            if (hmac_charm_update(&ctx, info, info_len) != 0) {
                hmac_charm_clear(&ctx);
                util_secure_clear(t_prev, sizeof(t_prev));
                util_secure_clear(t_curr, sizeof(t_curr));
                return -1;
            }
        }
        
        // Update with counter byte
        uint8_t counter = (uint8_t)i;
        if (hmac_charm_update(&ctx, &counter, 1) != 0) {
            hmac_charm_clear(&ctx);
            util_secure_clear(t_prev, sizeof(t_prev));
            util_secure_clear(t_curr, sizeof(t_curr));
            return -1;
        }
        
        // Finalize to get T(i)
        if (hmac_charm_final(&ctx, t_curr) != 0) {
            hmac_charm_clear(&ctx);
            util_secure_clear(t_prev, sizeof(t_prev));
            util_secure_clear(t_curr, sizeof(t_curr));
            return -1;
        }
        
        hmac_charm_clear(&ctx);
        
        // Copy the needed bytes to output
        size_t copy_len = (remaining < hash_len) ? remaining : hash_len;
        memcpy(output_ptr, t_curr, copy_len);
        output_ptr += copy_len;
        remaining -= copy_len;
        
        // Save T(i) as T(i-1) for next iteration
        memcpy(t_prev, t_curr, hash_len);
    }
    
    // Clear sensitive data
    util_secure_clear(t_prev, sizeof(t_prev));
    util_secure_clear(t_curr, sizeof(t_curr));
    
    return 0;
}

/**
 * @brief HKDF: Combined extract and expand operation
 */
int hkdf_charm(const uint8_t* salt, size_t salt_len,
               const uint8_t* ikm, size_t ikm_len,
               const uint8_t* info, size_t info_len,
               uint8_t* okm, size_t okm_len) {
    
    if (!ikm || !okm) return -1;
    
    uint8_t prk[32];
    
    // Extract phase
    if (hkdf_charm_extract(salt, salt_len, ikm, ikm_len, prk) != 0) {
        util_secure_clear(prk, sizeof(prk));
        return -1;
    }
    
    // Expand phase
    int result = hkdf_charm_expand(prk, info, info_len, okm, okm_len);
    
    // Clear PRK
    util_secure_clear(prk, sizeof(prk));
    
    return result;
}