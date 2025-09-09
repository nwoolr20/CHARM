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
 * @file audit.c
 * @brief CHARM Tamper-Evident Audit Logging Implementation
 */

#include "crypto/audit.h"
#include "charm_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

// Internal state
typedef struct {
    int initialized;
    charm_audit_config_t config;
    FILE* log_file;
    uint64_t next_sequence_id;
    uint8_t last_chain_hash[32];
    pthread_mutex_t mutex;
} charm_audit_state_t;

static charm_audit_state_t g_audit_state = {0};

// Helper function to get current time with microsecond precision
static void get_precise_time(time_t* seconds, uint32_t* microseconds) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seconds = tv.tv_sec;
    *microseconds = tv.tv_usec;
}

// Helper function to compute entry integrity hash
static int compute_entry_hash(const charm_audit_entry_t* entry, uint8_t* hash_out) {
    // Create hash input from all fields except the hash itself
    uint8_t hash_input[sizeof(charm_audit_entry_t) - 64]; // Exclude both hash fields
    size_t offset = 0;
    
    memcpy(hash_input + offset, &entry->sequence_id, sizeof(entry->sequence_id));
    offset += sizeof(entry->sequence_id);
    
    memcpy(hash_input + offset, &entry->timestamp, sizeof(entry->timestamp));
    offset += sizeof(entry->timestamp);
    
    memcpy(hash_input + offset, &entry->microseconds, sizeof(entry->microseconds));
    offset += sizeof(entry->microseconds);
    
    memcpy(hash_input + offset, &entry->level, sizeof(entry->level));
    offset += sizeof(entry->level);
    
    memcpy(hash_input + offset, &entry->category, sizeof(entry->category));
    offset += sizeof(entry->category);
    
    memcpy(hash_input + offset, &entry->outcome, sizeof(entry->outcome));
    offset += sizeof(entry->outcome);
    
    memcpy(hash_input + offset, entry->subject, CHARM_AUDIT_MAX_SUBJECT);
    offset += CHARM_AUDIT_MAX_SUBJECT;
    
    memcpy(hash_input + offset, entry->object, CHARM_AUDIT_MAX_OBJECT);
    offset += CHARM_AUDIT_MAX_OBJECT;
    
    memcpy(hash_input + offset, entry->message, CHARM_AUDIT_MAX_MESSAGE);
    offset += CHARM_AUDIT_MAX_MESSAGE;
    
    memcpy(hash_input + offset, entry->context, CHARM_AUDIT_MAX_CONTEXT);
    offset += CHARM_AUDIT_MAX_CONTEXT;
    
    return charm_digest_compute(hash_input, offset, hash_out);
}

// Helper function to compute chain hash
static int compute_chain_hash(const charm_audit_entry_t* entry, const uint8_t* prev_chain_hash, uint8_t* chain_hash_out) {
    uint8_t chain_input[64]; // Previous hash + current integrity hash
    memcpy(chain_input, prev_chain_hash, 32);
    memcpy(chain_input + 32, entry->integrity_hash, 32);
    
    return charm_digest_compute(chain_input, 64, chain_hash_out);
}

// Initialize audit logging system
int charm_audit_init(const charm_audit_config_t* config) {
    if (g_audit_state.initialized) {
        return 0; // Already initialized
    }
    
    if (!config) {
        return -EINVAL;
    }
    
    // Copy configuration
    g_audit_state.config = *config;
    
    // Initialize mutex
    if (pthread_mutex_init(&g_audit_state.mutex, NULL) != 0) {
        return -1;
    }
    
    // Open log file if configured
    if (config->log_to_file && config->log_file_path[0] != '\0') {
        g_audit_state.log_file = fopen(config->log_file_path, "a");
        if (!g_audit_state.log_file) {
            pthread_mutex_destroy(&g_audit_state.mutex);
            return -errno;
        }
        
        // Set line buffering for real-time logging
        setvbuf(g_audit_state.log_file, NULL, _IOLBF, 0);
    }
    
    // Initialize sequence ID and chain hash
    g_audit_state.next_sequence_id = 1;
    
    // Initialize chain hash with CHARM-derived seed
    uint8_t chain_seed[] = "CHARM_AUDIT_CHAIN_GENESIS";
    if (charm_digest_compute(chain_seed, sizeof(chain_seed) - 1, g_audit_state.last_chain_hash) != 0) {
        if (g_audit_state.log_file) {
            fclose(g_audit_state.log_file);
        }
        pthread_mutex_destroy(&g_audit_state.mutex);
        return -1;
    }
    
    g_audit_state.initialized = 1;
    
    // Log initialization event
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS,
                    "audit_system", "initialization", "Audit logging initialized", NULL);
    
    return 0;
}

// Shutdown audit logging system
int charm_audit_shutdown(void) {
    if (!g_audit_state.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_audit_state.mutex);
    
    // Log shutdown event
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS,
                    "audit_system", "shutdown", "Audit logging shutdown", NULL);
    
    // Close log file
    if (g_audit_state.log_file) {
        fclose(g_audit_state.log_file);
        g_audit_state.log_file = NULL;
    }
    
    g_audit_state.initialized = 0;
    
    pthread_mutex_unlock(&g_audit_state.mutex);
    pthread_mutex_destroy(&g_audit_state.mutex);
    
    return 0;
}

