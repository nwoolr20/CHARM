
/**
 * @file charm.h
 * @brief Main header file for the CHARM system
 */

#ifndef CHARM_H
#define CHARM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief CHARM digest size in bytes
 */
#define CHARM_DIGEST_SIZE 32

/**
 * @brief Hex digest size in bytes (including null terminator)
 */
#define CHARM_HEX_DIGEST_SIZE (CHARM_DIGEST_SIZE * 2 + 1)

/**
 * @brief Maximum entropy buffer size
 */
#define CHARM_MAX_ENTROPY_BUFFER 4096

/**
 * @brief Default entropy quality threshold
 */
#define CHARM_DEFAULT_ENTROPY_THRESHOLD 0.6

/**
 * @brief CHARM status codes
 */
typedef enum {
    CHARM_SUCCESS = 0,
    CHARM_ERROR_INVALID_PARAMETER = -1,
    CHARM_ERROR_NOT_INITIALIZED = -2,
    CHARM_ERROR_MEMORY_ALLOCATION = -3,
    CHARM_ERROR_IO = -4,
    CHARM_ERROR_ENTROPY_LOW = -5,
    CHARM_ERROR_SYSTEM = -6
} charm_status_t;

/**
 * @brief Entropy source information
 */
typedef struct {
    char name[32];              // Source name
    double quality;             // Quality estimate (0.0-1.0)
    uint64_t bytes_contributed; // Total bytes contributed
    uint64_t last_update;       // Timestamp of last update
    bool active;                // Whether source is active
} charm_entropy_source_t;

/**
 * @brief Digest context for streaming operations
 */
typedef struct {
    uint32_t state[8];          // Hash state
    uint8_t buffer[64];         // Input buffer
    uint64_t total_bytes;       // Total bytes processed
    uint8_t buffer_size;        // Current buffer fill level
    bool finalized;             // Whether context is finalized
} charm_digest_ctx_t;

/**
 * @brief System states
 */
typedef enum {
    CHARM_STATE_INIT,      // System initializing
    CHARM_STATE_RUN,       // Normal operation
    CHARM_STATE_DEGRADED,  // Degraded entropy quality
    CHARM_STATE_FAILOVER   // Using fallback entropy sources
} charm_system_state_t;

#ifdef __cplusplus
}
#endif

#endif /* CHARM_H */