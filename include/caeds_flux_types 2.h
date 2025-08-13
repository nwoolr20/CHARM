#ifndef CAEDS_FLUX_TYPES_H
#define CAEDS_FLUX_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "caeds_anomaly_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Maximum number of anomaly callbacks
 */
#define CAEDS_ANOMALY_MAX_CALLBACKS 16

/**
 * @brief Maximum number of anomaly sources
 */
#define CAEDS_ANOMALY_MAX_SOURCES 8

/**
 * @brief Entropy source type
 */
typedef enum {
    CAEDS_FLUX_SOURCE_TYPE_NONE = 0,        /**< No source */
    CAEDS_FLUX_SOURCE_TYPE_HARDWARE = 1,    /**< Hardware source */
    CAEDS_FLUX_SOURCE_TYPE_SYSTEM = 2,      /**< System source */
    CAEDS_FLUX_SOURCE_TYPE_EXTERNAL = 3,    /**< External source */
    CAEDS_FLUX_SOURCE_TYPE_USER = 4,        /**< User-provided source */
    CAEDS_FLUX_SOURCE_TYPE_SYNTHETIC = 5,   /**< Synthetic source */
    CAEDS_FLUX_SOURCE_TYPE_CUSTOM = 6       /**< Custom source */
} caeds_flux_source_type_t;

/**
 * @brief Entropy source status
 */
typedef enum {
    CAEDS_FLUX_SOURCE_STATUS_UNKNOWN = 0,   /**< Unknown status */
    CAEDS_FLUX_SOURCE_STATUS_INACTIVE = 1,  /**< Inactive */
    CAEDS_FLUX_SOURCE_STATUS_ACTIVE = 2,    /**< Active */
    CAEDS_FLUX_SOURCE_STATUS_ERROR = 3,     /**< Error */
    CAEDS_FLUX_SOURCE_STATUS_DEPLETED = 4   /**< Depleted */
} caeds_flux_source_status_t;

/**
 * @brief Entropy source
 */
typedef struct {
    caeds_flux_source_type_t type;          /**< Source type */
    caeds_flux_source_status_t status;      /**< Source status */
    uint32_t id;                            /**< Source ID */
    uint32_t priority;                      /**< Source priority */
    uint64_t bytes_generated;               /**< Bytes generated */
    uint32_t anomalies_detected;            /**< Anomalies detected */
    float entropy_rate;                     /**< Entropy rate (bits per byte) */
    uint32_t volatility;                    /**< Volatility */
    uint64_t last_update;                   /**< Last update timestamp */
    char name[32];                          /**< Source name */
    void* context;                          /**< Source context */
} caeds_flux_source_t;

/**
 * @brief Forward declaration of anomaly source state
 */
struct caeds_anomaly_source_state;
typedef struct caeds_anomaly_source_state caeds_anomaly_source_state_t;

/**
 * @brief Forward declaration of anomaly event
 */
struct caeds_anomaly_event;
typedef struct caeds_anomaly_event caeds_anomaly_event_t;

/**
 * @brief Anomaly callback function
 */
typedef void (*caeds_anomaly_callback_t)(const caeds_anomaly_event_t* event, void* user_data);

/**
 * @brief Anomaly detection context
 */
struct caeds_anomaly_context {
    uint32_t window_size;                                  /**< Analysis window size */
    uint32_t threshold;                                    /**< Detection threshold */
    uint32_t min_samples;                                  /**< Minimum samples required */
    uint32_t max_anomalies;                                /**< Maximum anomalies to track */
    bool auto_reset;                                       /**< Auto-reset after detection */
    uint32_t reset_interval;                               /**< Reset interval in milliseconds */
    uint32_t source_mask;                                  /**< Source mask */
    void* context;                                         /**< User context */
    caeds_anomaly_source_state_t sources[CAEDS_ANOMALY_MAX_SOURCES]; /**< Source states */
    caeds_anomaly_callback_t callbacks[CAEDS_ANOMALY_MAX_CALLBACKS]; /**< Callbacks */
    void* user_data[CAEDS_ANOMALY_MAX_CALLBACKS];                    /**< User data for callbacks */
    uint32_t callback_count;                               /**< Number of registered callbacks */
    uint64_t last_reset;                                   /**< Last reset timestamp */
    void* ml_model;                                        /**< Machine learning model */
    pthread_mutex_t mutex;                                 /**< Mutex for thread safety */
};

typedef struct caeds_anomaly_context caeds_anomaly_context_t;

#ifdef __cplusplus
}
#endif

#endif /* CAEDS_FLUX_TYPES_H */
