/**
 * @file ece_bus_integration.h
 * @brief Integration of ECE with the entropy bus
 * 
 * This header defines the interface for integrating the ECE (Entropic Collapse
 * Engine) with the entropy bus, allowing for entropy-aware hashing and
 * cryptographic operations that leverage the system's entropy state.
 */

#ifndef ECE_BUS_INTEGRATION_H
#define ECE_BUS_INTEGRATION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "ece_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECE bus integration status codes
 */
typedef enum {
    ECE_BUS_STATUS_OK = 0,           /**< Operation successful */
    ECE_BUS_STATUS_ERROR = -1,       /**< General error */
    ECE_BUS_STATUS_INVALID_ARG = -2, /**< Invalid argument */
    ECE_BUS_STATUS_NO_ENTROPY = -3,  /**< Insufficient entropy */
    ECE_BUS_STATUS_NOT_INITIALIZED = -4 /**< Not initialized */
} ece_bus_status_t;

/**
 * @brief Initialize the ECE bus integration
 * 
 * @param consumer_id Consumer ID for entropy bus
 * @param min_entropy_quality Minimum entropy quality required (0.0-1.0)
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_init(int consumer_id, double min_entropy_quality);

/**
 * @brief Shutdown the ECE bus integration
 * 
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_shutdown(void);

/**
 * @brief Get entropy from the bus for ECE operations
 * 
 * @param buffer Buffer to store entropy
 * @param size Size of buffer in bytes
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_get_entropy(uint8_t* buffer, size_t size);

/**
 * @brief Create a hash of data using ECE with entropy from the bus
 * 
 * @param data Input data
 * @param data_size Size of input data in bytes
 * @param digest Output buffer for digest
 * @param digest_size Size of digest buffer in bytes
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_hash(const uint8_t* data, size_t data_size,
                                         uint8_t* digest, size_t digest_size);

/**
 * @brief Set the minimum entropy quality required for operations
 * 
 * @param quality Minimum entropy quality (0.0-1.0)
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_set_min_quality(double quality);

/**
 * @brief Get the current entropy quality from the bus
 * 
 * @param quality Pointer to store quality value
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_get_quality(double* quality);

/**
 * @brief Check if the current entropy quality meets the minimum requirement
 * 
 * @param meets_requirement Pointer to store result (true if meets requirement)
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_check_quality(bool* meets_requirement);

/**
 * @brief Adapt ECE parameters based on current entropy quality
 * 
 * @param handle ECE handle to adapt
 * @return ece_bus_status_t Status code
 */
ece_bus_status_t ece_bus_integration_adapt_parameters(ece_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* ECE_BUS_INTEGRATION_H */
