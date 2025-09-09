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
 * @file charmctl.h
 * @brief Central controller for CHARM system
 * 
 * This header defines the interface for the central controller of the CHARM system,
 * managing system states, daemon lifecycle, and coordinating subsystems.
 */

#ifndef CHARMCTL_H
#define CHARMCTL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM system states
 */
typedef enum {
    CHARM_STATE_UNINITIALIZED = 0, /**< System not initialized */
    CHARM_STATE_INIT = 1,          /**< System initializing */
    CHARM_STATE_RUNNING = 2,       /**< System running normally */
    CHARM_STATE_DEGRADED = 3,      /**< System running with degraded entropy */
    CHARM_STATE_FAILOVER = 4,      /**< System in failover mode */
    CHARM_STATE_RECOVERY = 5,      /**< System recovering */
    CHARM_STATE_SHUTDOWN = 6       /**< System shutting down */
} charm_state_t;

/**
 * @brief CHARM subsystem IDs
 */
typedef enum {
    CHARM_SUBSYS_CAEDS = 0,        /**< CAEDS subsystem */
    CHARM_SUBSYS_CEE = 1,          /**< CEE subsystem */
    CHARM_SUBSYS_ECE = 2,          /**< ECE subsystem */
    CHARM_SUBSYS_BUS = 3,          /**< Entropy bus */
    CHARM_SUBSYS_WATCHDOG = 4,     /**< Watchdog daemon */
    CHARM_SUBSYS_FALLBACK = 5,     /**< Fallback entropy */
    CHARM_SUBSYS_LOGGER = 6,       /**< Snapshot logger */
    CHARM_SUBSYS_COUNT             /**< Number of subsystems */
} charm_subsystem_t;

/**
 * @brief CHARM subsystem states
 */
typedef enum {
    CHARM_SUBSYS_STATE_INACTIVE = 0,   /**< Subsystem inactive */
    CHARM_SUBSYS_STATE_STARTING = 1,   /**< Subsystem starting */
    CHARM_SUBSYS_STATE_ACTIVE = 2,     /**< Subsystem active */
    CHARM_SUBSYS_STATE_DEGRADED = 3,   /**< Subsystem degraded */
    CHARM_SUBSYS_STATE_FAILED = 4,     /**< Subsystem failed */
    CHARM_SUBSYS_STATE_STOPPING = 5    /**< Subsystem stopping */
} charm_subsystem_state_t;

/**
 * @brief CHARM controller status codes
 */
typedef enum {
    CHARM_STATUS_OK = 0,               /**< Operation successful */
    CHARM_STATUS_ERROR = -1,           /**< General error */
    CHARM_STATUS_INVALID_ARG = -2,     /**< Invalid argument */
    CHARM_STATUS_NOT_INITIALIZED = -3, /**< Not initialized */
    CHARM_STATUS_ALREADY_INIT = -4,    /**< Already initialized */
    CHARM_STATUS_SUBSYS_ERROR = -5,    /**< Subsystem error */
    CHARM_STATUS_INVALID_STATE = -6    /**< Invalid state transition */
} charm_status_t;

/**
 * @brief CHARM system configuration
 */
typedef struct {
    bool auto_start;                   /**< Auto-start subsystems */
    bool auto_recovery;                /**< Auto-recovery on failure */
    int min_entropy_sources;           /**< Minimum entropy sources required */
    double min_entropy_quality;        /**< Minimum entropy quality required */
    char* log_dir;                     /**< Log directory */
    int log_level;                     /**< Log level (0-5) */
} charm_config_t;

/**
 * @brief CHARM system statistics
 */
typedef struct {
    charm_state_t system_state;                        /**< Current system state */
    charm_subsystem_state_t subsys_states[CHARM_SUBSYS_COUNT]; /**< Subsystem states */
    uint64_t uptime_seconds;                           /**< System uptime in seconds */
    uint64_t entropy_bytes_processed;                  /**< Total entropy bytes processed */
    uint64_t hash_operations;                          /**< Total hash operations */
    double current_entropy_quality;                    /**< Current entropy quality */
    uint32_t active_entropy_sources;                   /**< Number of active entropy sources */
    uint32_t anomalies_detected;                       /**< Number of anomalies detected */
    uint32_t recovery_attempts;                        /**< Number of recovery attempts */
} charm_stats_t;

/**
 * @brief Initialize the CHARM controller
 * 
 * @param config Configuration structure
 * @return charm_status_t Status code
 */
charm_status_t charmctl_init(const charm_config_t* config);

/**
 * @brief Shutdown the CHARM controller
 * 
 * @return charm_status_t Status code
 */
charm_status_t charmctl_shutdown(void);

/**
 * @brief Start the CHARM system
 * 
 * @return charm_status_t Status code
 */
charm_status_t charmctl_start(void);

/**
 * @brief Stop the CHARM system
 * 
 * @return charm_status_t Status code
 */
charm_status_t charmctl_stop(void);

/**
 * @brief Get the current system state
 * 
 * @param state Pointer to store state
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_state(charm_state_t* state);

/**
 * @brief Get the current state of a subsystem
 * 
 * @param subsys Subsystem ID
 * @param state Pointer to store state
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_subsys_state(charm_subsystem_t subsys, charm_subsystem_state_t* state);

/**
 * @brief Start a specific subsystem
 * 
 * @param subsys Subsystem ID
 * @return charm_status_t Status code
 */
charm_status_t charmctl_start_subsystem(charm_subsystem_t subsys);

/**
 * @brief Stop a specific subsystem
 * 
 * @param subsys Subsystem ID
 * @return charm_status_t Status code
 */
charm_status_t charmctl_stop_subsystem(charm_subsystem_t subsys);

/**
 * @brief Restart a specific subsystem
 * 
 * @param subsys Subsystem ID
 * @return charm_status_t Status code
 */
charm_status_t charmctl_restart_subsystem(charm_subsystem_t subsys);

/**
 * @brief Get system statistics
 * 
 * @param stats Pointer to statistics structure
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_stats(charm_stats_t* stats);

/**
 * @brief Trigger system recovery
 * 
 * @return charm_status_t Status code
 */
charm_status_t charmctl_trigger_recovery(void);

/**
 * @brief Check if system is healthy
 * 
 * @param healthy Pointer to store result
 * @return charm_status_t Status code
 */
charm_status_t charmctl_is_healthy(bool* healthy);

/**
 * @brief Set minimum entropy quality
 * 
 * @param quality Minimum entropy quality (0.0-1.0)
 * @return charm_status_t Status code
 */
charm_status_t charmctl_set_min_entropy_quality(double quality);

/**
 * @brief Get subsystem name
 * 
 * @param subsys Subsystem ID
 * @return const char* Subsystem name
 */
const char* charmctl_get_subsys_name(charm_subsystem_t subsys);

/**
 * @brief Get state name
 * 
 * @param state System state
 * @return const char* State name
 */
const char* charmctl_get_state_name(charm_state_t state);

/**
 * @brief Get subsystem state name
 * 
 * @param state Subsystem state
 * @return const char* Subsystem state name
 */
const char* charmctl_get_subsys_state_name(charm_subsystem_state_t state);

#ifdef __cplusplus
}
#endif

#endif /* CHARMCTL_H */
