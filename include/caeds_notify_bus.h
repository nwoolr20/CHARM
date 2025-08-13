/**
 * @file caeds_notify_bus.h
 * @brief Integration between CAEDS notification system and entropy bus
 * 
 * This header defines the integration points between the CAEDS notification
 * system and the entropy bus, enabling system-wide propagation of entropy
 * anomalies and status updates.
 */

#ifndef CAEDS_NOTIFY_BUS_H
#define CAEDS_NOTIFY_BUS_H

#include "caeds_notify.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Notification codes for entropy-related events
 */
typedef enum {
    CAEDS_NOTIFY_CODE_ENTROPY_LOW = 1001,           /**< Entropy level below threshold */
    CAEDS_NOTIFY_CODE_ENTROPY_CRITICAL = 1002,      /**< Entropy level critically low */
    CAEDS_NOTIFY_CODE_ENTROPY_RECOVERED = 1003,     /**< Entropy level recovered */
    CAEDS_NOTIFY_CODE_ENTROPY_ANOMALY = 1004,       /**< Entropy anomaly detected */
    CAEDS_NOTIFY_CODE_SOURCE_DEGRADED = 1005,       /**< Entropy source degraded */
    CAEDS_NOTIFY_CODE_SOURCE_FAILED = 1006,         /**< Entropy source failed */
    CAEDS_NOTIFY_CODE_SOURCE_RECOVERED = 1007,      /**< Entropy source recovered */
    CAEDS_NOTIFY_CODE_PREDICTION_WARNING = 1008,    /**< Entropy degradation predicted */
    CAEDS_NOTIFY_CODE_SYSTEM_DEGRADED = 1009,       /**< System in degraded mode */
    CAEDS_NOTIFY_CODE_SYSTEM_FAILOVER = 1010,       /**< System in failover mode */
    CAEDS_NOTIFY_CODE_SYSTEM_RECOVERED = 1011       /**< System recovered */
} caeds_notify_code_t;

/**
 * @brief Initialize the CAEDS notification bus integration
 * 
 * @param notify_context CAEDS notification context
 * @return int 0 on success, non-zero on failure
 */
int caeds_notify_bus_init(caeds_notify_context_t* notify_context);

/**
 * @brief Register the CAEDS notification system as an entropy bus consumer
 * 
 * @return int Consumer ID on success, negative on failure
 */
int caeds_notify_bus_register(void);

/**
 * @brief Send an entropy-related notification
 * 
 * @param severity Notification severity
 * @param code Notification code
 * @param format Message format string (printf-style)
 * @param ... Format arguments
 * @return int 0 on success, non-zero on failure
 */
int caeds_notify_bus_send(caeds_notify_severity_t severity, 
                         caeds_notify_code_t code,
                         const char* format, ...);

/**
 * @brief Update entropy quality and send notifications if needed
 * 
 * @param source_id Source ID
 * @param quality New quality estimate (0.0-1.0)
 * @return int 0 on success, non-zero on failure
 */
int caeds_notify_bus_update_quality(int source_id, double quality);

/**
 * @brief Process entropy statistics and send notifications if needed
 * 
 * @param entropy_level Current entropy level (0.0-1.0)
 * @param source_count Number of active sources
 * @return int 0 on success, non-zero on failure
 */
int caeds_notify_bus_process_stats(double entropy_level, int source_count);

/**
 * @brief Shutdown the CAEDS notification bus integration
 * 
 * @return int 0 on success, non-zero on failure
 */
int caeds_notify_bus_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_NOTIFY_BUS_H */
