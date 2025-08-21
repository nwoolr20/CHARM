#ifndef CHARM_SECURITY_SUITE_H
#define CHARM_SECURITY_SUITE_H

/**
 * @file charm_security_suite.h
 * @brief CHARM Security Suite - Main API Header
 * 
 * Comprehensive security suite providing cryptographic services, identity & access
 * management, data protection, threat detection, vulnerability management, and more.
 * 
 * This is the main entry point for the CHARM Security Suite, providing unified
 * access to all security subsystems.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

// Include core CHARM functionality
#include "charm_api.h"

// Include security suite components
#include "crypto/key_manager.h"
#include "crypto/audit.h"
#include "crypto/auth.h"
#include "crypto/config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Security suite version
#define CHARM_SECURITY_SUITE_VERSION_MAJOR 1
#define CHARM_SECURITY_SUITE_VERSION_MINOR 0
#define CHARM_SECURITY_SUITE_VERSION_PATCH 0

// Security suite capabilities
typedef enum {
    CHARM_CAPABILITY_CRYPTOGRAPHIC_SERVICES = 0x01,    // Core crypto (hash, AEAD, etc.)
    CHARM_CAPABILITY_KEY_MANAGEMENT = 0x02,            // Key generation, storage, rotation
    CHARM_CAPABILITY_IDENTITY_ACCESS = 0x04,           // Authentication & authorization
    CHARM_CAPABILITY_DATA_PROTECTION = 0x08,           // At-rest/in-transit encryption
    CHARM_CAPABILITY_AUDIT_LOGGING = 0x10,             // Tamper-evident audit logs
    CHARM_CAPABILITY_CONFIG_MANAGEMENT = 0x20,         // Secure configuration
    CHARM_CAPABILITY_THREAT_DETECTION = 0x40,          // Security monitoring
    CHARM_CAPABILITY_VULNERABILITY_MGMT = 0x80,        // Dependency scanning, SBOM
    CHARM_CAPABILITY_INCIDENT_RESPONSE = 0x100,        // Alerting, forensics
    CHARM_CAPABILITY_LIFECYCLE_SECURITY = 0x200        // Update security, provenance
} charm_capability_t;

// Security suite configuration
typedef struct {
    // Core settings
    bool enable_fips_mode;
    bool enable_quantum_resistance;
    charm_keystore_type_t default_keystore;
    
    // Audit configuration
    charm_audit_config_t audit_config;
    
    // Authentication configuration
    charm_auth_method_t default_auth_method;
    charm_authz_policy_type_t default_authz_policy;
    
    // Security policies
    bool require_authentication;
    bool enable_runtime_config_changes;
    uint32_t session_timeout_seconds;
    uint32_t max_failed_auth_attempts;
    
    // Monitoring settings
    bool enable_anomaly_detection;
    bool enable_integrity_monitoring;
    double entropy_quality_threshold;
    
    // File paths
    char config_file[256];
    char keystore_path[256];
    char audit_log_path[256];
    char threat_log_path[256];
} charm_security_suite_config_t;

// Security suite status
typedef struct {
    bool initialized;
    uint32_t enabled_capabilities;
    time_t init_time;
    
    // Component status
    bool crypto_available;
    bool key_mgmt_available;
    bool auth_available;
    bool audit_available;
    bool config_available;
    
    // Security metrics
    uint64_t total_crypto_operations;
    uint64_t total_auth_attempts;
    uint64_t total_auth_failures;
    uint64_t total_audit_events;
    uint64_t total_config_changes;
    
    // Current security level
    int current_security_level;  // 0-100 (higher = more secure)
    double entropy_quality;
    time_t last_key_rotation;
    time_t last_audit_verification;
} charm_security_suite_status_t;

// Security incident types
typedef enum {
    CHARM_INCIDENT_AUTHENTICATION_FAILURE = 0,
    CHARM_INCIDENT_AUTHORIZATION_VIOLATION,
    CHARM_INCIDENT_CRYPTO_FAILURE,
    CHARM_INCIDENT_KEY_COMPROMISE,
    CHARM_INCIDENT_AUDIT_TAMPERING,
    CHARM_INCIDENT_CONFIG_VIOLATION,
    CHARM_INCIDENT_ENTROPY_DEGRADATION,
    CHARM_INCIDENT_UNKNOWN_THREAT
} charm_incident_type_t;

// Security incident
typedef struct {
    uint64_t incident_id;
    charm_incident_type_t type;
    time_t timestamp;
    char source[64];
    char description[256];
    char affected_resource[128];
    int severity;  // 1-10 (10 = critical)
    bool resolved;
    time_t resolution_time;
    char response_actions[512];
} charm_security_incident_t;

/**
 * Initialize the CHARM Security Suite
 * @param config Security suite configuration
 * @return 0 on success, negative on error
 */
int charm_security_suite_init(const charm_security_suite_config_t* config);

/**
 * Shutdown the CHARM Security Suite
 * @return 0 on success, negative on error
 */
int charm_security_suite_shutdown(void);

/**
 * Get security suite version
 * @return Version string
 */
const char* charm_security_suite_get_version(void);

/**
 * Get enabled capabilities
 * @param capabilities Output bitmask of enabled capabilities
 * @return 0 on success, negative on error
 */
int charm_security_suite_get_capabilities(uint32_t* capabilities);

/**
 * Get security suite status
 * @param status Output status structure
 * @return 0 on success, negative on error
 */
int charm_security_suite_get_status(charm_security_suite_status_t* status);

/**
 * Perform comprehensive security health check
 * @param issues Output array for discovered issues
 * @param max_issues Maximum number of issues to return
 * @param issue_count Actual number of issues found
 * @return Overall health score (0-100)
 */
