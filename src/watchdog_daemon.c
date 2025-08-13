/**
 * @file watchdog_daemon.c
 * @brief Runtime entropy monitoring daemon
 * 
 * This module implements the watchdog daemon for the CHARM system,
 * responsible for monitoring entropy levels, detecting anomalies,
 * and triggering appropriate responses when entropy quality degrades.
 * 
 * The daemon runs as a background process, continuously monitoring
 * the entropy bus and system state, and can initiate fallback mechanisms
 * when necessary.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// System states
typedef enum {
    CHARM_STATE_INIT,      // System initializing
    CHARM_STATE_RUN,       // Normal operation
    CHARM_STATE_DEGRADED,  // Degraded entropy quality
    CHARM_STATE_FAILOVER   // Using fallback entropy sources
} charm_system_state_t;

// Watchdog configuration
typedef struct {
    unsigned int check_interval_ms;  // Interval between checks in milliseconds
    double entropy_threshold;        // Minimum acceptable entropy quality (0.0-1.0)
    unsigned int degraded_samples;   // Number of consecutive degraded samples before alert
    bool auto_failover;              // Whether to automatically switch to fallback
} watchdog_config_t;

// Watchdog state
typedef struct {
    pthread_t thread_id;
    bool running;
    charm_system_state_t system_state;
    unsigned int degraded_count;
    time_t last_check_time;
    double last_entropy_quality;
    watchdog_config_t config;
    void (*state_change_callback)(charm_system_state_t old_state, 
                                 charm_system_state_t new_state);
} watchdog_state_t;

// Global watchdog state
static watchdog_state_t g_watchdog = {
    .running = false,
    .system_state = CHARM_STATE_INIT,
    .degraded_count = 0,
    .last_check_time = 0,
    .last_entropy_quality = 1.0,
    .config = {
        .check_interval_ms = 1000,  // Default: check every second
        .entropy_threshold = 0.7,    // Default: 70% quality threshold
        .degraded_samples = 3,       // Default: 3 consecutive degraded samples
        .auto_failover = true        // Default: auto-failover enabled
    },
    .state_change_callback = NULL
};

// Forward declarations of external functions
extern double entropy_bus_get_quality(void);
extern int fallback_entropy_activate(void);
extern int fallback_entropy_deactivate(void);
extern void caeds_notify_send(const char* message, int severity);

/**
 * @brief Set the system state and trigger callbacks
 * 
 * @param new_state New system state
 */
static void set_system_state(charm_system_state_t new_state) {
    if (new_state == g_watchdog.system_state) {
        return; // No change
    }
    
    charm_system_state_t old_state = g_watchdog.system_state;
    g_watchdog.system_state = new_state;
    
    // Log the state change
    char message[128];
    snprintf(message, sizeof(message), "System state changed: %d -> %d", 
             old_state, new_state);
    caeds_notify_send(message, (new_state == CHARM_STATE_RUN) ? 0 : 2);
    
    // Call the state change callback if registered
    if (g_watchdog.state_change_callback != NULL) {
        g_watchdog.state_change_callback(old_state, new_state);
    }
}

/**
 * @brief Watchdog thread function
 * 
 * @param arg Thread argument (unused)
 * @return void* Return value (unused)
 */
