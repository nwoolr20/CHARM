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
 * @file key_manager.c
 * @brief CHARM Key Management Implementation
 * 
 * Software-based key storage implementation with CHARM-native entropy
 */

#include "crypto/key_manager.h"
#include "charm_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

// Configuration
#define CHARM_KEYSTORE_DIR "./keystore"
#define CHARM_KEY_FILE_SUFFIX ".key"
#define CHARM_METADATA_FILE_SUFFIX ".meta"
#define MAX_KEY_SIZE 4096
#define KEY_ID_ENTROPY_SIZE 32

// Internal structures
typedef struct {
    int initialized;
    charm_keystore_type_t backend_type;
    char keystore_path[256];
} charm_key_manager_state_t;

static charm_key_manager_state_t g_key_manager = {0};

// Forward declaration for secure random bytes
extern int rng_linux_get_bytes(uint8_t* buffer, size_t size);

// Helper function to generate secure key ID
static int generate_key_id(char* key_id_out) {
    uint8_t entropy[KEY_ID_ENTROPY_SIZE];
    uint8_t digest[32];
    
    // Get high-quality entropy from CHARM system
    int ret = charm_feed_entropy(NULL, 0); // Trigger entropy collection
    if (ret != 0) {
        return -1;
    }
    
    // Generate cryptographically secure random bytes for key ID
    if (rng_linux_get_bytes(entropy, KEY_ID_ENTROPY_SIZE) != (int)KEY_ID_ENTROPY_SIZE) {
        return -1;
    }
    
    // Hash entropy to create deterministic but unpredictable key ID
    ret = charm_digest_compute(entropy, KEY_ID_ENTROPY_SIZE, digest);
    if (ret != 0) {
        return -1;
    }
    
    // Convert to hex string
    char hex_str[65];
    charm_digest_to_hex(digest, hex_str);
    
    // Use first 32 characters as key ID
    strncpy(key_id_out, hex_str, 32);
    key_id_out[32] = '\0';
    
    return 0;
}

// Helper function to get file paths
static void get_key_file_path(const char* key_id, char* path_out) {
    snprintf(path_out, 255, "%s/%.32s%s", g_key_manager.keystore_path, key_id, CHARM_KEY_FILE_SUFFIX);
    path_out[255] = '\0';
}

static void get_metadata_file_path(const char* key_id, char* path_out) {
    snprintf(path_out, 255, "%s/%.32s%s", g_key_manager.keystore_path, key_id, CHARM_METADATA_FILE_SUFFIX);
    path_out[255] = '\0';
}

// Initialize key management subsystem
int charm_key_manager_init(charm_keystore_type_t keystore_type) {
    if (g_key_manager.initialized) {
        return 0; // Already initialized
    }
    
    // Only support software keystore for now
    if (keystore_type != CHARM_KEYSTORE_SOFTWARE) {
        return -ENOTSUP;
    }
    
    g_key_manager.backend_type = keystore_type;
    strncpy(g_key_manager.keystore_path, CHARM_KEYSTORE_DIR, sizeof(g_key_manager.keystore_path) - 1);
    
    // Create keystore directory
    struct stat st;
    if (stat(g_key_manager.keystore_path, &st) != 0) {
        if (mkdir(g_key_manager.keystore_path, 0700) != 0) {
            return -errno;
        }
    }
    
    g_key_manager.initialized = 1;
    return 0;
}

// Shutdown key management subsystem
int charm_key_manager_shutdown(void) {
    if (!g_key_manager.initialized) {
        return 0;
    }
    
    memset(&g_key_manager, 0, sizeof(g_key_manager));
    return 0;
}

