// Add missing includes
#define _GNU_SOURCE
#include "crypto/auth.h"
#include "crypto/audit.h"
#include "charm_api.h"
#include "system_entropy.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

// Internal state
typedef struct {
    int initialized;
    charm_auth_method_t active_method;
    charm_authz_policy_type_t active_policy;
    charm_auth_adapter_t* adapters;
    charm_authz_policy_t* policies;
    charm_auth_context_t current_context;
    pthread_mutex_t mutex;
} charm_auth_state_t;

static charm_auth_state_t g_auth_state = {0};

// Built-in password adapter
static int password_adapter_init(const char* config) {
    // Simple password adapter - in production this would connect to password database
    (void)config; // Unused for simple implementation
    return 0;
}

static int password_adapter_authenticate(const char* credentials, charm_auth_context_t* context) {
    if (!credentials || !context) {
        return CHARM_AUTH_FAILURE;
    }
    
    // Parse credentials (format: "username:password")
    char* cred_copy = strdup(credentials);
    if (!cred_copy) {
        return CHARM_AUTH_FAILURE;
    }
    char* username = strtok(cred_copy, ":");
    char* password = strtok(NULL, ":");
    
    if (!username || !password) {
        free(cred_copy);
        return CHARM_AUTH_FAILURE;
    }
    
    // Simple password validation (in production: hash comparison, rate limiting, etc.)
    int valid = 0;
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        valid = 1;
        strncpy(context->identity, username, sizeof(context->identity) - 1);
        strncpy(context->roles[0], "admin", sizeof(context->roles[0]) - 1);
        context->role_count = 1;
    } else if (strcmp(username, "user") == 0 && strcmp(password, "user123") == 0) {
        valid = 1;
        strncpy(context->identity, username, sizeof(context->identity) - 1);
        strncpy(context->roles[0], "user", sizeof(context->roles[0]) - 1);
        context->role_count = 1;
    }
    
    if (valid) {
        context->auth_time = time(NULL);
        context->expires = context->auth_time + 3600; // 1 hour session
        context->method = CHARM_AUTH_METHOD_PASSWORD;
        
        // Generate session token
        uint8_t token_seed[64];
        // Use cryptographically secure entropy instead of rand()
        uint8_t random_bytes[16];
        system_entropy_extract_fast(random_bytes, sizeof(random_bytes));
        
        snprintf((char*)token_seed, sizeof(token_seed), "%s:%ld:", 
                username, context->auth_time);
        // Append the secure random bytes as hex
        for (int i = 0; i < 16; i++) {
            snprintf((char*)token_seed + strlen((char*)token_seed), 
                    sizeof(token_seed) - strlen((char*)token_seed), "%02x", random_bytes[i]);
        }
        
        uint8_t token_hash[32];
        if (charm_digest_compute(token_seed, strlen((char*)token_seed), token_hash) == 0) {
            charm_digest_to_hex(token_hash, context->token);
        }
        
        // Generate session ID
        uint8_t session_seed[64];
        snprintf((char*)session_seed, sizeof(session_seed), "session:%s:%ld", 
                username, context->auth_time);
        charm_digest_compute(session_seed, strlen((char*)session_seed), context->session_id);
    }
    
    free(cred_copy);
    return valid ? CHARM_AUTH_SUCCESS : CHARM_AUTH_FAILURE;
}

static int password_adapter_validate(const char* token, charm_auth_context_t* context) {
    // Simple token validation - in production this would check against session store
    if (!token || !context) {
        return CHARM_AUTH_FAILURE;
    }
    
    // For this simple implementation, assume valid if token matches current context
    if (strlen(token) > 0 && g_auth_state.current_context.token[0] != '\0') {
        if (strcmp(token, g_auth_state.current_context.token) == 0) {
            if (time(NULL) < g_auth_state.current_context.expires) {
                *context = g_auth_state.current_context;
                return CHARM_AUTH_SUCCESS;
            } else {
                return CHARM_AUTH_EXPIRED;
            }
        }
    }
    
    return CHARM_AUTH_FAILURE;
}

static int password_adapter_logout(const char* token) {
    // Simple logout - clear current context if token matches
    if (token && g_auth_state.current_context.token[0] != '\0' && 
        strcmp(token, g_auth_state.current_context.token) == 0) {
        memset(&g_auth_state.current_context, 0, sizeof(g_auth_state.current_context));
        return 0;
    }
    return -1;
}

