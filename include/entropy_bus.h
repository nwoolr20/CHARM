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
 * @file entropy_bus.h
 * @brief Inter-module entropy signaling and routing for the CHARM system
 * 
 * This header defines the entropy bus for the CHARM system, providing
 * mechanisms for inter-module communication, entropy signaling, and
 * routing of entropy-related messages.
 */

#ifndef ENTROPY_BUS_H
#define ENTROPY_BUS_H

#include "charm_status.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Entropy bus status codes
 */
typedef enum {
    ENTROPY_BUS_STATUS_OK = 0,           /**< Operation successful */
    ENTROPY_BUS_STATUS_ERROR = -1,       /**< General error */
    ENTROPY_BUS_STATUS_INVALID_ARG = -2, /**< Invalid argument */
    ENTROPY_BUS_STATUS_TIMEOUT = -3,     /**< Operation timed out */
    ENTROPY_BUS_STATUS_FULL = -4,        /**< Bus queue is full */
    ENTROPY_BUS_STATUS_EMPTY = -5        /**< Bus queue is empty */
} entropy_bus_status_t;

/**
 * @brief Entropy bus message types
 */
typedef enum {
    ENTROPY_BUS_MSG_ENTROPY_AVAILABLE = 0,       /**< Entropy is available */
    ENTROPY_BUS_MSG_ENTROPY_REQUEST = 1,         /**< Request for entropy */
    ENTROPY_BUS_MSG_ENTROPY_DATA = 2,            /**< Entropy data */
    ENTROPY_BUS_MSG_ENTROPY_QUALITY = 3,         /**< Entropy quality update */
    ENTROPY_BUS_MSG_SOURCE_STATUS = 4,           /**< Entropy source status */
    ENTROPY_BUS_MSG_ANOMALY_DETECTED = 5,        /**< Anomaly detected */
    ENTROPY_BUS_MSG_PREDICTION_UPDATE = 6,       /**< Prediction update */
    ENTROPY_BUS_MSG_REQUEST_SYNTHETIC_ENTROPY = 7, /**< Request for synthetic entropy */
    ENTROPY_BUS_MSG_LIFECYCLE_EVENT = 8,         /**< Lifecycle event */
    ENTROPY_BUS_MSG_CONTROL_COMMAND = 9,         /**< Control command */
    ENTROPY_BUS_MSG_VOLATILITY_HEADER = 10,      /**< Volatility header */
    ENTROPY_BUS_MSG_CUSTOM = 11                  /**< Custom message */
} entropy_bus_msg_type_t;

/**
 * @brief Entropy bus message sources
 */
typedef enum {
    ENTROPY_BUS_SOURCE_CAEDS = 0,        /**< CAEDS subsystem */
    ENTROPY_BUS_SOURCE_CEE = 1,          /**< CEE subsystem */
    ENTROPY_BUS_SOURCE_ECE = 2,          /**< ECE subsystem */
    ENTROPY_BUS_SOURCE_WATCHDOG = 3,     /**< Watchdog daemon */
    ENTROPY_BUS_SOURCE_CLI = 4,          /**< CLI tools */
    ENTROPY_BUS_SOURCE_DRIVER = 5,       /**< Hardware driver */
    ENTROPY_BUS_SOURCE_FALLBACK = 6,     /**< Fallback entropy */
    ENTROPY_BUS_SOURCE_CUSTOM = 7        /**< Custom source */
} entropy_bus_source_t;

/**
 * @brief Entropy bus message destinations
 */
typedef enum {
    ENTROPY_BUS_DESTINATION_CAEDS = 0,   /**< CAEDS subsystem */
    ENTROPY_BUS_DESTINATION_CEE = 1,     /**< CEE subsystem */
    ENTROPY_BUS_DESTINATION_ECE = 2,     /**< ECE subsystem */
    ENTROPY_BUS_DESTINATION_WATCHDOG = 3, /**< Watchdog daemon */
    ENTROPY_BUS_DESTINATION_CLI = 4,     /**< CLI tools */
    ENTROPY_BUS_DESTINATION_DRIVER = 5,  /**< Hardware driver */
    ENTROPY_BUS_DESTINATION_FALLBACK = 6, /**< Fallback entropy */
    ENTROPY_BUS_DESTINATION_CUSTOM = 7,  /**< Custom destination */
    ENTROPY_BUS_DESTINATION_BROADCAST = 255 /**< Broadcast to all */
} entropy_bus_destination_t;

/**
 * @brief Entropy bus message priorities
 */
