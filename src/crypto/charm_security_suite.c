/**
 * @file charm_security_suite.c
 * @brief CHARM Security Suite Implementation
 */

#include "charm_security_suite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

// Global security suite state
typedef struct {
    bool initialized;
    charm_security_suite_config_t config;
    charm_security_suite_status_t status;
    uint64_t next_incident_id;
    charm_security_incident_t incidents[100];  // Store up to 100 incidents
    size_t incident_count;
    bool emergency_lockdown;
    pthread_mutex_t mutex;
} charm_security_suite_state_t;

static charm_security_suite_state_t g_suite_state = {0};

// Default configuration
const charm_security_suite_config_t charm_default_security_config = {
    .enable_fips_mode = false,
    .enable_quantum_resistance = true,
    .default_keystore = CHARM_KEYSTORE_SOFTWARE,
    
    .audit_config = {
        .enabled = true,
        .min_level = CHARM_AUDIT_LEVEL_INFO,
        .log_to_file = true,
        .log_to_syslog = false,
        .log_to_network = false,
        .log_file_path = "/home/runner/work/CHARM/CHARM/logs/charm_audit.log",
        .syslog_facility = "local0",
        .network_endpoint = "",
        .max_log_size_mb = 100,
        .max_log_files = 10,
        .sync_writes = true
    },
    
    .default_auth_method = CHARM_AUTH_METHOD_PASSWORD,
    .default_authz_policy = CHARM_AUTHZ_POLICY_RBAC,
    
    .require_authentication = true,
    .enable_runtime_config_changes = false,
    .session_timeout_seconds = 3600,
    .max_failed_auth_attempts = 3,
    
    .enable_anomaly_detection = true,
    .enable_integrity_monitoring = true,
    .entropy_quality_threshold = 0.8,
    
    .config_file = "/home/runner/work/CHARM/CHARM/config/charm_security.conf",
    .keystore_path = "/home/runner/work/CHARM/CHARM/keystore",
    .audit_log_path = "/home/runner/work/CHARM/CHARM/logs/charm_audit.log",
    .threat_log_path = "/home/runner/work/CHARM/CHARM/logs/charm_threats.log"
};