static void password_adapter_cleanup(void) {
    // Nothing to clean up for simple implementation
}

// Built-in password adapter
charm_auth_adapter_t charm_auth_adapter_password = {
    .name = "password",
    .method = CHARM_AUTH_METHOD_PASSWORD,
    .init = password_adapter_init,
    .authenticate = password_adapter_authenticate,
    .validate = password_adapter_validate,
    .logout = password_adapter_logout,
    .cleanup = password_adapter_cleanup,
    .next = NULL
};

// Built-in RBAC policy
static int rbac_policy_init(const char* config) {
    (void)config; // Simple RBAC doesn't need config
    return 0;
}

static int rbac_policy_authorize(const charm_auth_context_t* context, 
                                const charm_authz_request_t* request) {
    if (!context || !request) {
        return 0; // Deny
    }
    
    // Simple RBAC rules
    for (size_t i = 0; i < context->role_count; i++) {
        const char* role = context->roles[i];
        
        if (strcmp(role, "admin") == 0) {
            return 1; // Admin can do anything
        }
        
        if (strcmp(role, "user") == 0) {
            // Users can read most things
            if (request->action & CHARM_ACTION_READ) {
                return 1;
            }
            // Users can write to their own resources
            if ((request->action & CHARM_ACTION_WRITE) && 
                strstr(request->resource, context->identity)) {
                return 1;
            }
        }
    }
    
    return 0; // Deny by default
}

static int rbac_policy_update_rules(const char* rules) {
    // Simple implementation doesn't support dynamic rule updates
    (void)rules;
    return -ENOTSUP;
}

static void rbac_policy_cleanup(void) {
    // Nothing to clean up
}

// Built-in RBAC policy
charm_authz_policy_t charm_authz_policy_rbac = {
    .name = "rbac",
    .type = CHARM_AUTHZ_POLICY_RBAC,
    .init = rbac_policy_init,
    .authorize = rbac_policy_authorize,
    .update_rules = rbac_policy_update_rules,
    .cleanup = rbac_policy_cleanup,
    .next = NULL
};

// Allow-all policy (for testing/development)
static int allow_all_policy_init(const char* config) {
    (void)config;
    return 0;
}

static int allow_all_policy_authorize(const charm_auth_context_t* context, 
                                     const charm_authz_request_t* request) {
    (void)context;
    (void)request;
    return 1; // Always allow
}

charm_authz_policy_t charm_authz_policy_allow_all = {
    .name = "allow_all",
    .type = CHARM_AUTHZ_POLICY_ALLOW_ALL,
    .init = allow_all_policy_init,
    .authorize = allow_all_policy_authorize,
    .update_rules = NULL,
    .cleanup = NULL,
    .next = NULL
};

// Deny-all policy
static int deny_all_policy_authorize(const charm_auth_context_t* context, 
                                    const charm_authz_request_t* request) {
    (void)context;
    (void)request;
    return 0; // Always deny
}

charm_authz_policy_t charm_authz_policy_deny_all = {
    .name = "deny_all",
    .type = CHARM_AUTHZ_POLICY_DENY_ALL,
    .init = allow_all_policy_init, // Same as allow_all
    .authorize = deny_all_policy_authorize,
    .update_rules = NULL,
    .cleanup = NULL,
    .next = NULL
};

// Initialize authentication/authorization subsystem
int charm_auth_init(void) {
    if (g_auth_state.initialized) {
        return 0; // Already initialized
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&g_auth_state.mutex, NULL) != 0) {
        return -1;
    }
    
    // Register built-in adapters and policies
    g_auth_state.adapters = &charm_auth_adapter_password;
    g_auth_state.policies = &charm_authz_policy_rbac;
    
    // Set default method and policy
    g_auth_state.active_method = CHARM_AUTH_METHOD_PASSWORD;
    g_auth_state.active_policy = CHARM_AUTHZ_POLICY_RBAC;
    
    // Initialize built-in components
    charm_auth_adapter_password.init(NULL);
    charm_authz_policy_rbac.init(NULL);
    
    g_auth_state.initialized = 1;
    
    // Audit log initialization
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_SUCCESS,
                    "auth_system", "initialization", "Authentication system initialized", NULL);
    
    return 0;
}

