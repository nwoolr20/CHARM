/**
 * @file test_security_suite.c
 * @brief Test program for CHARM Security Suite
 */

#include "charm_security_suite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int main(void) {
    printf("CHARM Security Suite Test\n");
    printf("========================\n\n");
    
    // Initialize the security suite
    printf("1. Initializing CHARM Security Suite...\n");
    int ret = charm_security_suite_init(NULL); // Use default config
    if (ret != 0) {
        printf("   ❌ Initialization failed: %d\n", ret);
        return 1;
    }
    printf("   ✅ Security suite initialized successfully\n\n");
    
    // Get version and capabilities
    printf("2. Security Suite Information:\n");
    printf("   Version: %s\n", charm_security_suite_get_version());
    
    uint32_t capabilities;
    if (charm_security_suite_get_capabilities(&capabilities) == 0) {
        printf("   Enabled capabilities: 0x%08X\n", capabilities);
        
        if (capabilities & CHARM_CAPABILITY_CRYPTOGRAPHIC_SERVICES) {
            printf("     - Cryptographic Services ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_KEY_MANAGEMENT) {
            printf("     - Key Management ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_IDENTITY_ACCESS) {
            printf("     - Identity & Access ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_DATA_PROTECTION) {
            printf("     - Data Protection ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_AUDIT_LOGGING) {
            printf("     - Audit Logging ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_CONFIG_MANAGEMENT) {
            printf("     - Configuration Management ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_THREAT_DETECTION) {
            printf("     - Threat Detection ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_VULNERABILITY_MGMT) {
            printf("     - Vulnerability Management ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_INCIDENT_RESPONSE) {
            printf("     - Incident Response ✅\n");
        }
        if (capabilities & CHARM_CAPABILITY_LIFECYCLE_SECURITY) {
            printf("     - Lifecycle Security ✅\n");
        }
    }
    printf("\n");
    
    // Perform health check
    printf("3. Security Health Check:\n");
    char issues[10][256];
    size_t issue_count = 0;  // Initialize to 0
    int health_score = charm_security_suite_health_check(issues, 10, &issue_count);
    
    printf("   Health Score: %d/100\n", health_score);
    if (issue_count > 0) {
        printf("   Issues found:\n");
        for (size_t i = 0; i < issue_count; i++) {
            printf("     - %s\n", issues[i]);
        }
    } else {
        printf("   No issues found ✅\n");
    }
    printf("\n");
    
    // Test authentication
    printf("4. Testing Authentication:\n");
    charm_auth_context_t auth_context;
    charm_auth_result_t auth_result = charm_auth_authenticate(CHARM_AUTH_METHOD_PASSWORD,
                                                             "admin:admin123",
                                                             &auth_context);
    
    if (auth_result == CHARM_AUTH_SUCCESS) {
        printf("   ✅ Authentication successful\n");
        printf("     Identity: %s\n", auth_context.identity);
        printf("     Roles: %s\n", auth_context.roles[0]);
        printf("     Session Token: %.32s...\n", auth_context.token);
    } else {
        printf("   ❌ Authentication failed: %d\n", auth_result);
    }
    printf("\n");
    
    // Test key generation
    printf("5. Testing Key Management:\n");
    charm_key_generation_params_t key_params = {
        .type = CHARM_KEY_TYPE_SYMMETRIC_AES256,
        .usage_flags = CHARM_KEY_USAGE_ENCRYPT | CHARM_KEY_USAGE_DECRYPT,
        .key_size_bits = 256,
        .expiry_time = 0,
        .rotation_days = 30,
        .description = "Test encryption key",
        .entropy_seed = NULL,
        .entropy_seed_len = 0
    };
    
    char key_id[64];
    ret = charm_key_generate(&key_params, key_id);
    if (ret == 0) {
        printf("   ✅ Key generated successfully\n");
        printf("     Key ID: %s\n", key_id);
        
        // Test key metadata retrieval
        charm_key_metadata_t metadata;
        if (charm_key_get_metadata(key_id, &metadata) == 0) {
            printf("     Type: %d\n", metadata.type);
            printf("     Usage: 0x%08X\n", metadata.usage_flags);
            printf("     Created: %ld\n", metadata.created);
        }
    } else {
        printf("   ❌ Key generation failed: %d\n", ret);
    }
    printf("\n");
    
    // Test authorization
    if (auth_result == CHARM_AUTH_SUCCESS) {
        printf("6. Testing Authorization:\n");
        charm_authz_request_t authz_request;
        ret = charm_auth_create_request(auth_context.identity, "test_resource", 
                                       CHARM_ACTION_READ, NULL, &authz_request);
        
        if (ret == 0) {
            int authorized = charm_auth_authorize(&auth_context, &authz_request);
            if (authorized) {
                printf("   ✅ Authorization granted for READ access\n");
            } else {
                printf("   ❌ Authorization denied for READ access\n");
            }
            
            // Test WRITE authorization
            authz_request.action = CHARM_ACTION_WRITE;
            authorized = charm_auth_authorize(&auth_context, &authz_request);
            if (authorized) {
                printf("   ✅ Authorization granted for WRITE access\n");
            } else {
                printf("   ❌ Authorization denied for WRITE access\n");
            }
        }
        printf("\n");
    }
    
    // Test secure encryption/decryption
    printf("7. Testing Secure Encryption:\n");
    const char* test_data = "This is a test message for encryption";
    uint8_t encrypted[1024];
    size_t encrypted_len;
    
    ret = charm_secure_encrypt((const uint8_t*)test_data, strlen(test_data),
                              NULL, encrypted, sizeof(encrypted), &encrypted_len);
    
    if (ret == 0) {
        printf("   ✅ Encryption successful\n");
        printf("     Original length: %zu bytes\n", strlen(test_data));
        printf("     Encrypted length: %zu bytes\n", encrypted_len);
    } else {
        printf("   ❌ Encryption failed: %d\n", ret);
    }
    printf("\n");
    
    // Test incident reporting
    printf("8. Testing Incident Reporting:\n");
    uint64_t incident_id;
    ret = charm_security_suite_report_incident(CHARM_INCIDENT_AUTHENTICATION_FAILURE,
                                              "test_program",
                                              "Test security incident",
                                              "test_resource",
                                              5,  // Medium severity
                                              &incident_id);
    
    if (ret == 0) {
        printf("   ✅ Incident reported successfully\n");
        printf("     Incident ID: %lu\n", incident_id);
        
        // Get incidents list
        charm_security_incident_t incidents[10];
        size_t incident_count;
        ret = charm_security_suite_get_incidents(incidents, 10, &incident_count, false);
        
        if (ret == 0 && incident_count > 0) {
            printf("     Total incidents: %zu\n", incident_count);
            printf("     Latest incident: %s\n", incidents[incident_count-1].description);
        }
    } else {
        printf("   ❌ Incident reporting failed: %d\n", ret);
    }
    printf("\n");
    
    // Final status check
    printf("9. Final Status Check:\n");
    charm_security_suite_status_t status;
    if (charm_security_suite_get_status(&status) == 0) {
        printf("   Initialized: %s\n", status.initialized ? "Yes" : "No");
        printf("   Security Level: %d/100\n", status.current_security_level);
        printf("   Entropy Quality: %.3f\n", status.entropy_quality);
        printf("   Total Operations: %lu\n", status.total_crypto_operations);
        printf("   Total Auth Attempts: %lu\n", status.total_auth_attempts);
        printf("   Total Audit Events: %lu\n", status.total_audit_events);
    }
    printf("\n");
    
    // Shutdown
    printf("10. Shutting down Security Suite...\n");
    ret = charm_security_suite_shutdown();
    if (ret == 0) {
        printf("    ✅ Shutdown successful\n");
    } else {
        printf("    ❌ Shutdown failed: %d\n", ret);
    }
    
    printf("\nCHARM Security Suite test completed!\n");
    return 0;
}