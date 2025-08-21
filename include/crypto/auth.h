#ifndef CHARM_AUTH_H
#define CHARM_AUTH_H

/**
 * @file auth.h
 * @brief CHARM Authentication and Authorization Framework
 * 
 * Provides pluggable authentication adapters and policy-based authorization
 * for the CHARM security suite.
 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Authentication methods
typedef enum {
    CHARM_AUTH_METHOD_NONE = 0,
    CHARM_AUTH_METHOD_PASSWORD,
    CHARM_AUTH_METHOD_TOKEN,
    CHARM_AUTH_METHOD_JWT,
    CHARM_AUTH_METHOD_MTLS,
    CHARM_AUTH_METHOD_OAUTH2,
    CHARM_AUTH_METHOD_OIDC,
    CHARM_AUTH_METHOD_API_KEY,
    CHARM_AUTH_METHOD_CUSTOM
} charm_auth_method_t;

// Authorization policies
typedef enum {
    CHARM_AUTHZ_POLICY_ALLOW_ALL = 0,
    CHARM_AUTHZ_POLICY_DENY_ALL,
    CHARM_AUTHZ_POLICY_RBAC,      // Role-based access control
    CHARM_AUTHZ_POLICY_ABAC,      // Attribute-based access control
    CHARM_AUTHZ_POLICY_CUSTOM
} charm_authz_policy_type_t;

// Authentication result
typedef enum {
    CHARM_AUTH_SUCCESS = 0,
    CHARM_AUTH_FAILURE = 1,
    CHARM_AUTH_EXPIRED = 2,
    CHARM_AUTH_REVOKED = 3,
    CHARM_AUTH_INSUFFICIENT = 4
} charm_auth_result_t;

// Authorization actions
typedef enum {
    CHARM_ACTION_READ = 0x01,
    CHARM_ACTION_WRITE = 0x02,
    CHARM_ACTION_DELETE = 0x04,
    CHARM_ACTION_EXECUTE = 0x08,
    CHARM_ACTION_ADMIN = 0x10
} charm_action_t;

// Maximum field sizes
#define CHARM_AUTH_MAX_IDENTITY 64
#define CHARM_AUTH_MAX_ROLE 32
#define CHARM_AUTH_MAX_RESOURCE 128
#define CHARM_AUTH_MAX_CONTEXT 256
#define CHARM_AUTH_MAX_TOKEN 512

// Authentication context
typedef struct {
    char identity[CHARM_AUTH_MAX_IDENTITY];
    char roles[8][CHARM_AUTH_MAX_ROLE];      // Up to 8 roles
    size_t role_count;
    time_t auth_time;
    time_t expires;
    charm_auth_method_t method;
    char token[CHARM_AUTH_MAX_TOKEN];
    uint8_t session_id[32];
    char source_ip[46];                       // IPv6 max length
    char user_agent[128];
} charm_auth_context_t;

// Authorization request
typedef struct {
    const char* subject;                      // Who is requesting
    const char* resource;                     // What resource
    uint32_t action;                          // What action (bitmask)
    const char* context;                      // Additional context
    time_t request_time;
} charm_authz_request_t;

// Authentication adapter interface
typedef struct charm_auth_adapter {
    const char* name;
    charm_auth_method_t method;
    
    // Initialize adapter
    int (*init)(const char* config);
    
    // Authenticate user
    int (*authenticate)(const char* credentials, charm_auth_context_t* context);
    
    // Validate existing session/token
    int (*validate)(const char* token, charm_auth_context_t* context);
    
    // Logout/invalidate session
    int (*logout)(const char* token);
    
    // Cleanup adapter
    void (*cleanup)(void);
    
    struct charm_auth_adapter* next;
} charm_auth_adapter_t;

// Authorization policy interface
typedef struct charm_authz_policy {
    const char* name;
    charm_authz_policy_type_t type;
    
    // Initialize policy
    int (*init)(const char* config);
    
    // Check authorization
    int (*authorize)(const charm_auth_context_t* context, 
                     const charm_authz_request_t* request);
    
    // Update policy rules at runtime
    int (*update_rules)(const char* rules);
    
    // Cleanup policy
    void (*cleanup)(void);
    
    struct charm_authz_policy* next;
} charm_authz_policy_t;

/**
 * Initialize the authentication/authorization subsystem
 * @return 0 on success, negative on error
 */