// Shutdown authentication/authorization subsystem
int charm_auth_shutdown(void) {
    if (!g_auth_state.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_auth_state.mutex);
    
    // Cleanup adapters and policies
    charm_auth_adapter_password.cleanup();
    charm_authz_policy_rbac.cleanup();
    
    // Clear state
    memset(&g_auth_state.current_context, 0, sizeof(g_auth_state.current_context));
    g_auth_state.initialized = 0;
    
    pthread_mutex_unlock(&g_auth_state.mutex);
    pthread_mutex_destroy(&g_auth_state.mutex);
    
    // Audit log shutdown
    charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_SUCCESS,
                    "auth_system", "shutdown", "Authentication system shutdown", NULL);
    
    return 0;
}

// Authenticate a user
charm_auth_result_t charm_auth_authenticate(charm_auth_method_t method,
                                           const char* credentials,
                                           charm_auth_context_t* context) {
    if (!g_auth_state.initialized || !credentials || !context) {
        return CHARM_AUTH_FAILURE;
    }
    
    pthread_mutex_lock(&g_auth_state.mutex);
    
    charm_auth_result_t result = CHARM_AUTH_FAILURE;
    
    // Find appropriate adapter
    charm_auth_adapter_t* adapter = g_auth_state.adapters;
    while (adapter) {
        if (adapter->method == method) {
            result = adapter->authenticate(credentials, context);
            if (result == CHARM_AUTH_SUCCESS) {
                // Store as current context
                g_auth_state.current_context = *context;
                
                // Audit successful authentication
                charm_audit_log(CHARM_AUDIT_LEVEL_INFO, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_SUCCESS,
                               context->identity, "authentication", "User authenticated", NULL);
            } else {
                // Audit failed authentication
                charm_audit_log(CHARM_AUDIT_LEVEL_WARN, CHARM_AUDIT_CAT_AUTH, CHARM_AUDIT_FAILURE,
                               "unknown", "authentication", "Authentication failed", credentials);
            }
            break;
        }
        adapter = adapter->next;
    }
    
    pthread_mutex_unlock(&g_auth_state.mutex);
    return result;
}

// Check authorization for a request
int charm_auth_authorize(const charm_auth_context_t* context,
                         const charm_authz_request_t* request) {
    if (!g_auth_state.initialized || !context || !request) {
        return 0; // Deny
    }
    
    pthread_mutex_lock(&g_auth_state.mutex);
    
    int authorized = 0;
    
    // Use RBAC policy for authorization
    authorized = charm_authz_policy_rbac.authorize(context, request);
    
    // Audit authorization decision
    charm_audit_logf(CHARM_AUDIT_LEVEL_DEBUG, CHARM_AUDIT_CAT_AUTH, 
                     authorized ? CHARM_AUDIT_SUCCESS : CHARM_AUDIT_FAILURE,
                     context->identity, request->resource,
                     "Authorization %s for action %d", 
                     authorized ? "granted" : "denied", request->action);
    
    pthread_mutex_unlock(&g_auth_state.mutex);
    return authorized;
}

// Create an authorization request
int charm_auth_create_request(const char* subject,
                              const char* resource,
                              uint32_t action,
                              const char* context_str,
                              charm_authz_request_t* request) {
    if (!request) {
        return -EINVAL;
    }
    
    request->subject = subject;
    request->resource = resource;
    request->action = action;
    request->context = context_str;
    request->request_time = time(NULL);
    
    return 0;
}

// Check if user has specific role
int charm_auth_has_role(const charm_auth_context_t* context, const char* role) {
    if (!context || !role) {
        return 0;
    }
    
    for (size_t i = 0; i < context->role_count; i++) {
        if (strcmp(context->roles[i], role) == 0) {
            return 1;
        }
    }
    
    return 0;
}

// Get current authentication context
int charm_auth_get_current_context(charm_auth_context_t* context) {
    if (!g_auth_state.initialized || !context) {
        return -EINVAL;
    }
    
    pthread_mutex_lock(&g_auth_state.mutex);
    *context = g_auth_state.current_context;
    pthread_mutex_unlock(&g_auth_state.mutex);
    
    return 0;
}