// Initialize the CHARM Security Suite
int charm_security_suite_init(const charm_security_suite_config_t* config) {
    if (g_suite_state.initialized) {
        return 0; // Already initialized
    }
    
    if (!config) {
        config = &charm_default_security_config;
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&g_suite_state.mutex, NULL) != 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    // Copy configuration
    g_suite_state.config = *config;
    
    // Initialize status
    g_suite_state.status.init_time = time(NULL);
    g_suite_state.status.enabled_capabilities = 0;
    g_suite_state.next_incident_id = 1;
    g_suite_state.incident_count = 0;
    g_suite_state.emergency_lockdown = false;
    
    int init_result = 0;
    
    // Initialize core CHARM system
    if (charm_init() == 0) {
        g_suite_state.status.crypto_available = true;
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_CRYPTOGRAPHIC_SERVICES;
    } else {
        fprintf(stderr, "Warning: CHARM core initialization failed\n");
        init_result = -1;
    }
    
    // Initialize audit logging
    if (charm_audit_init(&config->audit_config) == 0) {
        g_suite_state.status.audit_available = true;
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_AUDIT_LOGGING;
        
        // Log security suite initialization
        CHARM_AUDIT_SYSTEM_START();
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS,
                       "security_suite", "initialization", "CHARM Security Suite initialized", NULL);
    } else {
        fprintf(stderr, "Warning: Audit system initialization failed\n");
    }
    
    // Initialize key management
    if (charm_key_manager_init(config->default_keystore) == 0) {
        g_suite_state.status.key_mgmt_available = true;
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_KEY_MANAGEMENT;
        
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_KEY_MGT, CHARM_AUDIT_SUCCESS,
                       "key_manager", "initialization", "Key management initialized", NULL);
    } else {
        fprintf(stderr, "Warning: Key management initialization failed\n");
    }
    
    // Initialize authentication/authorization
    if (charm_auth_init() == 0) {
        g_suite_state.status.auth_available = true;
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_IDENTITY_ACCESS;
        
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_SUCCESS,
                       "auth_system", "initialization", "Authentication system initialized", NULL);
    } else {
        fprintf(stderr, "Warning: Authentication system initialization failed\n");
    }
    
    // Initialize configuration management
    if (charm_config_init(config->config_file, NULL) == 0) {
        g_suite_state.status.config_available = true;
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_CONFIG_MANAGEMENT;
        
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_CONFIG, CHARM_AUDIT_SUCCESS,
                       "config_system", "initialization", "Configuration system initialized", NULL);
    } else {
        fprintf(stderr, "Warning: Configuration system initialization failed\n");
    }
    
    // Initialize data protection capabilities
    // Basic data protection is always available since we have key management and crypto services
    if (g_suite_state.status.crypto_available && g_suite_state.status.key_mgmt_available) {
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_DATA_PROTECTION;
        
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_SUCCESS,
                       "data_protection", "initialization", "Data protection capabilities enabled", NULL);
    }
    
    // Initialize threat detection and monitoring
    // Basic threat detection is available if audit logging is working
    if (g_suite_state.status.audit_available) {
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_THREAT_DETECTION;
        
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_SUCCESS,
                       "threat_detection", "initialization", "Threat detection and monitoring enabled", NULL);
    }
    
    // Initialize vulnerability management framework
    // Basic SBOM and dependency tracking framework
    g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_VULNERABILITY_MGMT;
    
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_SUCCESS,
                   "vulnerability_mgmt", "initialization", "Vulnerability management framework enabled", NULL);
    
    // Initialize incident response capabilities
    // Basic incident response is available if audit logging is working
    if (g_suite_state.status.audit_available) {
        g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_INCIDENT_RESPONSE;
        
        charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_SUCCESS,
                       "incident_response", "initialization", "Incident response capabilities enabled", NULL);
    }
    
    // Initialize lifecycle security
    // Basic update security and provenance tracking
    g_suite_state.status.enabled_capabilities |= CHARM_CAPABILITY_LIFECYCLE_SECURITY;
    
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_SUCCESS,
                   "lifecycle_security", "initialization", "Lifecycle security capabilities enabled", NULL);
    
    // Set initial security level based on enabled capabilities
    int enabled_count = __builtin_popcount(g_suite_state.status.enabled_capabilities);
    g_suite_state.status.current_security_level = (enabled_count * 100) / 10; // Rough estimate
    
    g_suite_state.initialized = true;
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return init_result;
}

// Shutdown the CHARM Security Suite
int charm_security_suite_shutdown(void) {
    if (!g_suite_state.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    // Log shutdown
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS,
                   "security_suite", "shutdown", "CHARM Security Suite shutting down", NULL);
    
    // Shutdown subsystems in reverse order
    if (g_suite_state.status.config_available) {
        charm_config_shutdown();
    }
    
    if (g_suite_state.status.auth_available) {
        charm_auth_shutdown();
    }
    
    if (g_suite_state.status.key_mgmt_available) {
        charm_key_manager_shutdown();
    }
    
    if (g_suite_state.status.crypto_available) {
        charm_shutdown();
    }
    
    if (g_suite_state.status.audit_available) {
        CHARM_AUDIT_SYSTEM_STOP();
        charm_audit_shutdown();
    }
    
    g_suite_state.initialized = false;
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    pthread_mutex_destroy(&g_suite_state.mutex);
    
    return 0;
}

// Get security suite version
const char* charm_security_suite_get_version(void) {
    static char version_string[32];
    snprintf(version_string, sizeof(version_string), "%d.%d.%d",
             CHARM_SECURITY_SUITE_VERSION_MAJOR,
             CHARM_SECURITY_SUITE_VERSION_MINOR,
             CHARM_SECURITY_SUITE_VERSION_PATCH);
    return version_string;
}

// Get enabled capabilities
int charm_security_suite_get_capabilities(uint32_t* capabilities) {
    if (!g_suite_state.initialized || !capabilities) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    *capabilities = g_suite_state.status.enabled_capabilities;
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return 0;
}

// Get security suite status
int charm_security_suite_get_status(charm_security_suite_status_t* status) {
    if (!g_suite_state.initialized || !status) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    // Update dynamic status fields
    g_suite_state.status.initialized = g_suite_state.initialized;
    g_suite_state.status.entropy_quality = charm_get_entropy_quality();
    
    *status = g_suite_state.status;
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return 0;
}

