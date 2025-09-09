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

#ifndef CHARM_AUDIT_H
#define CHARM_AUDIT_H

/**
 * @file audit.h
 * @brief CHARM Tamper-Evident Audit Logging System
 * 
 * Provides integrity-protected audit logging for security-critical events
 * throughout the CHARM security suite.
 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Audit event severity levels
typedef enum {
    CHARM_AUDIT_LEVEL_DEBUG = 0,
    CHARM_AUDIT_LEVEL_INFO = 1,
    CHARM_AUDIT_LEVEL_WARN = 2,
    CHARM_AUDIT_LEVEL_ERROR = 3,
    CHARM_AUDIT_LEVEL_CRITICAL = 4
} charm_audit_level_t;

// Audit event categories
typedef enum {
    CHARM_AUDIT_CAT_SYSTEM = 0,        // System startup/shutdown
    CHARM_AUDIT_CAT_CRYPTO = 1,        // Cryptographic operations
    CHARM_AUDIT_CAT_KEY_MGT = 2,       // Key management
    CHARM_AUDIT_CAT_AUTH = 3,          // Authentication/authorization
    CHARM_AUDIT_CAT_CONFIG = 4,        // Configuration changes
    CHARM_AUDIT_CAT_ENTROPY = 5,       // Entropy collection/health
    CHARM_AUDIT_CAT_SECURITY = 6,      // Security violations/incidents
    CHARM_AUDIT_CAT_ACCESS = 7         // Data access/modification
} charm_audit_category_t;

// Audit event outcome
typedef enum {
    CHARM_AUDIT_SUCCESS = 0,
    CHARM_AUDIT_FAILURE = 1,
    CHARM_AUDIT_PENDING = 2
} charm_audit_outcome_t;

// Maximum sizes for audit fields
#define CHARM_AUDIT_MAX_MESSAGE 512
#define CHARM_AUDIT_MAX_SUBJECT 64
#define CHARM_AUDIT_MAX_OBJECT 128
#define CHARM_AUDIT_MAX_CONTEXT 256

// Audit log entry structure
typedef struct {
    uint64_t sequence_id;              // Monotonic sequence number
    time_t timestamp;                  // Event timestamp
    uint32_t microseconds;             // Sub-second precision
    charm_audit_level_t level;         // Severity level
    charm_audit_category_t category;   // Event category
    charm_audit_outcome_t outcome;     // Success/failure
    char subject[CHARM_AUDIT_MAX_SUBJECT];    // Who (user, process, etc.)
    char object[CHARM_AUDIT_MAX_OBJECT];      // What (resource, key, etc.)
    char message[CHARM_AUDIT_MAX_MESSAGE];    // Human-readable description
    char context[CHARM_AUDIT_MAX_CONTEXT];    // Additional context data
    uint8_t integrity_hash[32];        // CHARM hash for integrity
    uint8_t chain_hash[32];            // Hash chain for tamper detection
} charm_audit_entry_t;

// Audit configuration
typedef struct {
    int enabled;
    charm_audit_level_t min_level;
    int log_to_file;
    int log_to_syslog;
    int log_to_network;
    char log_file_path[256];
    char syslog_facility[32];
    char network_endpoint[128];
    int max_log_size_mb;
    int max_log_files;
    int sync_writes;
} charm_audit_config_t;

/**
 * Initialize the audit logging system
 * @param config Audit configuration
 * @return 0 on success, negative on error
 */
int charm_audit_init(const charm_audit_config_t* config);

/**
 * Shutdown the audit logging system
 * @return 0 on success, negative on error
 */
int charm_audit_shutdown(void);

/**
 * Log an audit event
 * @param level Severity level
 * @param category Event category
 * @param outcome Success/failure
 * @param subject Who performed the action
 * @param object What was acted upon
 * @param message Descriptive message
 * @param context Additional context (optional)
 * @return 0 on success, negative on error
 */
int charm_audit_log(charm_audit_level_t level,
                    charm_audit_category_t category,
                    charm_audit_outcome_t outcome,
                    const char* subject,
                    const char* object,
                    const char* message,
                    const char* context);