int charm_auth_init(void);

/**
 * Shutdown the authentication/authorization subsystem
 * @return 0 on success, negative on error
 */
int charm_auth_shutdown(void);

/**
 * Register an authentication adapter
 * @param adapter Authentication adapter to register
 * @return 0 on success, negative on error
 */
int charm_auth_register_adapter(charm_auth_adapter_t* adapter);

/**
 * Register an authorization policy
 * @param policy Authorization policy to register
 * @return 0 on success, negative on error
 */
int charm_auth_register_policy(charm_authz_policy_t* policy);

/**
 * Set the active authentication method
 * @param method Authentication method to use
 * @return 0 on success, negative on error
 */
int charm_auth_set_method(charm_auth_method_t method);

/**
 * Set the active authorization policy
 * @param policy_type Authorization policy to use
 * @return 0 on success, negative on error
 */
int charm_auth_set_policy(charm_authz_policy_type_t policy_type);

/**
 * Authenticate a user
 * @param method Authentication method to try
 * @param credentials Authentication credentials
 * @param context Output authentication context
 * @return Authentication result
 */
charm_auth_result_t charm_auth_authenticate(charm_auth_method_t method,
                                           const char* credentials,
                                           charm_auth_context_t* context);

/**
 * Validate an existing authentication token/session
 * @param token Token to validate
 * @param context Output authentication context if valid
 * @return Authentication result
 */
charm_auth_result_t charm_auth_validate(const char* token, charm_auth_context_t* context);

/**
 * Check authorization for a request
 * @param context Authentication context
 * @param request Authorization request
 * @return 1 if authorized, 0 if denied, negative on error
 */
int charm_auth_authorize(const charm_auth_context_t* context,
                         const charm_authz_request_t* request);

/**
 * Logout/invalidate a session
 * @param token Token to invalidate
 * @return 0 on success, negative on error
 */
int charm_auth_logout(const char* token);

/**
 * Create an authorization request
 * @param subject Subject (user/identity)
 * @param resource Resource being accessed
 * @param action Action being performed
 * @param context Additional context
 * @param request Output request structure
 * @return 0 on success, negative on error
 */
int charm_auth_create_request(const char* subject,
                              const char* resource,
                              uint32_t action,
                              const char* context,
                              charm_authz_request_t* request);

/**
 * Get authentication context for current session
 * @param context Output authentication context
 * @return 0 on success, negative on error
 */
int charm_auth_get_current_context(charm_auth_context_t* context);

/**
 * Check if user has specific role
 * @param context Authentication context
 * @param role Role name to check
 * @return 1 if has role, 0 if not, negative on error
 */
int charm_auth_has_role(const charm_auth_context_t* context, const char* role);

/**
 * Add role to authentication context
 * @param context Authentication context to modify
 * @param role Role name to add
 * @return 0 on success, negative on error
 */
int charm_auth_add_role(charm_auth_context_t* context, const char* role);

/**
 * Remove role from authentication context
 * @param context Authentication context to modify
 * @param role Role name to remove
 * @return 0 on success, negative on error
 */
int charm_auth_remove_role(charm_auth_context_t* context, const char* role);

// Built-in authentication adapters
extern charm_auth_adapter_t charm_auth_adapter_password;
extern charm_auth_adapter_t charm_auth_adapter_token;
extern charm_auth_adapter_t charm_auth_adapter_jwt;
extern charm_auth_adapter_t charm_auth_adapter_api_key;

// Built-in authorization policies
extern charm_authz_policy_t charm_authz_policy_rbac;
extern charm_authz_policy_t charm_authz_policy_allow_all;
extern charm_authz_policy_t charm_authz_policy_deny_all;

// Convenience macros
#define CHARM_AUTH_REQUIRE(context, resource, action) \
    do { \
        charm_authz_request_t req; \
        charm_auth_create_request((context)->identity, resource, action, NULL, &req); \
        if (charm_auth_authorize(context, &req) != 1) { \
            return -EACCES; \
        } \
    } while(0)

#define CHARM_AUTH_CHECK_ROLE(context, required_role) \
    do { \
        if (!charm_auth_has_role(context, required_role)) { \
            return -EACCES; \
        } \
    } while(0)

#ifdef __cplusplus
}
#endif

#endif // CHARM_AUTH_H