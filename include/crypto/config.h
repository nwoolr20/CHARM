#ifndef CHARM_CONFIG_H
#define CHARM_CONFIG_H

/**
 * @file config.h
 * @brief CHARM Secure Configuration Management
 * 
 * Provides secure configuration loading, validation, and runtime management
 * with integrity protection and security policy enforcement.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Configuration value types
typedef enum {
    CHARM_CONFIG_TYPE_STRING = 0,
    CHARM_CONFIG_TYPE_INTEGER,
    CHARM_CONFIG_TYPE_BOOLEAN,
    CHARM_CONFIG_TYPE_FLOAT,
    CHARM_CONFIG_TYPE_ARRAY,
    CHARM_CONFIG_TYPE_OBJECT
} charm_config_type_t;

// Configuration security levels
typedef enum {
    CHARM_CONFIG_SECURITY_PUBLIC = 0,      // No encryption
    CHARM_CONFIG_SECURITY_PROTECTED,       // Encrypted at rest
    CHARM_CONFIG_SECURITY_SECRET,          // Encrypted + access control
    CHARM_CONFIG_SECURITY_TOP_SECRET       // Encrypted + HSM storage
} charm_config_security_t;

// Configuration validation rules
typedef enum {
    CHARM_CONFIG_RULE_NONE = 0x00,
    CHARM_CONFIG_RULE_REQUIRED = 0x01,
    CHARM_CONFIG_RULE_MIN_LENGTH = 0x02,
    CHARM_CONFIG_RULE_MAX_LENGTH = 0x04,
    CHARM_CONFIG_RULE_REGEX = 0x08,
    CHARM_CONFIG_RULE_RANGE = 0x10,
    CHARM_CONFIG_RULE_ENUM = 0x20,
    CHARM_CONFIG_RULE_CUSTOM = 0x40
} charm_config_rule_t;

// Maximum sizes
#define CHARM_CONFIG_MAX_KEY 128
#define CHARM_CONFIG_MAX_STRING 1024
#define CHARM_CONFIG_MAX_DESCRIPTION 256
#define CHARM_CONFIG_MAX_PATTERN 128

// Configuration value
typedef struct {
    charm_config_type_t type;
    charm_config_security_t security_level;
    bool read_only;
    bool runtime_modifiable;
    char description[CHARM_CONFIG_MAX_DESCRIPTION];
    
    union {
        char string_val[CHARM_CONFIG_MAX_STRING];
        int64_t int_val;
        bool bool_val;
        double float_val;
    } value;
    
    // Validation rules
    uint32_t validation_rules;
    int64_t min_value;
    int64_t max_value;
    size_t min_length;
    size_t max_length;
    char regex_pattern[CHARM_CONFIG_MAX_PATTERN];
    char enum_values[8][64];  // Up to 8 enum options
    size_t enum_count;
    
    // Integrity protection
    uint8_t value_hash[32];
    time_t last_modified;
    char modified_by[64];
} charm_config_value_t;

// Configuration schema entry
typedef struct {
    char key[CHARM_CONFIG_MAX_KEY];
    charm_config_type_t type;
    charm_config_security_t security_level;
    uint32_t validation_rules;
    const char* default_value;
    const char* description;
    const char* validation_pattern;
    int64_t min_value;
    int64_t max_value;
} charm_config_schema_t;

// Configuration change event
typedef struct {
    char key[CHARM_CONFIG_MAX_KEY];
    charm_config_value_t old_value;
    charm_config_value_t new_value;
    time_t change_time;
    char changed_by[64];
    char reason[256];
} charm_config_change_t;

// Configuration validation result
typedef struct {
    bool valid;
    char error_message[256];
    char failed_key[CHARM_CONFIG_MAX_KEY];
    uint32_t failed_rule;
} charm_config_validation_result_t;

/**
 * Initialize the configuration management system
 * @param config_file Path to configuration file
 * @param schema_file Path to configuration schema file
 * @return 0 on success, negative on error
 */
int charm_config_init(const char* config_file, const char* schema_file);

/**
 * Shutdown the configuration management system
 * @return 0 on success, negative on error
 */
int charm_config_shutdown(void);

/**
 * Load configuration from file
 * @param config_file Path to configuration file
 * @return 0 on success, negative on error
 */
int charm_config_load(const char* config_file);

/**
 * Save configuration to file
 * @param config_file Path to save configuration to
 * @return 0 on success, negative on error
 */