/**
 * Log a formatted audit event
 * @param level Severity level
 * @param category Event category
 * @param outcome Success/failure
 * @param subject Who performed the action
 * @param object What was acted upon
 * @param format Printf-style format string
 * @param ... Format arguments
 * @return 0 on success, negative on error
 */
int charm_audit_logf(charm_audit_level_t level,
                     charm_audit_category_t category,
                     charm_audit_outcome_t outcome,
                     const char* subject,
                     const char* object,
                     const char* format, ...);

/**
 * Verify audit log integrity
 * @param start_sequence Starting sequence number (0 for full log)
 * @param end_sequence Ending sequence number (0 for current)
 * @param tampered Output flag indicating if tampering detected
 * @return 0 on success, negative on error
 */
int charm_audit_verify_integrity(uint64_t start_sequence, uint64_t end_sequence, int* tampered);

/**
 * Get audit statistics
 * @param total_entries Total number of audit entries
 * @param first_sequence First sequence number in log
 * @param last_sequence Last sequence number in log
 * @return 0 on success, negative on error
 */
int charm_audit_get_stats(uint64_t* total_entries, uint64_t* first_sequence, uint64_t* last_sequence);

/**
 * Query audit log entries
 * @param start_time Start time filter (0 for no filter)
 * @param end_time End time filter (0 for no filter)
 * @param min_level Minimum severity level
 * @param category Category filter (-1 for all categories)
 * @param entries Output buffer for matching entries
 * @param max_entries Maximum number of entries to return
 * @param count Actual number of entries returned
 * @return 0 on success, negative on error
 */
int charm_audit_query(time_t start_time, time_t end_time,
                      charm_audit_level_t min_level,
                      int category,
                      charm_audit_entry_t* entries,
                      size_t max_entries,
                      size_t* count);

/**
 * Archive old audit log entries
 * @param archive_before_time Archive entries older than this time
 * @param archive_path Path to archive file
 * @return 0 on success, negative on error
 */
int charm_audit_archive(time_t archive_before_time, const char* archive_path);

/**
 * Enable/disable audit logging at runtime
 * @param enabled 1 to enable, 0 to disable
 * @return 0 on success, negative on error
 */
int charm_audit_set_enabled(int enabled);

/**
 * Set minimum audit level at runtime
 * @param min_level New minimum level
 * @return 0 on success, negative on error
 */
int charm_audit_set_level(charm_audit_level_t min_level);

// Convenience macros for common audit events
#define CHARM_AUDIT_SYSTEM_START() \
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS, \
                    "system", "charm", "CHARM system started", NULL)

#define CHARM_AUDIT_SYSTEM_STOP() \
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS, \
                    "system", "charm", "CHARM system stopped", NULL)

#define CHARM_AUDIT_KEY_GENERATED(key_id, key_type) \
    charm_audit_logf(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_KEY_MGT, CHARM_AUDIT_SUCCESS, \
                     "key_manager", key_id, "Key generated: type=%d", key_type)

#define CHARM_AUDIT_KEY_DELETED(key_id) \
    charm_audit_log(CHARM_AUDIT_LEVEL_WARN, CHARM_AUDIT_CAT_KEY_MGT, CHARM_AUDIT_SUCCESS, \
                    "key_manager", key_id, "Key deleted", NULL)

#define CHARM_AUDIT_AUTH_SUCCESS(user, resource) \
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_SUCCESS, \
                    user, resource, "Authentication successful", NULL)

#define CHARM_AUDIT_AUTH_FAILURE(user, resource, reason) \
    charm_audit_log(CHARM_AUDIT_LEVEL_WARN, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_FAILURE, \
                    user, resource, "Authentication failed", reason)

#define CHARM_AUDIT_ENTROPY_HEALTH(quality, source) \
    charm_audit_logf(CHARM_AUDIT_LEVEL_DEBUG, CHARM_AUDIT_CAT_ENTROPY, CHARM_AUDIT_SUCCESS, \
                     "entropy_monitor", source, "Entropy quality: %.3f", quality)

#define CHARM_AUDIT_SECURITY_VIOLATION(threat_type, details) \
    charm_audit_log(CHARM_AUDIT_LEVEL_CRITICAL, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_FAILURE, \
                    "security_monitor", threat_type, "Security violation detected", details)

#ifdef __cplusplus
}
#endif

#endif // CHARM_AUDIT_H