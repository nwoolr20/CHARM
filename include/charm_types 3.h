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

#ifndef CHARM_TYPES_H
#define CHARM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Entropy source type
 */
typedef enum {
    CHARM_SOURCE_TYPE_NONE = 0,        /**< No source */
    CHARM_SOURCE_TYPE_HARDWARE = 1,    /**< Hardware source */
    CHARM_SOURCE_TYPE_SYSTEM = 2,      /**< System source */
    CHARM_SOURCE_TYPE_EXTERNAL = 3,    /**< External source */
    CHARM_SOURCE_TYPE_USER = 4,        /**< User-provided source */
    CHARM_SOURCE_TYPE_SYNTHETIC = 5,   /**< Synthetic source */
    CHARM_SOURCE_TYPE_CUSTOM = 6       /**< Custom source */
} charm_source_type_t;

/**
 * @brief Entropy quality level
 */
typedef enum {
    CHARM_QUALITY_UNKNOWN = 0,     /**< Unknown quality */
    CHARM_QUALITY_LOW = 1,         /**< Low quality */
    CHARM_QUALITY_MEDIUM = 2,      /**< Medium quality */
    CHARM_QUALITY_HIGH = 3,        /**< High quality */
    CHARM_QUALITY_VERY_HIGH = 4    /**< Very high quality */
} charm_quality_t;

/**
 * @brief Entropy source status
 */
typedef enum {
    CHARM_SOURCE_STATUS_UNKNOWN = 0,   /**< Unknown status */
    CHARM_SOURCE_STATUS_INACTIVE = 1,  /**< Inactive */
    CHARM_SOURCE_STATUS_ACTIVE = 2,    /**< Active */
    CHARM_SOURCE_STATUS_ERROR = 3,     /**< Error */
    CHARM_SOURCE_STATUS_DEPLETED = 4   /**< Depleted */
} charm_source_status_t;

/**
 * @brief Entropy source information
 */
typedef struct {
    charm_source_type_t type;          /**< Source type */
    charm_source_status_t status;      /**< Source status */
    charm_quality_t quality;           /**< Source quality */
    uint32_t id;                       /**< Source ID */
    uint32_t priority;                 /**< Source priority */
    uint64_t bytes_generated;          /**< Bytes generated */
    uint32_t anomalies_detected;       /**< Anomalies detected */
    float entropy_rate;                /**< Entropy rate (bits per byte) */
    uint32_t volatility;               /**< Volatility */
    uint64_t last_update;              /**< Last update timestamp */
    char name[32];                     /**< Source name */
} charm_source_info_t;

/**
 * @brief Entropy status
 */
typedef struct {
    float quality;                     /**< Overall quality (0.0-1.0) */
    uint32_t volatility;               /**< Overall volatility */
    uint32_t source_diversity;         /**< Source diversity */
    uint64_t bytes_generated;          /**< Total bytes generated */
    uint32_t anomalies_detected;       /**< Total anomalies detected */
    bool source_active[8];             /**< Source active flags */
    float source_quality[8];           /**< Source quality values */
    uint64_t source_bytes[8];          /**< Source bytes generated */
    uint32_t source_anomalies[8];      /**< Source anomalies detected */
} entropy_status_t;

/**
 * @brief Lifecycle state
 */
typedef enum {
    CHARM_LIFECYCLE_STATE_UNINITIALIZED = 0,   /**< Uninitialized */
    CHARM_LIFECYCLE_STATE_INITIALIZING = 1,    /**< Initializing */
    CHARM_LIFECYCLE_STATE_READY = 2,           /**< Ready */
    CHARM_LIFECYCLE_STATE_RUNNING = 3,         /**< Running */
    CHARM_LIFECYCLE_STATE_PAUSED = 4,          /**< Paused */
    CHARM_LIFECYCLE_STATE_STOPPING = 5,        /**< Stopping */
    CHARM_LIFECYCLE_STATE_STOPPED = 6,         /**< Stopped */
    CHARM_LIFECYCLE_STATE_ERROR = 7            /**< Error */
} charm_lifecycle_state_t;

/**
 * @brief SIMD implementation type
 */
typedef enum {
    CHARM_SIMD_NONE = 0,       /**< No SIMD */
    CHARM_SIMD_AVX2 = 1,       /**< AVX2 */
    CHARM_SIMD_NEON = 2,       /**< NEON */
    CHARM_SIMD_FALLBACK = 3    /**< Fallback */
} charm_simd_type_t;

/**
 * @brief SIMD capabilities
 */
typedef struct {
    bool avx;                  /**< AVX support */
    bool avx2;                 /**< AVX2 support */
    bool sse4_2;               /**< SSE4.2 support */
    bool neon;                 /**< NEON support */
    charm_simd_type_t optimal; /**< Optimal implementation */
} charm_simd_capabilities_t;

/**
 * @brief Entropy packet header
 */
