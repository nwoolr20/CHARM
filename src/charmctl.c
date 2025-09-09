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
 * @file charmctl.c
 * @brief Lifecycle coordinator for the CHARM v2.0 system
 * 
 * This module implements the main lifecycle coordinator for the CHARM system,
 * responsible for initialization, state management, and shutdown of all
 * subsystems. It orchestrates the boot sequence, entropy initialization,
 * watchdog startup, and entropy verification.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

// System states
typedef enum {
    CHARM_STATE_INIT,      // System initializing
    CHARM_STATE_RUN,       // Normal operation
    CHARM_STATE_DEGRADED,  // Degraded entropy quality
    CHARM_STATE_FAILOVER   // Using fallback entropy sources
} charm_system_state_t;

// Global state
static struct {
    charm_system_state_t state;
    bool initialized;
    time_t start_time;
    double entropy_quality;
} g_charm = {
    .state = CHARM_STATE_INIT,
    .initialized = false,
    .start_time = 0,
    .entropy_quality = 0.0
};

// Forward declarations of external functions
extern int caeds_flux_init(void);
extern int caeds_anomaly_init(void);
extern int caeds_predict_init(void);
extern int caeds_notify_init(void);

extern int cee_mix_init(size_t size);
extern int cee_whiten_init(size_t size, int algo);

extern int ece_seed_init(void);
extern int ece_stream_init(void);

extern int entropy_bus_init(void);
extern double entropy_bus_get_quality(void);

extern int fallback_entropy_init(void);
extern int fallback_entropy_activate(void);
extern int fallback_entropy_deactivate(void);

extern int watchdog_daemon_start(void);
extern int watchdog_daemon_stop(void);
extern void watchdog_daemon_register_callback(void (*callback)(charm_system_state_t old_state, 
                                                             charm_system_state_t new_state));

extern int entropy_trace_init(void);

/**
 * @brief State change callback from watchdog
 * 
 * @param old_state Previous state
 * @param new_state New state
 */
static void state_change_callback(charm_system_state_t old_state, charm_system_state_t new_state) {
    g_charm.state = new_state;
    
    // Handle state transitions
    switch (new_state) {
        case CHARM_STATE_RUN:
            // Normal operation, nothing special to do
            break;
            
        case CHARM_STATE_DEGRADED:
            // Degraded operation, might need to alert
            fprintf(stderr, "CHARM system in DEGRADED state\n");
            break;
            
        case CHARM_STATE_FAILOVER:
            // Failover operation, using fallback entropy
            fprintf(stderr, "CHARM system in FAILOVER state\n");
            break;
            
        case CHARM_STATE_INIT:
            // Should not transition back to INIT
            fprintf(stderr, "WARNING: Unexpected transition to INIT state\n");
            break;
    }
}

/**
 * @brief Initialize the CHARM system
 * 
 * @return int 0 on success, non-zero on failure
 */
int charmctl_init(void) {
    if (g_charm.initialized) {
        return 0; // Already initialized
    }
    
    g_charm.state = CHARM_STATE_INIT;
    g_charm.start_time = time(NULL);
    
    // Initialize subsystems in the correct order
    
    // 1. Initialize entropy bus (central communication)
    if (entropy_bus_init() != 0) {
        fprintf(stderr, "Failed to initialize entropy bus\n");
        return -1;
    }
    
    // 2. Initialize CAEDS subsystem
    if (caeds_flux_init() != 0 ||
        caeds_anomaly_init() != 0 ||
        caeds_predict_init() != 0 ||
        caeds_notify_init() != 0) {
        fprintf(stderr, "Failed to initialize CAEDS subsystem\n");
        return -1;
    }
    
    // 3. Initialize CEE subsystem
    if (cee_mix_init(1024) != 0 ||
        cee_whiten_init(1024, 0) != 0) {
        fprintf(stderr, "Failed to initialize CEE subsystem\n");
        return -1;
    }
    
    // 4. Initialize ECE subsystem
    if (ece_seed_init() != 0 ||
        ece_stream_init() != 0) {
        fprintf(stderr, "Failed to initialize ECE subsystem\n");
        return -1;
    }
    
    // 5. Initialize fallback entropy source
    if (fallback_entropy_init() != 0) {
        fprintf(stderr, "Failed to initialize fallback entropy source\n");
        // Non-fatal, continue
    }
    
    // 6. Initialize entropy trace
    if (entropy_trace_init() != 0) {
        fprintf(stderr, "Failed to initialize entropy trace\n");
        // Non-fatal, continue
    }
    
    // 7. Register state change callback
    watchdog_daemon_register_callback(state_change_callback);
    
    // 8. Start watchdog daemon
    if (watchdog_daemon_start() != 0) {
        fprintf(stderr, "Failed to start watchdog daemon\n");
        return -1;
    }
    
    // Wait for initial entropy quality assessment
    sleep(1);
    g_charm.entropy_quality = entropy_bus_get_quality();
    
    // Check if we need to activate fallback immediately
    if (g_charm.entropy_quality < 0.3) {
        fprintf(stderr, "Initial entropy quality too low (%.2f), activating fallback\n", 
                g_charm.entropy_quality);
        fallback_entropy_activate();
    }
    
    g_charm.initialized = true;
    return 0;
}

/**
 * @brief Shutdown the CHARM system
 * 
 * @return int 0 on success, non-zero on failure
 */
int charmctl_shutdown(void) {
    if (!g_charm.initialized) {
        return 0; // Already shut down
    }
    
    // Stop watchdog daemon
    watchdog_daemon_stop();
    
    // Deactivate fallback if active
    if (g_charm.state == CHARM_STATE_FAILOVER) {
        fallback_entropy_deactivate();
    }
    
    // Reset state
    g_charm.state = CHARM_STATE_INIT;
    g_charm.initialized = false;
    
    return 0;
}

/**
 * @brief Get the current system state
 * 
 * @return charm_system_state_t Current system state
 */
charm_system_state_t charmctl_get_state(void) {
    return g_charm.state;
}

/**
 * @brief Convert system state to string
 * 
 * @param state System state
 * @return const char* String representation of state
 */
const char* charmctl_state_to_string(charm_system_state_t state) {
    switch (state) {
        case CHARM_STATE_INIT:
            return "INIT";
        case CHARM_STATE_RUN:
            return "RUN";
        case CHARM_STATE_DEGRADED:
            return "DEGRADED";
        case CHARM_STATE_FAILOVER:
            return "FAILOVER";
        default:
            return "UNKNOWN";
    }
}

/**
 * @brief Get the system uptime in seconds
 * 
 * @return uint64_t Uptime in seconds
 */
uint64_t charmctl_get_uptime(void) {
    if (!g_charm.initialized) {
        return 0;
    }
    
    return (uint64_t)(time(NULL) - g_charm.start_time);
}

/**
 * @brief Force a state transition (for testing)
 * 
 * @param state New state to set
 * @return int 0 on success, non-zero on failure
 */
int charmctl_force_state(charm_system_state_t state) {
    if (!g_charm.initialized) {
        return -1;
    }
    
    charm_system_state_t old_state = g_charm.state;
    g_charm.state = state;
    
    // Call the callback to handle the transition
    state_change_callback(old_state, state);
    
    return 0;
}
