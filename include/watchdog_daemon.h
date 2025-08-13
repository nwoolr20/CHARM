/**
 * @file watchdog_daemon.h
 * @brief Watchdog daemon for monitoring and maintaining CHARM system health
 * 
 * This header defines the watchdog daemon for the CHARM system, providing
 * mechanisms for monitoring entropy sources, detecting anomalies, managing
 * system lifecycle, and maintaining the Entropy Health Score (EHS).
 */

#ifndef WATCHDOG_DAEMON_H
#define WATCHDOG_DAEMON_H

#include "caeds_flux.h"
#include "caeds_anomaly.h"
#include "caeds_predict.h"
#include "entropy_bus.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Watchdog daemon status codes
 */
typedef enum {
    WATCHDOG_STATUS_OK = 0,           /**< Operation successful */
    WATCHDOG_STATUS_ERROR = -1,       /**< General error */
    WATCHDOG_STATUS_INVALID_ARG = -2, /**< Invalid argument */
    WATCHDOG_STATUS_NOT_RUNNING = -3, /**< Daemon not running */
    WATCHDOG_STATUS_ALREADY_RUNNING = -4 /**< Daemon already running */
} watchdog_status_t;

/**
 * @brief Watchdog daemon lifecycle states
 */
typedef enum {
    WATCHDOG_STATE_STOPPED = 0,       /**< Daemon stopped */
    WATCHDOG_STATE_STARTING = 1,      /**< Daemon starting */
    WATCHDOG_STATE_RUNNING = 2,       /**< Daemon running */
    WATCHDOG_STATE_PAUSED = 3,        /**< Daemon paused */
    WATCHDOG_STATE_STOPPING = 4       /**< Daemon stopping */
} watchdog_state_t;

/**
 * @brief Watchdog daemon alert levels
 */
typedef enum {
    WATCHDOG_ALERT_NONE = 0,          /**< No alert */
    WATCHDOG_ALERT_INFO = 1,          /**< Informational alert */
    WATCHDOG_ALERT_WARNING = 2,       /**< Warning alert */
    WATCHDOG_ALERT_CRITICAL = 3       /**< Critical alert */
} watchdog_alert_level_t;

/**
 * @brief Watchdog daemon alert types
 */
typedef enum {
    WATCHDOG_ALERT_TYPE_ENTROPY_LOW = 0,      /**< Low entropy quality */
    WATCHDOG_ALERT_TYPE_ENTROPY_DEPLETED = 1, /**< Entropy depleted */
    WATCHDOG_ALERT_TYPE_ANOMALY_DETECTED = 2, /**< Anomaly detected */
    WATCHDOG_ALERT_TYPE_SOURCE_FAILURE = 3,   /**< Entropy source failure */
    WATCHDOG_ALERT_TYPE_PREDICTION_FAILURE = 4, /**< Prediction failure */
    WATCHDOG_ALERT_TYPE_SYSTEM_ERROR = 5      /**< System error */
} watchdog_alert_type_t;

/**
 * @brief Watchdog daemon alert structure
 */
typedef struct {
    watchdog_alert_level_t level;     /**< Alert level */
    watchdog_alert_type_t type;       /**< Alert type */
    uint64_t timestamp;               /**< Alert timestamp */
    caeds_flux_source_t source;       /**< Entropy source (if applicable) */
    char message[256];                /**< Alert message */
} watchdog_alert_t;

/**
 * @brief Watchdog daemon configuration structure
 */
typedef struct {
    uint32_t check_interval_ms;       /**< Check interval in milliseconds */
    uint32_t min_entropy_quality;     /**< Minimum entropy quality (0-100) */
    bool auto_recovery;               /**< Enable automatic recovery */
    bool log_alerts;                  /**< Log alerts to file */
    char log_file[256];               /**< Log file path */
    uint32_t max_alerts;              /**< Maximum number of alerts to store */
} watchdog_config_t;

/**
 * @brief Entropy Health Score (EHS) structure
 */
typedef struct {
    uint32_t overall_score;           /**< Overall health score (0-100) */
    uint32_t source_scores[CAEDS_FLUX_SOURCE_COUNT]; /**< Per-source scores */
    double entropy_quality;           /**< Current entropy quality (0.0-1.0) */
    double source_diversity;          /**< Source diversity score (0.0-1.0) */
    double prediction_accuracy;       /**< Prediction accuracy (0.0-1.0) */
    uint64_t timestamp;               /**< Timestamp of last update */
} watchdog_ehs_t;