typedef enum {
    ENTROPY_BUS_PRIORITY_LOW = 0,        /**< Low priority */
    ENTROPY_BUS_PRIORITY_NORMAL = 1,     /**< Normal priority */
    ENTROPY_BUS_PRIORITY_HIGH = 2,       /**< High priority */
    ENTROPY_BUS_PRIORITY_CRITICAL = 3    /**< Critical priority */
} entropy_bus_priority_t;

/**
 * @brief Entropy bus lifecycle events
 */
typedef enum {
    ENTROPY_BUS_LIFECYCLE_INIT = 0,      /**< Initialization */
    ENTROPY_BUS_LIFECYCLE_START = 1,     /**< Start */
    ENTROPY_BUS_LIFECYCLE_STOP = 2,      /**< Stop */
    ENTROPY_BUS_LIFECYCLE_PAUSE = 3,     /**< Pause */
    ENTROPY_BUS_LIFECYCLE_RESUME = 4,    /**< Resume */
    ENTROPY_BUS_LIFECYCLE_SHUTDOWN = 5   /**< Shutdown */
} entropy_bus_lifecycle_t;

/**
 * @brief Entropy bus control commands
 */
typedef enum {
    ENTROPY_BUS_CONTROL_RESET = 0,       /**< Reset */
    ENTROPY_BUS_CONTROL_FLUSH = 1,       /**< Flush */
    ENTROPY_BUS_CONTROL_ACTIVATE = 2,    /**< Activate */
    ENTROPY_BUS_CONTROL_DEACTIVATE = 3,  /**< Deactivate */
    ENTROPY_BUS_CONTROL_SET_PARAM = 4,   /**< Set parameter */
    ENTROPY_BUS_CONTROL_GET_PARAM = 5    /**< Get parameter */
} entropy_bus_control_t;

/**
 * @brief Maximum message data size
 */
#define ENTROPY_BUS_MAX_DATA_SIZE 1024

/**
 * @brief Entropy bus message structure
 */
typedef struct {
    entropy_bus_msg_type_t type;         /**< Message type */
    entropy_bus_source_t source;         /**< Message source */
    entropy_bus_destination_t destination; /**< Message destination */
    entropy_bus_priority_t priority;     /**< Message priority */
    uint64_t timestamp;                  /**< Message timestamp */
    uint32_t data_size;                  /**< Data size */
    uint8_t data[ENTROPY_BUS_MAX_DATA_SIZE]; /**< Message data */
} entropy_bus_message_t;

/**
 * @brief Entropy bus volatility header structure
 */
typedef struct {
    uint8_t entropy_quality;             /**< Entropy quality (0-255) */
    uint8_t source_diversity;            /**< Source diversity (0-255) */
    uint8_t temporal_variance;           /**< Temporal variance (0-255) */
    uint8_t prediction_confidence;       /**< Prediction confidence (0-255) */
    uint32_t source_mask;                /**< Source mask */
    uint64_t timestamp;                  /**< Timestamp */
} entropy_bus_volatility_t;

/**
 * @brief Entropy bus handle
 */
typedef struct entropy_bus_context* entropy_bus_handle_t;

/**
 * @brief Entropy bus configuration structure
 */
typedef struct {
    uint32_t queue_size;                 /**< Message queue size */
    uint32_t timeout_ms;                 /**< Operation timeout in milliseconds */
    bool use_shared_memory;              /**< Use shared memory for IPC */
    bool use_sockets;                    /**< Use sockets for IPC */
    char socket_path[256];               /**< Socket path for IPC */
} entropy_bus_config_t;

/**
 * @brief Entropy bus statistics structure
 */
typedef struct {
    uint64_t messages_sent;              /**< Messages sent */
    uint64_t messages_received;          /**< Messages received */
    uint64_t bytes_sent;                 /**< Bytes sent */
    uint64_t bytes_received;             /**< Bytes received */
    uint32_t queue_high_water_mark;      /**< Queue high water mark */
    uint32_t current_queue_size;         /**< Current queue size */
    uint64_t dropped_messages;           /**< Dropped messages */
} entropy_bus_stats_t;

/**
 * @brief Entropy bus message callback function type
 * 
 * @param message Message structure
 * @param user_data User-defined data
 */
typedef void (*entropy_bus_callback_t)(const entropy_bus_message_t* message, void* user_data);

/**
 * @brief Default entropy bus configuration
 */
extern const entropy_bus_config_t ENTROPY_BUS_DEFAULT_CONFIG;

