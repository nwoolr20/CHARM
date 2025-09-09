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
 * @file entropy_trace.h
 * @brief Entropy tracing and visualization for the CHARM system
 * 
 * This header defines the entropy tracing and visualization interfaces
 * for the CHARM system, providing tools for monitoring entropy quality
 * and visualizing entropy patterns.
 */

#ifndef ENTROPY_TRACE_H
#define ENTROPY_TRACE_H

#include "charm_status.h"
#include "caeds_flux.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Trace output format
 */
typedef enum {
    TRACE_FORMAT_TEXT = 0,      /**< Plain text format */
    TRACE_FORMAT_CSV = 1,       /**< CSV format */
    TRACE_FORMAT_JSON = 2,      /**< JSON format */
    TRACE_FORMAT_BINARY = 3     /**< Binary format */
} trace_format_t;

/**
 * @brief Trace visualization type
 */
typedef enum {
    TRACE_VIZ_NONE = 0,         /**< No visualization */
    TRACE_VIZ_ASCII = 1,        /**< ASCII visualization */
    TRACE_VIZ_HEATMAP = 2,      /**< Heatmap visualization */
    TRACE_VIZ_GRAPH = 3,        /**< Graph visualization */
    TRACE_VIZ_HISTOGRAM = 4     /**< Histogram visualization */
} trace_viz_t;

/**
 * @brief Trace data point
 */
typedef struct {
    uint64_t timestamp;                     /**< Timestamp in milliseconds */
    double entropy_quality;                 /**< Overall entropy quality (0.0-1.0) */
    double source_quality[CAEDS_FLUX_SOURCE_COUNT]; /**< Source-specific quality */
    uint32_t bytes_generated;               /**< Bytes generated since last point */
    uint32_t anomalies_detected;            /**< Anomalies detected since last point */
    uint8_t volatility;                     /**< Entropy volatility (0-255) */
    uint8_t source_diversity;               /**< Source diversity (0-255) */
} trace_data_point_t;

/**
 * @brief Trace configuration
 */
typedef struct {
    trace_format_t format;                  /**< Output format */
    trace_viz_t visualization;              /**< Visualization type */
    uint32_t sample_interval_ms;            /**< Sampling interval in milliseconds */
    uint32_t max_points;                    /**< Maximum number of data points to store */
    bool include_raw_samples;               /**< Include raw entropy samples */
    bool include_source_details;            /**< Include source-specific details */
    bool include_anomalies;                 /**< Include anomaly details */
    bool include_predictions;               /**< Include prediction details */
    char output_file[256];                  /**< Output file path */
} trace_config_t;

/**
 * @brief Trace statistics
 */
typedef struct {
    uint64_t start_time;                    /**< Trace start time */
    uint64_t end_time;                      /**< Trace end time */
    uint32_t points_collected;              /**< Number of data points collected */
    uint32_t bytes_traced;                  /**< Total bytes traced */
    uint32_t anomalies_detected;            /**< Total anomalies detected */
    double min_entropy_quality;             /**< Minimum entropy quality */
    double max_entropy_quality;             /**< Maximum entropy quality */
    double avg_entropy_quality;             /**< Average entropy quality */
} trace_stats_t;

/**
 * @brief Trace context handle
 */
typedef struct trace_context* trace_handle_t;

/**
 * @brief Default trace configuration
 */
extern const trace_config_t TRACE_DEFAULT_CONFIG;

/**
 * @brief Initialize entropy trace
 * 
 * @param config Configuration (NULL for default)
 * @return trace_handle_t Handle or NULL on failure
 */
trace_handle_t trace_init(const trace_config_t* config);

/**
 * @brief Shutdown entropy trace
 * 
 * @param handle Trace handle
 */
void trace_shutdown(trace_handle_t handle);

/**
 * @brief Start entropy tracing
 * 
 * @param handle Trace handle
 * @return charm_status_t Status code
 */
charm_status_t trace_start(trace_handle_t handle);

/**
 * @brief Stop entropy tracing
 * 
 * @param handle Trace handle
 * @return charm_status_t Status code
 */
charm_status_t trace_stop(trace_handle_t handle);

/**
 * @brief Add data point to trace
 * 
 * @param handle Trace handle
 * @param point Data point
 * @return charm_status_t Status code
 */
charm_status_t trace_add_point(trace_handle_t handle, const trace_data_point_t* point);

/**
 * @brief Add raw entropy sample to trace
 * 
 * @param handle Trace handle
 * @param source Entropy source
 * @param sample Sample buffer
 * @param size Sample size in bytes
 * @return charm_status_t Status code
 */
charm_status_t trace_add_sample(trace_handle_t handle, caeds_flux_source_t source, const uint8_t* sample, size_t size);

/**
 * @brief Add anomaly to trace
 * 
 * @param handle Trace handle
 * @param source Entropy source
 * @param anomaly_type Anomaly type
 * @param confidence Confidence level (0.0-1.0)
 * @param description Anomaly description
 * @return charm_status_t Status code
 */
charm_status_t trace_add_anomaly(trace_handle_t handle, caeds_flux_source_t source, uint32_t anomaly_type, double confidence, const char* description);

/**
 * @brief Get trace statistics
 * 
 * @param handle Trace handle
 * @param stats Statistics structure
 * @return charm_status_t Status code
 */
charm_status_t trace_get_stats(trace_handle_t handle, trace_stats_t* stats);

/**
 * @brief Save trace to file
 * 
 * @param handle Trace handle
 * @param filename Output filename (NULL to use configured filename)
 * @return charm_status_t Status code
 */
charm_status_t trace_save(trace_handle_t handle, const char* filename);

/**
 * @brief Generate visualization
 * 
 * @param handle Trace handle
 * @param viz_type Visualization type
 * @param filename Output filename
 * @return charm_status_t Status code
 */
charm_status_t trace_visualize(trace_handle_t handle, trace_viz_t viz_type, const char* filename);

/**
 * @brief Print trace summary to stdout
 * 
 * @param handle Trace handle
 * @return charm_status_t Status code
 */
charm_status_t trace_print_summary(trace_handle_t handle);

/**
 * @brief Get string representation of trace format
 * 
 * @param format Trace format
 * @return const char* String representation
 */
const char* trace_format_to_string(trace_format_t format);

/**
 * @brief Get string representation of visualization type
 * 
 * @param viz_type Visualization type
 * @return const char* String representation
 */
const char* trace_viz_to_string(trace_viz_t viz_type);

#ifdef __cplusplus
}
#endif

#endif /* ENTROPY_TRACE_H */