/**
 * @brief Watchdog daemon statistics structure
 */
typedef struct {
    uint64_t uptime_ms;               /**< Daemon uptime in milliseconds */
    uint32_t check_count;             /**< Number of checks performed */
    uint32_t alert_count;             /**< Number of alerts generated */
    uint32_t recovery_count;          /**< Number of recovery actions */
    watchdog_state_t current_state;   /**< Current daemon state */
    watchdog_ehs_t current_ehs;       /**< Current Entropy Health Score */
} watchdog_stats_t;

/**
 * @brief Watchdog daemon alert callback function type
 * 
 * @param alert Alert structure
 * @param user_data User-defined data
 */
typedef void (*watchdog_alert_callback_t)(const watchdog_alert_t* alert, void* user_data);

/**
 * @brief Default watchdog daemon configuration
 */
extern const watchdog_config_t WATCHDOG_DEFAULT_CONFIG;

/**
 * @brief Initialize the watchdog daemon
 * 
 * @param config Configuration structure (NULL for defaults)
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_init(const watchdog_config_t* config);

/**
 * @brief Start the watchdog daemon
 * 
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_start(void);

/**
 * @brief Stop the watchdog daemon
 * 
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_stop(void);

/**
 * @brief Pause the watchdog daemon
 * 
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_pause(void);

/**
 * @brief Resume the watchdog daemon
 * 
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_resume(void);

/**
 * @brief Get the current watchdog daemon state
 * 
 * @param state Pointer to store state
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_get_state(watchdog_state_t* state);

/**
 * @brief Get the current Entropy Health Score (EHS)
 * 
 * @param ehs Pointer to store EHS
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_get_ehs(watchdog_ehs_t* ehs);

/**
 * @brief Get watchdog daemon statistics
 * 
 * @param stats Pointer to store statistics
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_get_stats(watchdog_stats_t* stats);

/**
 * @brief Register an alert callback
 * 
 * @param callback Callback function
 * @param user_data User-defined data
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_register_alert_callback(watchdog_alert_callback_t callback, void* user_data);

/**
 * @brief Unregister an alert callback
 * 
 * @param callback Callback function
 * @param user_data User-defined data
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_unregister_alert_callback(watchdog_alert_callback_t callback, void* user_data);

/**
 * @brief Get recent alerts
 * 
 * @param alerts Array to store alerts
 * @param max_alerts Maximum number of alerts to retrieve
 * @param num_alerts Pointer to store number of alerts retrieved
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_get_alerts(watchdog_alert_t* alerts, uint32_t max_alerts, uint32_t* num_alerts);

/**
 * @brief Clear all alerts
 * 
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_clear_alerts(void);

/**
 * @brief Force an immediate check
 * 
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_force_check(void);

/**
 * @brief Set the check interval
 * 
 * @param interval_ms Check interval in milliseconds
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_set_check_interval(uint32_t interval_ms);

/**
 * @brief Set the minimum entropy quality
 * 
 * @param min_quality Minimum entropy quality (0-100)
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_set_min_entropy_quality(uint32_t min_quality);

/**
 * @brief Enable or disable automatic recovery
 * 
 * @param enable Enable flag
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_set_auto_recovery(bool enable);

/**
 * @brief Enable or disable alert logging
 * 
 * @param enable Enable flag
 * @param log_file Log file path (NULL for default)
 * @return watchdog_status_t Status code
 */
watchdog_status_t watchdog_set_alert_logging(bool enable, const char* log_file);

/**
 * @brief Get string representation of watchdog state
 * 
 * @param state Watchdog state
 * @return const char* String representation
 */
const char* watchdog_state_to_string(watchdog_state_t state);

/**
 * @brief Get string representation of alert level
 * 
 * @param level Alert level
 * @return const char* String representation
 */
const char* watchdog_alert_level_to_string(watchdog_alert_level_t level);

/**
 * @brief Get string representation of alert type
 * 
 * @param type Alert type
 * @return const char* String representation
 */
const char* watchdog_alert_type_to_string(watchdog_alert_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* WATCHDOG_DAEMON_H */