typedef struct {
    uint32_t source_id;        /**< Source ID */
    uint32_t size;             /**< Packet size */
    uint32_t sequence;         /**< Sequence number */
    uint32_t volatility;       /**< Volatility */
    uint64_t timestamp;        /**< Timestamp */
    uint32_t flags;            /**< Flags */
    uint32_t reserved;         /**< Reserved */
} charm_packet_header_t;

/**
 * @brief Entropy packet
 */
typedef struct {
    charm_packet_header_t header;  /**< Packet header */
    uint8_t* data;                 /**< Packet data */
} charm_packet_t;

/**
 * @brief Digest context
 */
typedef struct {
    uint8_t state[64];         /**< Internal state */
    uint64_t total_bytes;      /**< Total bytes processed */
    uint32_t block_size;       /**< Block size */
    uint32_t digest_size;      /**< Digest size */
    uint8_t buffer[128];       /**< Input buffer */
    uint32_t buffer_size;      /**< Buffer size */
    bool finalized;            /**< Finalization flag */
} charm_digest_context_t;

/**
 * @brief Stream context
 */
typedef struct {
    charm_digest_context_t digest;    /**< Digest context */
    uint32_t chunk_size;              /**< Chunk size */
    uint32_t buffer_size;             /**< Buffer size */
    uint8_t* buffer;                  /**< Buffer */
    uint32_t buffer_used;             /**< Buffer used */
    bool initialized;                 /**< Initialization flag */
} charm_stream_context_t;

/**
 * @brief Trace format
 */
typedef enum {
    CHARM_TRACE_FORMAT_TEXT = 0,      /**< Text format */
    CHARM_TRACE_FORMAT_CSV = 1,       /**< CSV format */
    CHARM_TRACE_FORMAT_JSON = 2,      /**< JSON format */
    CHARM_TRACE_FORMAT_BINARY = 3     /**< Binary format */
} charm_trace_format_t;

/**
 * @brief Trace visualization type
 */
typedef enum {
    CHARM_TRACE_VIZ_NONE = 0,         /**< No visualization */
    CHARM_TRACE_VIZ_ASCII = 1,        /**< ASCII visualization */
    CHARM_TRACE_VIZ_HEATMAP = 2,      /**< Heatmap visualization */
    CHARM_TRACE_VIZ_GRAPH = 3,        /**< Graph visualization */
    CHARM_TRACE_VIZ_HISTOGRAM = 4     /**< Histogram visualization */
} charm_trace_viz_t;

/**
 * @brief Benchmark algorithm
 */
typedef enum {
    CHARM_BENCH_ALG_ECE = 0,          /**< ECE algorithm */
    CHARM_BENCH_ALG_SHA256 = 1,       /**< SHA-256 algorithm */
    CHARM_BENCH_ALG_SHA512 = 2,       /**< SHA-512 algorithm */
    CHARM_BENCH_ALG_BLAKE2B = 3,      /**< BLAKE2b algorithm */
    CHARM_BENCH_ALG_BLAKE3 = 4        /**< BLAKE3 algorithm */
} charm_bench_algorithm_t;

/**
 * @brief Benchmark size
 */
typedef enum {
    CHARM_BENCH_SIZE_64B = 0,         /**< 64 bytes */
    CHARM_BENCH_SIZE_256B = 1,        /**< 256 bytes */
    CHARM_BENCH_SIZE_1KB = 2,         /**< 1 KB */
    CHARM_BENCH_SIZE_4KB = 3,         /**< 4 KB */
    CHARM_BENCH_SIZE_16KB = 4,        /**< 16 KB */
    CHARM_BENCH_SIZE_64KB = 5,        /**< 64 KB */
    CHARM_BENCH_SIZE_256KB = 6,       /**< 256 KB */
    CHARM_BENCH_SIZE_1MB = 7,         /**< 1 MB */
    CHARM_BENCH_SIZE_4MB = 8,         /**< 4 MB */
    CHARM_BENCH_SIZE_16MB = 9         /**< 16 MB */
} charm_bench_size_t;

/**
 * @brief Benchmark format
 */
typedef enum {
    CHARM_BENCH_FORMAT_TEXT = 0,      /**< Text format */
    CHARM_BENCH_FORMAT_CSV = 1,       /**< CSV format */
    CHARM_BENCH_FORMAT_JSON = 2,      /**< JSON format */
    CHARM_BENCH_FORMAT_MARKDOWN = 3   /**< Markdown format */
} charm_bench_format_t;

/**
 * @brief Benchmark result
 */
typedef struct {
    charm_bench_algorithm_t algorithm;    /**< Algorithm */
    charm_bench_size_t size;              /**< Size */
    uint64_t cycles;                      /**< CPU cycles */
    double throughput;                    /**< Throughput (MB/s) */
    double entropy_decay;                 /**< Entropy decay rate */
    double time_ms;                       /**< Time (ms) */
} charm_bench_result_t;

#ifdef __cplusplus
}
#endif

#endif /* CHARM_TYPES_H */