// Log an audit event
int charm_audit_log(charm_audit_level_t level,
                    charm_audit_category_t category,
                    charm_audit_outcome_t outcome,
                    const char* subject,
                    const char* object,
                    const char* message,
                    const char* context) {
    
    if (!g_audit_state.initialized || !g_audit_state.config.enabled) {
        return 0; // Audit disabled
    }
    
    if (level < g_audit_state.config.min_level) {
        return 0; // Below minimum level
    }
    
    pthread_mutex_lock(&g_audit_state.mutex);
    
    // Create audit entry
    charm_audit_entry_t entry = {0};
    entry.sequence_id = g_audit_state.next_sequence_id++;
    get_precise_time(&entry.timestamp, &entry.microseconds);
    entry.level = level;
    entry.category = category;
    entry.outcome = outcome;
    
    // Copy strings with bounds checking
    if (subject) {
        strncpy(entry.subject, subject, CHARM_AUDIT_MAX_SUBJECT - 1);
    }
    if (object) {
        strncpy(entry.object, object, CHARM_AUDIT_MAX_OBJECT - 1);
    }
    if (message) {
        strncpy(entry.message, message, CHARM_AUDIT_MAX_MESSAGE - 1);
    }
    if (context) {
        strncpy(entry.context, context, CHARM_AUDIT_MAX_CONTEXT - 1);
    }
    
    // Compute integrity hash
    if (compute_entry_hash(&entry, entry.integrity_hash) != 0) {
        pthread_mutex_unlock(&g_audit_state.mutex);
        return -1;
    }
    
    // Compute chain hash
    if (compute_chain_hash(&entry, g_audit_state.last_chain_hash, entry.chain_hash) != 0) {
        pthread_mutex_unlock(&g_audit_state.mutex);
        return -1;
    }
    
    // Update chain hash for next entry
    memcpy(g_audit_state.last_chain_hash, entry.chain_hash, 32);
    
    // Write to log file if configured
    if (g_audit_state.config.log_to_file && g_audit_state.log_file) {
        // Write binary entry for integrity verification
        size_t written = fwrite(&entry, 1, sizeof(entry), g_audit_state.log_file);
        if (written != sizeof(entry)) {
            pthread_mutex_unlock(&g_audit_state.mutex);
            return -EIO;
        }
        
        if (g_audit_state.config.sync_writes) {
            fflush(g_audit_state.log_file);
            fsync(fileno(g_audit_state.log_file));
        }
    }
    
    pthread_mutex_unlock(&g_audit_state.mutex);
    return 0;
}

// Log a formatted audit event
int charm_audit_logf(charm_audit_level_t level,
                     charm_audit_category_t category,
                     charm_audit_outcome_t outcome,
                     const char* subject,
                     const char* object,
                     const char* format, ...) {
    
    char message[CHARM_AUDIT_MAX_MESSAGE];
    va_list args;
    
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);
    
    return charm_audit_log(level, category, outcome, subject, object, message, NULL);
}

// Verify audit log integrity
int charm_audit_verify_integrity(uint64_t start_sequence, uint64_t end_sequence, int* tampered) {
    if (!g_audit_state.initialized || !tampered) {
        return -EINVAL;
    }
    
    *tampered = 0;
    
    if (!g_audit_state.log_file) {
        return -ENOENT; // No log file to verify
    }
    
    pthread_mutex_lock(&g_audit_state.mutex);
    
    // Rewind to beginning of file
    fseek(g_audit_state.log_file, 0, SEEK_SET);
    
    charm_audit_entry_t entry;
    uint8_t expected_chain_hash[32];
    int first_entry = 1;
    
    // Initialize expected chain hash
    uint8_t chain_seed[] = "CHARM_AUDIT_CHAIN_GENESIS";
    if (charm_digest_compute(chain_seed, sizeof(chain_seed) - 1, expected_chain_hash) != 0) {
        pthread_mutex_unlock(&g_audit_state.mutex);
        return -1;
    }
    
    while (fread(&entry, sizeof(entry), 1, g_audit_state.log_file) == 1) {
        // Skip entries outside requested range
        if (start_sequence > 0 && entry.sequence_id < start_sequence) {
            continue;
        }
        if (end_sequence > 0 && entry.sequence_id > end_sequence) {
            break;
        }
        
        // Verify integrity hash
        uint8_t computed_hash[32];
        if (compute_entry_hash(&entry, computed_hash) != 0) {
            pthread_mutex_unlock(&g_audit_state.mutex);
            return -1;
        }
        
        if (memcmp(computed_hash, entry.integrity_hash, 32) != 0) {
            *tampered = 1;
            break;
        }
        
        // Verify chain hash (skip for first entry being verified)
        if (!first_entry) {
            uint8_t computed_chain_hash[32];
            if (compute_chain_hash(&entry, expected_chain_hash, computed_chain_hash) != 0) {
                pthread_mutex_unlock(&g_audit_state.mutex);
                return -1;
            }
            
            if (memcmp(computed_chain_hash, entry.chain_hash, 32) != 0) {
                *tampered = 1;
                break;
            }
        }
        
        // Update expected chain hash for next entry
        memcpy(expected_chain_hash, entry.chain_hash, 32);
        first_entry = 0;
    }
    
    pthread_mutex_unlock(&g_audit_state.mutex);
    return 0;
}

// Enable/disable audit logging at runtime
int charm_audit_set_enabled(int enabled) {
    if (!g_audit_state.initialized) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_audit_state.mutex);
    g_audit_state.config.enabled = enabled;
    pthread_mutex_unlock(&g_audit_state.mutex);
    
    return 0;
}

// Set minimum audit level at runtime
int charm_audit_set_level(charm_audit_level_t min_level) {
    if (!g_audit_state.initialized) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_audit_state.mutex);
    g_audit_state.config.min_level = min_level;
    pthread_mutex_unlock(&g_audit_state.mutex);
    
    return 0;
}