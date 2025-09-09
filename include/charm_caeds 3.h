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

#ifndef CHARM_CAEDS_H
#define CHARM_CAEDS_H

/**
 * @file charm_caeds.h
 * @brief CAEDS subsystem header for the CHARM v2.0 system
 * 
 * This header defines the CAEDS (Charm Advanced Entropy Diagnostic System)
 * subsystem structures and interfaces.
 */

#include "core/charm.h"
#include "core/charm_status.h"
#include "ml/charm_ml.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CAEDS subsystem identifier
 */
#define CHARM_SUBSYS_CAEDS 1

/**
 * @brief CAEDS context structure
 */
typedef struct {
    void* adm_context;           /**< Anomaly Detection Module context */
    void* efm_context;           /**< Entropy Flow Monitor context */
    void* pae_context;           /**< Predictive Analytics Engine context */
    void* notify_context;        /**< Notification Module context */
    charm_status_t status;       /**< Current status */
    uint32_t flags;              /**< Configuration flags */
} charm_caeds_t;

/**
 * @brief Initialize the CAEDS subsystem
 * 
 * @param ctx Global CHARM context
 * @param config Configuration parameters
 * @return charm_status_t Status code
 */
charm_status_t charm_caeds_init(charm_context ctx, const void* config);

/**
 * @brief Process entropy statistics
 * 
 * @param ctx Global CHARM context
 * @param stats Entropy statistics
 * @return charm_status_t Status code
 */
charm_status_t charm_caeds_process(charm_context ctx, const charm_entropy_stats_t* stats);

/**
 * @brief Get current CAEDS status
 * 
 * @param ctx Global CHARM context
 * @param status Pointer to store status
 * @return charm_status_t Status code
 */
charm_status_t charm_caeds_get_status(charm_context ctx, charm_status_t* status);

/**
 * @brief Shutdown the CAEDS subsystem
 * 
 * @param ctx Global CHARM context
 * @return charm_status_t Status code
 */
charm_status_t charm_caeds_shutdown(charm_context ctx);

#ifdef __cplusplus
}
#endif

#endif /* CHARM_CAEDS_H */