int charm_config_save(const char* config_file);

/**
 * Get configuration value as string
 * @param key Configuration key
 * @param value Output buffer for value
 * @param max_len Maximum length of output buffer
 * @return 0 on success, negative on error
 */
int charm_config_get_string(const char* key, char* value, size_t max_len);

/**
 * Get configuration value as integer
 * @param key Configuration key
 * @param value Output value
 * @return 0 on success, negative on error
 */
int charm_config_get_int(const char* key, int64_t* value);

/**
 * Get configuration value as boolean
 * @param key Configuration key
 * @param value Output value
 * @return 0 on success, negative on error
 */
int charm_config_get_bool(const char* key, bool* value);

/**
 * Get configuration value as float
 * @param key Configuration key
 * @param value Output value
 * @return 0 on success, negative on error
 */
int charm_config_get_float(const char* key, double* value);

/**
 * Set configuration value from string
 * @param key Configuration key
 * @param value Value to set
 * @param changed_by Identity of who made the change
 * @return 0 on success, negative on error
 */
int charm_config_set_string(const char* key, const char* value, const char* changed_by);

/**
 * Set configuration value from integer
 * @param key Configuration key
 * @param value Value to set
 * @param changed_by Identity of who made the change
 * @return 0 on success, negative on error
 */
int charm_config_set_int(const char* key, int64_t value, const char* changed_by);

/**
 * Set configuration value from boolean
 * @param key Configuration key
 * @param value Value to set
 * @param changed_by Identity of who made the change
 * @return 0 on success, negative on error
 */
int charm_config_set_bool(const char* key, bool value, const char* changed_by);

/**
 * Set configuration value from float
 * @param key Configuration key
 * @param value Value to set
 * @param changed_by Identity of who made the change
 * @return 0 on success, negative on error
 */
int charm_config_set_float(const char* key, double value, const char* changed_by);

/**
 * Validate all configuration values
 * @param result Output validation result
 * @return 0 on success, negative on error
 */
int charm_config_validate(charm_config_validation_result_t* result);

/**
 * Validate specific configuration value
 * @param key Configuration key
 * @param result Output validation result
 * @return 0 on success, negative on error
 */
int charm_config_validate_key(const char* key, charm_config_validation_result_t* result);

/**
 * Check if configuration has been tampered with
 * @param tampered Output flag indicating tampering
 * @return 0 on success, negative on error
 */
int charm_config_check_integrity(bool* tampered);

/**
 * Get list of all configuration keys
 * @param keys Output array of key names
 * @param max_keys Maximum number of keys to return
 * @param count Actual number of keys returned
 * @return 0 on success, negative on error
 */
int charm_config_list_keys(char keys[][CHARM_CONFIG_MAX_KEY], size_t max_keys, size_t* count);

/**
 * Get configuration metadata
 * @param key Configuration key
 * @param metadata Output metadata structure
 * @return 0 on success, negative on error
 */
int charm_config_get_metadata(const char* key, charm_config_value_t* metadata);

/**
 * Register for configuration change notifications
 * @param callback Callback function to call on changes
 * @return 0 on success, negative on error
 */
int charm_config_register_change_callback(void (*callback)(const charm_config_change_t* change));

/**
 * Enable/disable runtime configuration modification
 * @param enabled 1 to enable, 0 to disable
 * @return 0 on success, negative on error
 */
int charm_config_set_runtime_modification(bool enabled);

/**
 * Create configuration backup
 * @param backup_file Path to backup file
 * @return 0 on success, negative on error
 */
int charm_config_backup(const char* backup_file);

/**
 * Restore configuration from backup
 * @param backup_file Path to backup file
 * @return 0 on success, negative on error
 */
int charm_config_restore(const char* backup_file);

// Secure configuration defaults for CHARM
extern const charm_config_schema_t charm_default_config_schema[];

// Convenience macros for common configuration access
#define CHARM_CONFIG_GET_STRING(key, buf) \
    charm_config_get_string(key, buf, sizeof(buf))

#define CHARM_CONFIG_GET_INT(key, var) \
    charm_config_get_int(key, &(var))

#define CHARM_CONFIG_GET_BOOL(key, var) \
    charm_config_get_bool(key, &(var))

#define CHARM_CONFIG_GET_FLOAT(key, var) \
    charm_config_get_float(key, &(var))

#ifdef __cplusplus
}
#endif

#endif // CHARM_CONFIG_H