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

#ifndef CAEDS_NOTIFY_H
#define CAEDS_NOTIFY_H

/**
 * @file caeds_notify.h
 * @brief Notification system for the CAEDS subsystem
 * 
 * This header defines the notification system for the CAEDS (Charm Advanced 
 * Entropy Diagnostic System) subsystem, providing mechanisms to broadcast
 * entropy anomalies and status updates to various output channels.
 */

#include "charm_status.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Notification severity levels
 */
typedef enum {
    CAEDS_NOTIFY_INFO = 0,       /**< Informational message */
    CAEDS_NOTIFY_WARNING = 1,    /**< Warning message */
    CAEDS_NOTIFY_ERROR = 2,      /**< Error message */
    CAEDS_NOTIFY_CRITICAL = 3,   /**< Critical message */
    CAEDS_NOTIFY_ALERT = 4       /**< Alert message requiring immediate attention */
} caeds_notify_severity_t;

/**
 * @brief Notification output channels
 */
typedef enum {
    CAEDS_NOTIFY_CHANNEL_NONE = 0,           /**< No output */
    CAEDS_NOTIFY_CHANNEL_CLI = (1 << 0),     /**< Command-line interface */
    CAEDS_NOTIFY_CHANNEL_LOG = (1 << 1),     /**< Log file */
    CAEDS_NOTIFY_CHANNEL_SYSLOG = (1 << 2),  /**< System log */
    CAEDS_NOTIFY_CHANNEL_CALLBACK = (1 << 3),/**< User-defined callback */
    CAEDS_NOTIFY_CHANNEL_ALL = 0xFF          /**< All channels */
} caeds_notify_channel_t;

/**
 * @brief Notification message structure
 */
typedef struct {
    caeds_notify_severity_t severity;    /**< Message severity */
    uint64_t timestamp;                  /**< Message timestamp (microseconds since epoch) */
    const char* source;                  /**< Source module */
    const char* message;                 /**< Message text */
    uint32_t code;                       /**< Message code */
    void* context;                       /**< Additional context (optional) */
} caeds_notify_message_t;

/**
 * @brief Notification callback function type
 * 
 * @param message Notification message
 * @param user_data User-defined data
 */
typedef void (*caeds_notify_callback_t)(const caeds_notify_message_t* message, void* user_data);

/**
 * @brief Notification context structure
 */
typedef struct {
    caeds_notify_channel_t channels;     /**< Enabled output channels */
    caeds_notify_severity_t min_severity; /**< Minimum severity level to output */
    const char* log_file;                /**< Log file path (if CAEDS_NOTIFY_CHANNEL_LOG enabled) */
    caeds_notify_callback_t callback;    /**< User callback (if CAEDS_NOTIFY_CHANNEL_CALLBACK enabled) */
    void* callback_data;                 /**< User data for callback */
    uint32_t flags;                      /**< Configuration flags */
} caeds_notify_context_t;

/**
 * @brief Initialize the notification system
 * 
 * @param context Pointer to store the notification context
 * @param config Configuration parameters
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_init(caeds_notify_context_t** context, const caeds_notify_context_t* config);

/**
 * @brief Send a notification message
 * 
 * @param context Notification context
 * @param severity Message severity
 * @param source Source module
 * @param code Message code
 * @param format Message format string (printf-style)
 * @param ... Format arguments
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_send(caeds_notify_context_t* context, 
                                caeds_notify_severity_t severity,
                                const char* source,
                                uint32_t code,
                                const char* format, ...);

/**
 * @brief Send a notification message with context
 * 
 * @param context Notification context
 * @param severity Message severity
 * @param source Source module
 * @param code Message code
 * @param user_context Additional context (optional)
 * @param format Message format string (printf-style)
 * @param ... Format arguments
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_send_with_context(caeds_notify_context_t* context, 
                                            caeds_notify_severity_t severity,
                                            const char* source,
                                            uint32_t code,
                                            void* user_context,
                                            const char* format, ...);

/**
 * @brief Register a callback for notifications
 * 
 * @param context Notification context
 * @param callback Callback function
 * @param user_data User-defined data
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_register_callback(caeds_notify_context_t* context,
                                            caeds_notify_callback_t callback,
                                            void* user_data);

/**
 * @brief Set minimum severity level for notifications
 * 
 * @param context Notification context
 * @param min_severity Minimum severity level
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_set_min_severity(caeds_notify_context_t* context,
                                           caeds_notify_severity_t min_severity);

/**
 * @brief Set enabled output channels
 * 
 * @param context Notification context
 * @param channels Enabled output channels
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_set_channels(caeds_notify_context_t* context,
                                       caeds_notify_channel_t channels);

/**
 * @brief Set log file path
 * 
 * @param context Notification context
 * @param log_file Log file path
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_set_log_file(caeds_notify_context_t* context,
                                       const char* log_file);

/**
 * @brief Shutdown the notification system
 * 
 * @param context Notification context
 * @return charm_status_t Status code
 */
charm_status_t caeds_notify_shutdown(caeds_notify_context_t* context);

/**
 * @brief Get string representation of severity level
 * 
 * @param severity Severity level
 * @return const char* String representation
 */
const char* caeds_notify_severity_to_string(caeds_notify_severity_t severity);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_NOTIFY_H */
