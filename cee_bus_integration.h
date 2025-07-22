/**
 * @file cee_bus_integration.h
 * @brief Integration layer between CEE and the entropy bus
 * 
 * This header defines the integration layer between the CEE (Charm Entropic
 * Engine) subsystem and the central entropy bus, providing functions for
 * entropy routing, quality feedback, and system-wide coordination.
 */

#ifndef CEE_BUS_INTEGRATION_H
#define CEE_BUS_INTEGRATION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "cee_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the CEE-bus integration layer
 * 
 * @param buffer_handle Handle to the CEE buffer
 * @return int 0 on success, non-zero on failure
 */
int cee_bus_integration_init(cee_buffer_handle_t buffer_handle);

/**
 * @brief Shutdown the CEE-bus integration layer
 * 
 * @return int 0 on success, non-zero on failure
 */
int cee_bus_integration_shutdown(void);

/**
 * @brief Push entropy from CEE buffer to the entropy bus
 * 
 * @param size Number of bytes to push
 * @return int Number of bytes pushed, negative on failure
 */
int cee_bus_integration_push(size_t size);

/**
 * @brief Pull entropy from the entropy bus to the CEE buffer
 * 
 * @param size Number of bytes to pull
 * @param quality Quality level to store the entropy at
 * @return int Number of bytes pulled, negative on failure
 */
int cee_bus_integration_pull(size_t size, cee_buffer_quality_t quality);

/**
 * @brief Update the entropy quality estimate on the bus
 * 
 * @return int 0 on success, non-zero on failure
 */
int cee_bus_integration_update_quality(void);

/**
 * @brief Get the current entropy quality from the bus
 * 
 * @return double Quality value between 0.0 (poor) and 1.0 (excellent)
 */
double cee_bus_integration_get_quality(void);

/**
 * @brief Check if the CEE-bus integration is active
 * 
 * @return bool true if active, false otherwise
 */
bool cee_bus_integration_is_active(void);

/**
 * @brief Start the automatic entropy exchange between CEE and the bus
 * 
 * @param push_interval_ms Interval in milliseconds for pushing entropy to the bus
 * @param pull_interval_ms Interval in milliseconds for pulling entropy from the bus
 * @return int 0 on success, non-zero on failure
 */
int cee_bus_integration_start_auto_exchange(int push_interval_ms, int pull_interval_ms);

/**
 * @brief Stop the automatic entropy exchange
 * 
 * @return int 0 on success, non-zero on failure
 */
int cee_bus_integration_stop_auto_exchange(void);

#ifdef __cplusplus
}
#endif

#endif /* CEE_BUS_INTEGRATION_H */