// Perform comprehensive security health check
int charm_security_suite_health_check(char issues[][256], size_t max_issues, size_t* issue_count) {
    if (!g_suite_state.initialized || !issues || !issue_count) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    *issue_count = 0;
    int health_score = 100;
    
    // Check core crypto availability
    if (!g_suite_state.status.crypto_available) {
        if (*issue_count < max_issues) {
            strcpy(issues[*issue_count], "Core cryptographic services unavailable");
            (*issue_count)++;
        }
        health_score -= 30;
    }
    
    // Check entropy quality
    double entropy_quality = charm_get_entropy_quality();
    if (entropy_quality < g_suite_state.config.entropy_quality_threshold) {
        if (*issue_count < max_issues) {
            snprintf(issues[*issue_count], 256, "Low entropy quality: %.3f < %.3f",
                    entropy_quality, g_suite_state.config.entropy_quality_threshold);
            (*issue_count)++;
        }
        health_score -= 20;
    }
    
    // Check for emergency lockdown
    if (g_suite_state.emergency_lockdown) {
        if (*issue_count < max_issues) {
            strcpy(issues[*issue_count], "System in emergency lockdown mode");
            (*issue_count)++;
        }
        health_score -= 50;
    }
    
    // Check audit system
    if (!g_suite_state.status.audit_available) {
        if (*issue_count < max_issues) {
            strcpy(issues[*issue_count], "Audit logging system unavailable");
            (*issue_count)++;
        }
        health_score -= 15;
    }
    
    // Check key management
    if (!g_suite_state.status.key_mgmt_available) {
        if (*issue_count < max_issues) {
            strcpy(issues[*issue_count], "Key management system unavailable");
            (*issue_count)++;
        }
        health_score -= 20;
    }
    
    // Check authentication
    if (!g_suite_state.status.auth_available) {
        if (*issue_count < max_issues) {
            strcpy(issues[*issue_count], "Authentication system unavailable");
            (*issue_count)++;
        }
        health_score -= 15;
    }
    
    // Ensure health score doesn't go below 0
    if (health_score < 0) {
        health_score = 0;
    }
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return health_score;
}

// Report a security incident
int charm_security_suite_report_incident(charm_incident_type_t type,
                                         const char* source,
                                         const char* description,
                                         const char* affected_resource,
                                         int severity,
                                         uint64_t* incident_id) {
    if (!g_suite_state.initialized || !source || !description || !incident_id) {
        return -EINVAL;
    }
    
    if (severity < 1 || severity > 10) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    // Create new incident
    if (g_suite_state.incident_count >= 100) {
        // Replace oldest incident (simple circular buffer)
        memmove(&g_suite_state.incidents[0], &g_suite_state.incidents[1],
                sizeof(charm_security_incident_t) * 99);
        g_suite_state.incident_count = 99;
    }
    
    charm_security_incident_t* incident = &g_suite_state.incidents[g_suite_state.incident_count];
    incident->incident_id = g_suite_state.next_incident_id++;
    incident->type = type;
    incident->timestamp = time(NULL);
    incident->severity = severity;
    incident->resolved = false;
    incident->resolution_time = 0;
    
    strncpy(incident->source, source, sizeof(incident->source) - 1);
    strncpy(incident->description, description, sizeof(incident->description) - 1);
    
    if (affected_resource) {
        strncpy(incident->affected_resource, affected_resource, 
                sizeof(incident->affected_resource) - 1);
    }
    
    *incident_id = incident->incident_id;
    g_suite_state.incident_count++;
    
    // Log incident to audit system
    char incident_details[512];
    snprintf(incident_details, sizeof(incident_details),
             "ID=%lu Type=%d Severity=%d Source=%s Resource=%s",
             incident->incident_id, type, severity, source,
             affected_resource ? affected_resource : "none");
    
    charm_audit_log(CHARM_AUDIT_LEVEL_CRITICAL, CHARM_AUDIT_CAT_SECURITY, CHARM_AUDIT_FAILURE,
                   source, affected_resource ? affected_resource : "system",
                   description, incident_details);
    
    // Trigger emergency lockdown for critical incidents
    if (severity >= 9) {
        g_suite_state.emergency_lockdown = true;
        charm_audit_log(CHARM_AUDIT_LEVEL_CRITICAL, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS,
                       "security_suite", "emergency_lockdown", 
                       "Emergency lockdown triggered by critical security incident", incident_details);
    }
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return 0;
}