int charm_security_suite_health_check(char issues[][256], size_t max_issues, size_t* issue_count);

/**
 * Enable/disable specific capability
 * @param capability Capability to modify
 * @param enable True to enable, false to disable
 * @return 0 on success, negative on error
 */
int charm_security_suite_set_capability(charm_capability_t capability, bool enable);

/**
 * Report a security incident
 * @param type Incident type
 * @param source Source of the incident
 * @param description Description of the incident
 * @param affected_resource Affected resource (optional)
 * @param severity Severity level (1-10)
 * @param incident_id Output incident ID
 * @return 0 on success, negative on error
 */
int charm_security_suite_report_incident(charm_incident_type_t type,
                                         const char* source,
                                         const char* description,
                                         const char* affected_resource,
                                         int severity,
                                         uint64_t* incident_id);

/**
 * Get security incidents
 * @param incidents Output array for incidents
 * @param max_incidents Maximum number of incidents to return
 * @param incident_count Actual number of incidents returned
 * @param unresolved_only If true, only return unresolved incidents
 * @return 0 on success, negative on error
 */
int charm_security_suite_get_incidents(charm_security_incident_t* incidents,
                                       size_t max_incidents,
                                       size_t* incident_count,
                                       bool unresolved_only);

/**
 * Resolve a security incident
 * @param incident_id Incident ID to resolve
 * @param response_actions Description of response actions taken
 * @return 0 on success, negative on error
 */
int charm_security_suite_resolve_incident(uint64_t incident_id, const char* response_actions);

/**
 * Perform automatic key rotation for all eligible keys
 * @param rotated_count Output count of keys rotated
 * @return 0 on success, negative on error
 */
int charm_security_suite_auto_key_rotation(size_t* rotated_count);

/**
 * Verify integrity of all security components
 * @param tampered Output flag indicating if tampering detected
 * @param component Output name of tampered component (if any)
 * @return 0 on success, negative on error
 */
int charm_security_suite_verify_integrity(bool* tampered, char* component);

/**
 * Generate Security Bill of Materials (SBOM)
 * @param sbom_file Output file for SBOM
 * @param format Format ("json", "xml", "text")
 * @return 0 on success, negative on error
 */
int charm_security_suite_generate_sbom(const char* sbom_file, const char* format);

/**
 * Perform dependency vulnerability scan
 * @param vulnerabilities Output array for found vulnerabilities
 * @param max_vulns Maximum number of vulnerabilities to return
 * @param vuln_count Actual number of vulnerabilities found
 * @return 0 on success, negative on error
 */
int charm_security_suite_scan_vulnerabilities(char vulnerabilities[][256],
                                              size_t max_vulns,
                                              size_t* vuln_count);

/**
 * Export security configuration and logs for compliance
 * @param export_file Output file for export
 * @param format Export format ("json", "xml", "csv")
 * @param include_sensitive Include sensitive data in export
 * @return 0 on success, negative on error
 */
int charm_security_suite_export_compliance_data(const char* export_file,
                                                const char* format,
                                                bool include_sensitive);

/**
 * Perform emergency security lockdown
 * @param reason Reason for lockdown
 * @return 0 on success, negative on error
 */
int charm_security_suite_emergency_lockdown(const char* reason);

/**
 * Restore from emergency lockdown
 * @param auth_token Administrative authorization token
 * @return 0 on success, negative on error
 */
int charm_security_suite_restore_from_lockdown(const char* auth_token);

// High-level convenience functions combining multiple subsystems

/**
 * Secure data encryption with automatic key management
 * @param data Data to encrypt
 * @param data_len Length of data
 * @param key_id Key ID to use (NULL for auto-generated)
 * @param encrypted_data Output buffer for encrypted data
 * @param max_encrypted Maximum size of output buffer
 * @param actual_encrypted Actual size of encrypted data
 * @return 0 on success, negative on error
 */
int charm_secure_encrypt(const uint8_t* data, size_t data_len,
                         const char* key_id,
                         uint8_t* encrypted_data, size_t max_encrypted,
                         size_t* actual_encrypted);

/**
 * Secure data decryption with access control
 * @param encrypted_data Encrypted data
 * @param encrypted_len Length of encrypted data
 * @param key_id Key ID to use for decryption
 * @param auth_context Authentication context for access control
 * @param decrypted_data Output buffer for decrypted data
 * @param max_decrypted Maximum size of output buffer
 * @param actual_decrypted Actual size of decrypted data
 * @return 0 on success, negative on error
 */
int charm_secure_decrypt(const uint8_t* encrypted_data, size_t encrypted_len,
                         const char* key_id,
                         const charm_auth_context_t* auth_context,
                         uint8_t* decrypted_data, size_t max_decrypted,
                         size_t* actual_decrypted);

/**
 * Secure configuration value access with audit logging
 * @param key Configuration key
 * @param value Output buffer for value
 * @param max_len Maximum length of output buffer
 * @param auth_context Authentication context
 * @return 0 on success, negative on error
 */
int charm_secure_config_get(const char* key, char* value, size_t max_len,
                            const charm_auth_context_t* auth_context);

/**
 * Secure configuration value modification with audit logging
 * @param key Configuration key
 * @param value New value to set
 * @param auth_context Authentication context
 * @return 0 on success, negative on error
 */
int charm_secure_config_set(const char* key, const char* value,
                            const charm_auth_context_t* auth_context);

// Default security suite configuration
extern const charm_security_suite_config_t charm_default_security_config;

#ifdef __cplusplus
}
#endif

#endif // CHARM_SECURITY_SUITE_H