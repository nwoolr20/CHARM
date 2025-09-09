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

#ifndef CHARM_CONTEXT_H
#define CHARM_CONTEXT_H

/**
 * @file charm_context.h
 * @brief Global context structure for the CHARM v2.0 system
 * 
 * This header defines the global context structure that maintains state
 * across all CHARM subsystems and provides inter-subsystem communication.
 */

#include "core/charm_status.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of subsystems
 */
#define CHARM_MAX_SUBSYSTEMS 8

/**
 * @brief Subsystem identifiers
 */
typedef enum {
    CHARM_SUBSYS_CORE = 0,       /**< Core subsystem */
    CHARM_SUBSYS_CAEDS = 1,      /**< CAEDS subsystem */
    CHARM_SUBSYS_CEE = 2,        /**< CEE subsystem */
    CHARM_SUBSYS_ECE = 3,        /**< ECE subsystem */
    CHARM_SUBSYS_ENTROPY_BUS = 4, /**< Entropy bus subsystem */
    CHARM_SUBSYS_CLI = 5,         /**< CLI subsystem */
    CHARM_SUBSYS_DRIVERS = 6,     /**< Hardware drivers subsystem */
    CHARM_SUBSYS_WATCHDOG = 7     /**< Watchdog subsystem */
} charm_subsystem_id_t;

/**
 * @brief Global context structure
 */
struct charm_context_s {
    void* subsystems[CHARM_MAX_SUBSYSTEMS]; /**< Subsystem contexts */
    charm_status_t status;                  /**< Global system status */
    uint32_t active_subsystems;             /**< Bitmap of active subsystems */
    uint32_t flags;                         /**< Global flags */
    void* user_data;                        /**< User-defined data */
};

/**
 * @brief Create a new CHARM context
 * 
 * @param ctx Pointer to store the created context
 * @return charm_status_t Status code
 */
charm_status_t charm_context_create(charm_context* ctx);

/**
 * @brief Destroy a CHARM context
 * 
 * @param ctx Context to destroy
 * @return charm_status_t Status code
 */
charm_status_t charm_context_destroy(charm_context ctx);

/**
 * @brief Get subsystem context
 * 
 * @param ctx Global context
 * @param subsys Subsystem ID
 * @param subsys_ctx Pointer to store subsystem context
 * @return charm_status_t Status code
 */
charm_status_t charm_context_get_subsystem(charm_context ctx, 
                                          charm_subsystem_id_t subsys,
                                          void** subsys_ctx);

/**
 * @brief Set subsystem context
 * 
 * @param ctx Global context
 * @param subsys Subsystem ID
 * @param subsys_ctx Subsystem context
 * @return charm_status_t Status code
 */
charm_status_t charm_context_set_subsystem(charm_context ctx,
                                          charm_subsystem_id_t subsys,
                                          void* subsys_ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_CONTEXT_H */