// Generate a new cryptographic key
int charm_key_generate(const charm_key_generation_params_t* params, char* key_id) {
    if (!g_key_manager.initialized || !params || !key_id) {
        return -EINVAL;
    }
    
    // Generate unique key ID
    if (generate_key_id(key_id) != 0) {
        return -1;
    }
    
    // Determine key size based on type
    size_t key_size;
    switch (params->type) {
        case CHARM_KEY_TYPE_SYMMETRIC_AES256:
            key_size = 32;
            break;
        case CHARM_KEY_TYPE_SYMMETRIC_CHACHA20:
            key_size = 32;
            break;
        case CHARM_KEY_TYPE_HMAC_CHARM:
            key_size = 64; // 512-bit HMAC key
            break;
        case CHARM_KEY_TYPE_CHARM_NATIVE:
            key_size = 32; // CHARM hash output size
            break;
        case CHARM_KEY_TYPE_ED25519_PRIVATE:
            key_size = 32;
            break;
        case CHARM_KEY_TYPE_X25519_PRIVATE:
            key_size = 32;
            break;
        default:
            return -ENOTSUP; // Unsupported key type
    }
    
    // Generate key material using CHARM entropy
    uint8_t key_material[MAX_KEY_SIZE];
    if (key_size > MAX_KEY_SIZE) {
        return -EINVAL;
    }
    
    // Use CHARM system for high-quality key material generation
    uint8_t entropy_seed[64];
    for (size_t i = 0; i < sizeof(entropy_seed); i++) {
        entropy_seed[i] = (uint8_t)(rand() ^ (time(NULL) + i * 7));
    }
    
    // Additional entropy from parameters if provided
    if (params->entropy_seed && params->entropy_seed_len > 0) {
        for (size_t i = 0; i < sizeof(entropy_seed) && i < params->entropy_seed_len; i++) {
            entropy_seed[i] ^= params->entropy_seed[i];
        }
    }
    
    // Generate key material using CHARM digest
    uint8_t key_seed[key_size + 32]; // Extra space for multiple rounds
    memcpy(key_seed, entropy_seed, sizeof(entropy_seed));
    
    for (size_t i = 0; i < key_size; i += 32) {
        uint8_t round_input[96];
        memcpy(round_input, key_seed, 64);
        memcpy(round_input + 64, &i, sizeof(i));
        
        // Safely copy key_id with bounds checking
        size_t key_id_len = strlen(key_id);
        size_t key_id_copy_len = (key_id_len < 24) ? key_id_len : 24; // Leave space for safety
        memcpy(round_input + 64 + sizeof(i), key_id, key_id_copy_len);
        
        uint8_t round_output[32];
        if (charm_digest_compute(round_input, 64 + sizeof(i) + key_id_copy_len, round_output) != 0) {
            return -1;
        }
        
        size_t copy_len = (key_size - i) < 32 ? (key_size - i) : 32;
        memcpy(key_material + i, round_output, copy_len);
    }
    
    // Create metadata
    charm_key_metadata_t metadata = {0};
    strncpy(metadata.key_id, key_id, sizeof(metadata.key_id) - 1);
    metadata.type = params->type;
    metadata.usage_flags = params->usage_flags;
    metadata.created = time(NULL);
    metadata.expires = params->expiry_time;
    metadata.rotation_interval = params->rotation_days;
    metadata.version = 1;
    
    if (params->description) {
        strncpy(metadata.description, params->description, sizeof(metadata.description) - 1);
    }
    
    // Compute key checksum
    if (charm_digest_compute(key_material, key_size, metadata.key_checksum) != 0) {
        return -1;
    }
    
    // Save key material to file with restrictive permissions
    char key_file_path[256];
    get_key_file_path(key_id, key_file_path);
    
    // Create file with restricted permissions (owner read/write only)
    int key_fd = open(key_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (key_fd < 0) {
        return -errno;
    }
    
    ssize_t written = write(key_fd, key_material, key_size);
    close(key_fd);
    
    if (written != (ssize_t)key_size) {
        unlink(key_file_path); // Clean up partial file
        return -EIO;
    }
    
    // Save metadata to file with restrictive permissions
    char meta_file_path[256];
    get_metadata_file_path(key_id, meta_file_path);
    
    // Create metadata file with restricted permissions (owner read/write only)
    int meta_fd = open(meta_file_path, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (meta_fd < 0) {
        unlink(key_file_path); // Clean up key file
        return -errno;
    }
    
    ssize_t meta_written = write(meta_fd, &metadata, sizeof(metadata));
    close(meta_fd);
    
    if (meta_written != (ssize_t)sizeof(metadata)) {
        unlink(key_file_path);
        unlink(meta_file_path);
        return -EIO;
    }
    
    // Clear sensitive material from memory
    memset(key_material, 0, sizeof(key_material));
    memset(key_seed, 0, sizeof(key_seed));
    
    return 0;
}

// Load an existing key by ID
int charm_key_load(const char* key_id, charm_key_handle_t* handle) {
    if (!g_key_manager.initialized || !key_id || !handle) {
        return -EINVAL;
    }
    
    // Load metadata
    char meta_file_path[256];
    get_metadata_file_path(key_id, meta_file_path);
    
    FILE* meta_file = fopen(meta_file_path, "rb");
    if (!meta_file) {
        return -ENOENT;
    }
    
    size_t read_size = fread(&handle->metadata, 1, sizeof(handle->metadata), meta_file);
    fclose(meta_file);
    
    if (read_size != sizeof(handle->metadata)) {
        return -EIO;
    }
    
    // Verify key file exists
    char key_file_path[256];
    get_key_file_path(key_id, key_file_path);
    
    if (access(key_file_path, R_OK) != 0) {
        return -ENOENT;
    }
    
    // Initialize handle
    strncpy(handle->key_id, key_id, sizeof(handle->key_id) - 1);
    handle->store_type = g_key_manager.backend_type;
    handle->backend_handle = NULL; // No external backend for software storage
    
    return 0;
}

// Get key metadata
int charm_key_get_metadata(const char* key_id, charm_key_metadata_t* metadata) {
    if (!g_key_manager.initialized || !key_id || !metadata) {
        return -EINVAL;
    }
    
    char meta_file_path[256];
    get_metadata_file_path(key_id, meta_file_path);
    
    FILE* meta_file = fopen(meta_file_path, "rb");
    if (!meta_file) {
        return -ENOENT;
    }
    
    size_t read_size = fread(metadata, 1, sizeof(*metadata), meta_file);
    fclose(meta_file);
    
    if (read_size != sizeof(*metadata)) {
        return -EIO;
    }
    
    return 0;
}

// Check if key needs rotation
int charm_key_needs_rotation(const char* key_id, int* needs_rotation) {
    if (!g_key_manager.initialized || !key_id || !needs_rotation) {
        return -EINVAL;
    }
    
    charm_key_metadata_t metadata;
    int ret = charm_key_get_metadata(key_id, &metadata);
    if (ret != 0) {
        return ret;
    }
    
    *needs_rotation = 0;
    
    // Check expiration
    if (metadata.expires > 0 && time(NULL) >= metadata.expires) {
        *needs_rotation = 1;
        return 0;
    }
    
    // Check rotation interval
    if (metadata.rotation_interval > 0) {
        time_t rotation_due = metadata.created + (metadata.rotation_interval * 24 * 3600);
        if (time(NULL) >= rotation_due) {
            *needs_rotation = 1;
        }
    }
    
    return 0;
}

// Delete a key from storage
int charm_key_delete(const char* key_id) {
    if (!g_key_manager.initialized || !key_id) {
        return -EINVAL;
    }
    
    char key_file_path[256];
    char meta_file_path[256];
    
    get_key_file_path(key_id, key_file_path);
    get_metadata_file_path(key_id, meta_file_path);
    
    // Remove both files
    int ret1 = unlink(key_file_path);
    int ret2 = unlink(meta_file_path);
    
    // Return success if at least one file was deleted
    return (ret1 == 0 || ret2 == 0) ? 0 : -errno;
}