static void* watchdog_thread(void* arg) {
    (void)arg; // Unused
    
    while (g_watchdog.running) {
        // Get current entropy quality
        double quality = entropy_bus_get_quality();
        g_watchdog.last_entropy_quality = quality;
        g_watchdog.last_check_time = time(NULL);
        
        // Check against threshold
        if (quality < g_watchdog.config.entropy_threshold) {
            g_watchdog.degraded_count++;
            
            // If we've seen enough degraded samples, take action
            if (g_watchdog.degraded_count >= g_watchdog.config.degraded_samples) {
                if (g_watchdog.system_state == CHARM_STATE_RUN) {
                    // Transition to DEGRADED state
                    set_system_state(CHARM_STATE_DEGRADED);
                    
                    // Send notification
                    char message[128];
                    snprintf(message, sizeof(message), 
                             "Entropy quality degraded: %.2f (threshold: %.2f)",
                             quality, g_watchdog.config.entropy_threshold);
                    caeds_notify_send(message, 2);
                    
                    // If auto-failover is enabled, activate fallback
                    if (g_watchdog.config.auto_failover) {
                        if (fallback_entropy_activate() == 0) {
                            set_system_state(CHARM_STATE_FAILOVER);
                            caeds_notify_send("Fallback entropy source activated", 1);
                        } else {
                            caeds_notify_send("Failed to activate fallback entropy source", 3);
                        }
                    }
                }
            }
        } else {
            // Quality is good
            g_watchdog.degraded_count = 0;
            
            // If we're in FAILOVER or DEGRADED, check if we can return to normal
            if (g_watchdog.system_state == CHARM_STATE_FAILOVER ||
                g_watchdog.system_state == CHARM_STATE_DEGRADED) {
                
                // Only return to normal if quality is significantly above threshold
                if (quality > g_watchdog.config.entropy_threshold + 0.1) {
                    if (g_watchdog.system_state == CHARM_STATE_FAILOVER) {
                        fallback_entropy_deactivate();
                    }
                    
                    set_system_state(CHARM_STATE_RUN);
                    caeds_notify_send("Entropy quality restored, returning to normal operation", 1);
                }
            } else if (g_watchdog.system_state == CHARM_STATE_INIT) {
                // Transition from INIT to RUN
                set_system_state(CHARM_STATE_RUN);
                caeds_notify_send("System initialized and running normally", 0);
            }
        }
        
        // Sleep until next check
        usleep(g_watchdog.config.check_interval_ms * 1000);
    }
    
    return NULL;
}

/**
 * @brief Start the watchdog daemon
 * 
 * @return int 0 on success, non-zero on failure
 */
int watchdog_daemon_start(void) {
    if (g_watchdog.running) {
        return 0; // Already running
    }
    
    g_watchdog.running = true;
    g_watchdog.system_state = CHARM_STATE_INIT;
    g_watchdog.degraded_count = 0;
    
    // Create watchdog thread
    if (pthread_create(&g_watchdog.thread_id, NULL, watchdog_thread, NULL) != 0) {
        g_watchdog.running = false;
        return -1;
    }
    
    return 0;
}

/**
 * @brief Stop the watchdog daemon
 * 
 * @return int 0 on success, non-zero on failure
 */
int watchdog_daemon_stop(void) {
    if (!g_watchdog.running) {
        return 0; // Already stopped
    }
    
    g_watchdog.running = false;
    
    // Wait for thread to exit
    pthread_join(g_watchdog.thread_id, NULL);
    
    return 0;
}

/**
 * @brief Configure the watchdog daemon
 * 
 * @param config Configuration parameters
 * @return int 0 on success, non-zero on failure
 */
int watchdog_daemon_configure(const watchdog_config_t* config) {
    if (config == NULL) {
        return -1;
    }
    
    // Copy configuration
    g_watchdog.config = *config;
    
    return 0;
}

/**
 * @brief Register a callback for state changes
 * 
 * @param callback Function to call when system state changes
 */
void watchdog_daemon_register_callback(void (*callback)(charm_system_state_t old_state, 
                                                      charm_system_state_t new_state)) {
    g_watchdog.state_change_callback = callback;
}

/**
 * @brief Get the current system state
 * 
 * @return charm_system_state_t Current system state
 */
charm_system_state_t watchdog_daemon_get_state(void) {
    return g_watchdog.system_state;
}

/**
 * @brief Get the last measured entropy quality
 * 
 * @return double Last entropy quality measurement (0.0-1.0)
 */
double watchdog_daemon_get_quality(void) {
    return g_watchdog.last_entropy_quality;
}

/**
 * @brief Force a state transition (for testing or manual control)
 * 
 * @param state New state to set
 * @return int 0 on success, non-zero on failure
 */
int watchdog_daemon_force_state(charm_system_state_t state) {
    if (state < CHARM_STATE_INIT || state > CHARM_STATE_FAILOVER) {
        return -1;
    }
    
    set_system_state(state);
    
    // If transitioning to FAILOVER, activate fallback
    if (state == CHARM_STATE_FAILOVER) {
        fallback_entropy_activate();
    }
    // If transitioning from FAILOVER, deactivate fallback
    else if (g_watchdog.system_state == CHARM_STATE_FAILOVER) {
        fallback_entropy_deactivate();
    }
    
    return 0;
}
