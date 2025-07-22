/**
 * @file charmctl.h
 * @brief Lifecycle coordinator for the CHARM v2.0 system
 * 
 * This header defines the interfaces for the lifecycle coordinator for the CHARM system,
 * responsible for initialization, state management, and shutdown of all
 * subsystems.
 */

#ifndef CHARMCTL_H
#define CHARMCTL_H

#include "charm_status.h"
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief System states
 */
typedef enum {
    CHARM_STATE_INIT = 0,      /**< System initializing */
    CHARM_STATE_MONITORING = 1, /**< Monitoring entropy sources */
    CHARM_STATE_RUNNING = 2,    /**< Normal operation */
    CHARM_STATE_DEGRADED = 3,   /**< Degraded entropy quality */
    CHARM_STATE_FAILOVER = 4    /**< Using fallback entropy sources */
} charm_system_state_t;

/**
 * @brief State change callback function type
 * 
 * @param user_data User-defined data
 * @param old_state Previous state
 * @param new_state New state
 */
typedef void (*charm_state_callback_t)(void* user_data, 
                                     charm_system_state_t old_state, 
                                     charm_system_state_t new_state);

/**
 * @brief Initialize the CHARM system
 * 
 * @param ctx CHARM context
 * @return charm_status_t Status code
 */
charm_status_t charmctl_init(charm_context_t ctx);

/**
 * @brief Register a state change callback
 * 
 * @param ctx CHARM context
 * @param callback Callback function
 * @param user_data User-defined data
 * @return charm_status_t Status code
 */
charm_status_t charmctl_register_callback(charm_context_t ctx,
                                        charm_state_callback_t callback,
                                        void* user_data);

/**
 * @brief Unregister a state change callback
 * 
 * @param ctx CHARM context
 * @param callback Callback function to unregister
 * @return charm_status_t Status code
 */
charm_status_t charmctl_unregister_callback(charm_context_t ctx,
                                          charm_state_callback_t callback);

/**
 * @brief Get the current system state
 * 
 * @param ctx CHARM context
 * @param state Pointer to receive system state
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_state(charm_context_t ctx,
                                charm_system_state_t* state);

/**
 * @brief Get the Entropy Health Score (EHS)
 * 
 * @param ctx CHARM context
 * @param ehs Pointer to receive EHS value (0-100)
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_ehs(charm_context_t ctx,
                              double* ehs);

/**
 * @brief Get detailed Entropy Health Score (EHS) components
 * 
 * @param ctx CHARM context
 * @param variance Pointer to receive variance score (0-100)
 * @param source_diversity Pointer to receive source diversity score (0-100)
 * @param volatility Pointer to receive volatility score (0-100)
 * @param frequency Pointer to receive frequency score (0-100)
 * @param overall Pointer to receive overall EHS (0-100)
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_ehs_details(charm_context_t ctx,
                                      double* variance,
                                      double* source_diversity,
                                      double* volatility,
                                      double* frequency,
                                      double* overall);

/**
 * @brief Force a state transition (for testing)
 * 
 * @param ctx CHARM context
 * @param state New state to set
 * @return charm_status_t Status code
 */
charm_status_t charmctl_force_state(charm_context_t ctx,
                                  charm_system_state_t state);

/**
 * @brief Get the system uptime in seconds
 * 
 * @param ctx CHARM context
 * @param uptime Pointer to receive uptime in seconds
 * @return charm_status_t Status code
 */
charm_status_t charmctl_get_uptime(charm_context_t ctx,
                                 uint64_t* uptime);

/**
 * @brief Convert system state to string
 * 
 * @param state System state
 * @return const char* String representation of state
 */
const char* charmctl_state_to_string(charm_system_state_t state);

/**
 * @brief Shutdown the CHARM system
 * 
 * @param ctx CHARM context
 * @return charm_status_t Status code
 */
charm_status_t charmctl_shutdown(charm_context_t ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHARMCTL_H */