/**
 * @brief Initialize the entropy bus
 * 
 * @param config Configuration structure (NULL for defaults)
 * @return entropy_bus_handle_t Bus handle or NULL on failure
 */
entropy_bus_handle_t entropy_bus_init(const entropy_bus_config_t* config);

/**
 * @brief Shutdown the entropy bus
 * 
 * @param handle Bus handle
 */
void entropy_bus_shutdown(entropy_bus_handle_t handle);

/**
 * @brief Send a message on the entropy bus
 * 
 * @param handle Bus handle
 * @param message Message structure
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_send_message(entropy_bus_handle_t handle, const entropy_bus_message_t* message);

/**
 * @brief Receive a message from the entropy bus
 * 
 * @param handle Bus handle
 * @param message Message structure to fill
 * @param timeout_ms Timeout in milliseconds (0 for non-blocking, -1 for infinite)
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_receive_message(entropy_bus_handle_t handle, entropy_bus_message_t* message, int32_t timeout_ms);

/**
 * @brief Register a message callback
 * 
 * @param handle Bus handle
 * @param type Message type (or -1 for all types)
 * @param source Message source (or -1 for all sources)
 * @param callback Callback function
 * @param user_data User-defined data
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_register_callback(entropy_bus_handle_t handle, int32_t type, int32_t source, entropy_bus_callback_t callback, void* user_data);

/**
 * @brief Unregister a message callback
 * 
 * @param handle Bus handle
 * @param callback Callback function
 * @param user_data User-defined data
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_unregister_callback(entropy_bus_handle_t handle, entropy_bus_callback_t callback, void* user_data);

/**
 * @brief Create a volatility header
 * 
 * @param handle Bus handle
 * @param volatility Volatility header structure to fill
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_create_volatility_header(entropy_bus_handle_t handle, entropy_bus_volatility_t* volatility);

/**
 * @brief Send a volatility header
 * 
 * @param handle Bus handle
 * @param volatility Volatility header structure
 * @param destination Message destination
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_send_volatility_header(entropy_bus_handle_t handle, const entropy_bus_volatility_t* volatility, entropy_bus_destination_t destination);

/**
 * @brief Send a lifecycle event
 * 
 * @param handle Bus handle
 * @param event Lifecycle event
 * @param source Message source
 * @param destination Message destination
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_send_lifecycle_event(entropy_bus_handle_t handle, entropy_bus_lifecycle_t event, entropy_bus_source_t source, entropy_bus_destination_t destination);

/**
 * @brief Send a control command
 * 
 * @param handle Bus handle
 * @param command Control command
 * @param source Message source
 * @param destination Message destination
 * @param data Command data
 * @param data_size Data size
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_send_control_command(entropy_bus_handle_t handle, entropy_bus_control_t command, entropy_bus_source_t source, entropy_bus_destination_t destination, const void* data, uint32_t data_size);

/**
 * @brief Get entropy bus statistics
 * 
 * @param handle Bus handle
 * @param stats Statistics structure to fill
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_get_stats(entropy_bus_handle_t handle, entropy_bus_stats_t* stats);

/**
 * @brief Flush the entropy bus
 * 
 * @param handle Bus handle
 * @return entropy_bus_status_t Status code
 */
entropy_bus_status_t entropy_bus_flush(entropy_bus_handle_t handle);

/**
 * @brief Get string representation of message type
 * 
 * @param type Message type
 * @return const char* String representation
 */
const char* entropy_bus_msg_type_to_string(entropy_bus_msg_type_t type);

/**
 * @brief Get string representation of message source
 * 
 * @param source Message source
 * @return const char* String representation
 */
const char* entropy_bus_source_to_string(entropy_bus_source_t source);

/**
 * @brief Get string representation of message destination
 * 
 * @param destination Message destination
 * @return const char* String representation
 */
const char* entropy_bus_destination_to_string(entropy_bus_destination_t destination);

/**
 * @brief Get string representation of message priority
 * 
 * @param priority Message priority
 * @return const char* String representation
 */
const char* entropy_bus_priority_to_string(entropy_bus_priority_t priority);

/**
 * @brief Get string representation of lifecycle event
 * 
 * @param event Lifecycle event
 * @return const char* String representation
 */
const char* entropy_bus_lifecycle_to_string(entropy_bus_lifecycle_t event);

/**
 * @brief Get string representation of control command
 * 
 * @param command Control command
 * @return const char* String representation
 */
const char* entropy_bus_control_to_string(entropy_bus_control_t command);

#ifdef __cplusplus
}
#endif

#endif /* ENTROPY_BUS_H */