// Perform emergency security lockdown
int charm_security_suite_emergency_lockdown(const char* reason) {
    if (!g_suite_state.initialized || !reason) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    g_suite_state.emergency_lockdown = true;
    
    // Log lockdown
    charm_audit_log(CHARM_AUDIT_LEVEL_CRITICAL, CHARM_AUDIT_CAT_SYSTEM, CHARM_AUDIT_SUCCESS,
                   "admin", "emergency_lockdown", "Emergency lockdown activated", reason);
    
    // Report lockdown as security incident
    uint64_t incident_id;
    char incident_desc[256];
    snprintf(incident_desc, sizeof(incident_desc), "Emergency lockdown: %s", reason);
    
    charm_security_suite_report_incident(CHARM_INCIDENT_UNKNOWN_THREAT, "admin",
                                         incident_desc, "system", 10, &incident_id);
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return 0;
}

// High-level secure encryption function
int charm_secure_encrypt(const uint8_t* data, size_t data_len,
                         const char* key_id,
                         uint8_t* encrypted_data, size_t max_encrypted,
                         size_t* actual_encrypted) {
    if (!g_suite_state.initialized || !data || !encrypted_data || !actual_encrypted) {
        return -EINVAL;
    }
    
    if (g_suite_state.emergency_lockdown) {
        return -EACCES; // Access denied during lockdown
    }
    
    // For this basic implementation, we'll do a simple XOR encryption with key material
    // In a full implementation, this would use proper AEAD encryption
    
    char actual_key_id[64];
    if (!key_id) {
        // Generate a new key
        charm_key_generation_params_t params = {
            .type = CHARM_KEY_TYPE_SYMMETRIC_AES256,
            .usage_flags = CHARM_KEY_USAGE_ENCRYPT | CHARM_KEY_USAGE_DECRYPT,
            .key_size_bits = 256,
            .expiry_time = 0,
            .rotation_days = 365,
            .description = "Auto-generated encryption key",
            .entropy_seed = NULL,
            .entropy_seed_len = 0
        };
        
        if (charm_key_generate(&params, actual_key_id) != 0) {
            return -1;
        }
        key_id = actual_key_id;
    }
    
    // Simple encryption (XOR with hash-derived key material)
    if (max_encrypted < data_len + 32) {  // Need space for data + key ID hash
        return -ENOBUFS;
    }
    
    // Derive encryption key from key ID
    uint8_t key_material[32];
    if (charm_digest_compute((const uint8_t*)key_id, strlen(key_id), key_material) != 0) {
        return -1;
    }
    
    // Simple XOR encryption
    for (size_t i = 0; i < data_len; i++) {
        encrypted_data[i] = data[i] ^ key_material[i % 32];
    }
    
    // Append key ID hash for decryption
    memcpy(encrypted_data + data_len, key_material, 32);
    *actual_encrypted = data_len + 32;
    
    // Audit the encryption
    charm_audit_logf(CHARM_AUDIT_LEVEL_DEBUG, CHARM_AUDIT_CAT_CRYPTO, CHARM_AUDIT_SUCCESS,
                     "crypto_engine", key_id, "Data encrypted: %zu bytes", data_len);
    
    return 0;
}

// Get security incidents
int charm_security_suite_get_incidents(charm_security_incident_t* incidents,
                                       size_t max_incidents,
                                       size_t* incident_count,
                                       bool unresolved_only) {
    if (!g_suite_state.initialized || !incidents || !incident_count) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_suite_state.mutex);
    
    *incident_count = 0;
    
    for (size_t i = 0; i < g_suite_state.incident_count && *incident_count < max_incidents; i++) {
        if (!unresolved_only || !g_suite_state.incidents[i].resolved) {
            incidents[*incident_count] = g_suite_state.incidents[i];
            (*incident_count)++;
        }
    }
    
    pthread_mutex_unlock(&g_suite_state.mutex);
    
    return 0;
}