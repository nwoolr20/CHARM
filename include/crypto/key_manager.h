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

#ifndef CHARM_KEY_MANAGER_H
#define CHARM_KEY_MANAGER_H

/**
 * @file key_manager.h
 * @brief CHARM Key Management Framework
 * 
 * Provides key generation, storage, rotation, and lifecycle management
 * for the CHARM security suite. Supports both software and HSM backends.
 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Key types supported by CHARM
typedef enum {
    CHARM_KEY_TYPE_SYMMETRIC_AES256,    // AES-256 symmetric key
    CHARM_KEY_TYPE_SYMMETRIC_CHACHA20,  // ChaCha20 symmetric key  
    CHARM_KEY_TYPE_HMAC_CHARM,          // HMAC key using CHARM hash
    CHARM_KEY_TYPE_CHARM_NATIVE,        // CHARM-native derived key
    CHARM_KEY_TYPE_ED25519_PRIVATE,     // Ed25519 signing key
    CHARM_KEY_TYPE_X25519_PRIVATE,      // X25519 ECDH key
    CHARM_KEY_TYPE_RSA_2048,            // RSA-2048 for legacy support
    CHARM_KEY_TYPE_RSA_4096             // RSA-4096 for legacy support
} charm_key_type_t;

// Key usage flags
typedef enum {
    CHARM_KEY_USAGE_ENCRYPT     = 0x01,
    CHARM_KEY_USAGE_DECRYPT     = 0x02,
    CHARM_KEY_USAGE_SIGN        = 0x04,
    CHARM_KEY_USAGE_VERIFY      = 0x08,
    CHARM_KEY_USAGE_DERIVE      = 0x10,
    CHARM_KEY_USAGE_WRAP        = 0x20,
    CHARM_KEY_USAGE_UNWRAP      = 0x40
} charm_key_usage_t;

// Key storage backend types
typedef enum {
    CHARM_KEYSTORE_SOFTWARE,    // Software-based storage
    CHARM_KEYSTORE_HSM,         // Hardware Security Module
    CHARM_KEYSTORE_PKCS11,      // PKCS#11 interface
    CHARM_KEYSTORE_TPM          // Trusted Platform Module
} charm_keystore_type_t;

// Key metadata structure
typedef struct {
    char key_id[64];              // Unique key identifier
    charm_key_type_t type;        // Key algorithm/type
    uint32_t usage_flags;         // Permitted usage flags
    time_t created;               // Creation timestamp
    time_t expires;               // Expiration timestamp (0 = no expiry)
    uint32_t rotation_interval;   // Auto-rotation interval in days
    uint8_t version;              // Key version number
    char description[128];        // Human-readable description
    uint8_t key_checksum[32];     // CHARM hash of key material
} charm_key_metadata_t;

// Key handle for operations
typedef struct charm_key_handle {
    char key_id[64];
    charm_keystore_type_t store_type;
    void* backend_handle;
    charm_key_metadata_t metadata;
} charm_key_handle_t;

// Key generation parameters
typedef struct {
    charm_key_type_t type;
    uint32_t usage_flags;
    uint32_t key_size_bits;       // For variable-size keys
    time_t expiry_time;           // 0 = no expiry
    uint32_t rotation_days;       // 0 = no auto-rotation
    const char* description;
    const uint8_t* entropy_seed;  // Optional additional entropy
    size_t entropy_seed_len;
} charm_key_generation_params_t;

/**
 * Initialize the key management subsystem
 * @param keystore_type Primary keystore backend to use
 * @return 0 on success, negative on error
 */
int charm_key_manager_init(charm_keystore_type_t keystore_type);

/**
 * Shutdown the key management subsystem
 * @return 0 on success, negative on error
 */
int charm_key_manager_shutdown(void);

/**
 * Generate a new cryptographic key
 * @param params Key generation parameters
 * @param key_id Buffer to receive generated key ID (min 64 bytes)
 * @return 0 on success, negative on error
 */
int charm_key_generate(const charm_key_generation_params_t* params, char* key_id);

/**
 * Load an existing key by ID
 * @param key_id Key identifier
 * @param handle Output key handle
 * @return 0 on success, negative on error
 */
int charm_key_load(const char* key_id, charm_key_handle_t* handle);

/**
 * Delete a key from storage
 * @param key_id Key identifier
 * @return 0 on success, negative on error
 */
int charm_key_delete(const char* key_id);

/**
 * Rotate a key (generate new version, preserve old)
 * @param key_id Key identifier to rotate
 * @param new_key_id Buffer for new key ID (min 64 bytes)
 * @return 0 on success, negative on error
 */
int charm_key_rotate(const char* key_id, char* new_key_id);

/**
 * List all available keys
 * @param metadata_list Output array of key metadata
 * @param max_keys Maximum number of keys to return
 * @param count Output count of keys returned
 * @return 0 on success, negative on error
 */
int charm_key_list(charm_key_metadata_t* metadata_list, size_t max_keys, size_t* count);

/**
 * Get key metadata
 * @param key_id Key identifier
 * @param metadata Output metadata structure
 * @return 0 on success, negative on error
 */
int charm_key_get_metadata(const char* key_id, charm_key_metadata_t* metadata);

/**
 * Derive key material for cryptographic operations
 * @param handle Key handle
 * @param context Context string for derivation
 * @param output Buffer for derived key material
 * @param output_len Length of output buffer
 * @return 0 on success, negative on error
 */
int charm_key_derive(const charm_key_handle_t* handle, const char* context, 
                     uint8_t* output, size_t output_len);

/**
 * Check if key needs rotation based on policy
 * @param key_id Key identifier
 * @param needs_rotation Output flag
 * @return 0 on success, negative on error
 */
int charm_key_needs_rotation(const char* key_id, int* needs_rotation);

/**
 * Export key in secure format (for backup/migration)
 * @param key_id Key identifier
 * @param wrapping_key_id Key to encrypt export with
 * @param exported_data Output buffer for encrypted key
 * @param max_len Maximum length of output buffer
 * @param actual_len Actual length of exported data
 * @return 0 on success, negative on error
 */
int charm_key_export(const char* key_id, const char* wrapping_key_id,
                     uint8_t* exported_data, size_t max_len, size_t* actual_len);

/**
 * Import key from secure format
 * @param exported_data Encrypted key data
 * @param data_len Length of encrypted data
 * @param wrapping_key_id Key to decrypt import with
 * @param imported_key_id Output buffer for imported key ID
 * @return 0 on success, negative on error
 */
int charm_key_import(const uint8_t* exported_data, size_t data_len,
                     const char* wrapping_key_id, char* imported_key_id);

#ifdef __cplusplus
}
#endif

#endif // CHARM_KEY_MANAGER_H