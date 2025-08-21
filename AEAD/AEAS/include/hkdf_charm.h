/**
 * @file hkdf_charm.h
 * @brief HKDF (HMAC-based Key Derivation Function) using HMAC-CHARM
 * 
 * Implements HKDF as defined in RFC 5869 using HMAC-CHARM as the underlying PRF.
 */

#ifndef HKDF_CHARM_H
#define HKDF_CHARM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief HKDF-Extract: Extract a pseudorandom key from input keying material
 * 
 * PRK = HMAC-CHARM(salt, IKM)
 * 
 * @param salt Salt value (if NULL, uses zero-filled salt)
 * @param salt_len Length of salt in bytes
 * @param ikm Input keying material
 * @param ikm_len Length of IKM in bytes
 * @param prk Output pseudorandom key (32 bytes)
 * @return 0 on success, negative on error
 */
int hkdf_charm_extract(const uint8_t* salt, size_t salt_len,
                       const uint8_t* ikm, size_t ikm_len,
                       uint8_t prk[32]);

/**
 * @brief HKDF-Expand: Expand a pseudorandom key to desired length
 * 
 * OKM = HKDF-Expand(PRK, info, L)
 * 
 * @param prk Pseudorandom key from extract step (32 bytes)
 * @param info Optional context and application specific information
 * @param info_len Length of info in bytes
 * @param okm Output keying material
 * @param okm_len Desired length of OKM in bytes (max 8160 = 255 * 32)
 * @return 0 on success, negative on error
 */
int hkdf_charm_expand(const uint8_t prk[32],
                      const uint8_t* info, size_t info_len,
                      uint8_t* okm, size_t okm_len);

/**
 * @brief HKDF: Combined extract and expand operation
 * 
 * Convenience function that performs both extract and expand in one call.
 * 
 * @param salt Salt value (if NULL, uses zero-filled salt)
 * @param salt_len Length of salt in bytes
 * @param ikm Input keying material
 * @param ikm_len Length of IKM in bytes
 * @param info Optional context and application specific information
 * @param info_len Length of info in bytes
 * @param okm Output keying material
 * @param okm_len Desired length of OKM in bytes (max 8160 = 255 * 32)
 * @return 0 on success, negative on error
 */
int hkdf_charm(const uint8_t* salt, size_t salt_len,
               const uint8_t* ikm, size_t ikm_len,
               const uint8_t* info, size_t info_len,
               uint8_t* okm, size_t okm_len);

#ifdef __cplusplus
}
#endif

#endif /* HKDF_CHARM_H */