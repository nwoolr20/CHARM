
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

#ifdef __cplusplus
}