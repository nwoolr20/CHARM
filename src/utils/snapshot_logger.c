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
 * snapshot_logger.c - CHARM Entropy Snapshot Logging System
 * 
 * This module provides functionality for capturing, storing, and analyzing
 * entropy snapshots throughout the CHARM system lifecycle. It maintains
 * a circular buffer of entropy state snapshots that can be used for:
 * - Debugging entropy anomalies
 * - Providing audit trails for entropy transitions
 * - Supporting entropy health monitoring
 * - Facilitating post-mortem analysis
 *
 * The snapshot logger is designed to have minimal performance impact
 * while providing comprehensive visibility into the entropy subsystem.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include <errno.h>

#include "charm.h"

#define SNAPSHOT_BUFFER_SIZE 128
#define SNAPSHOT_LOG_PATH "./logs/entropy_snapshots.log"
#define SNAPSHOT_BINARY_PATH "./logs/entropy_snapshots.bin"

typedef struct {
    time_t timestamp;
    uint64_t sequence_id;
    charm_system_state_t entropy_state;
    double entropy_density;
    uint32_t anomaly_count;
    uint32_t source_flags;
    uint8_t entropy_sample[64];
    char source_description[128];
} entropy_snapshot_t;

static entropy_snapshot_t snapshot_buffer[SNAPSHOT_BUFFER_SIZE];
static uint64_t current_sequence_id = 0;
static uint32_t buffer_position = 0;
static pthread_mutex_t snapshot_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *snapshot_log_file = NULL;
static bool logging_initialized = false;

/**
 * Initialize the snapshot logging system
 */
int snapshot_logger_init(void) {
    pthread_mutex_lock(&snapshot_mutex);
    
    if (logging_initialized) {
        pthread_mutex_unlock(&snapshot_mutex);
        return 0;
    }
    
    // Create logs directory if it doesn't exist (secure way)
    struct stat st = {0};
    if (stat("./logs", &st) == -1) {
        if (mkdir("./logs", 0755) != 0) {
            fprintf(stderr, "Failed to create logs directory: %s\n", strerror(errno));
            pthread_mutex_unlock(&snapshot_mutex);
            return -1;
        }
    }
    
    // Open log file
    snapshot_log_file = fopen(SNAPSHOT_LOG_PATH, "a");
    if (!snapshot_log_file) {
        fprintf(stderr, "Failed to open snapshot log file: %s\n", SNAPSHOT_LOG_PATH);
        pthread_mutex_unlock(&snapshot_mutex);
        return -1;
    }
    
    // Initialize buffer
    memset(snapshot_buffer, 0, sizeof(snapshot_buffer));
    current_sequence_id = 0;
    buffer_position = 0;
    
    // Write header to log file
    fprintf(snapshot_log_file, 
            "--- CHARM ENTROPY SNAPSHOT LOG INITIALIZED %s ---\n"
            "SEQ_ID,TIMESTAMP,STATE,ENTROPY,ANOMALIES,SOURCES,DESCRIPTION\n",
            ctime(&(time_t){time(NULL)}));
    fflush(snapshot_log_file);
    
    logging_initialized = true;
    pthread_mutex_unlock(&snapshot_mutex);
    
    return 0;
}

/**
 * Capture an entropy snapshot with the current system state
 */
int snapshot_logger_capture(charm_system_state_t state, 
                           double entropy_density,
                           uint32_t anomaly_count,
                           uint32_t source_flags,
                           const uint8_t *entropy_sample,
                           const char *source_description) {
    if (!logging_initialized) {
        if (snapshot_logger_init() != 0) {
            return -1;
        }
    }
    
    pthread_mutex_lock(&snapshot_mutex);
    
    // Create new snapshot
    entropy_snapshot_t *snapshot = &snapshot_buffer[buffer_position];
    snapshot->timestamp = time(NULL);
    snapshot->sequence_id = current_sequence_id++;
    snapshot->entropy_state = state;
    snapshot->entropy_density = entropy_density;
    snapshot->anomaly_count = anomaly_count;
    snapshot->source_flags = source_flags;
    
    if (entropy_sample) {
        memcpy(snapshot->entropy_sample, entropy_sample, 64);
    } else {
        memset(snapshot->entropy_sample, 0, 64);
    }
    
    if (source_description) {
        strncpy(snapshot->source_description, source_description, 127);
        snapshot->source_description[127] = '\0';
    } else {
        strncpy(snapshot->source_description, "Unknown", 127);
        snapshot->source_description[127] = '\0';
    }
    
    // Write to log file
    if (snapshot_log_file) {
        fprintf(snapshot_log_file, 
                "%lu,%lu,%d,%.4f,%u,0x%08x,%s\n",
                snapshot->sequence_id,
                (unsigned long)snapshot->timestamp,
                snapshot->entropy_state,
                snapshot->entropy_density,
                snapshot->anomaly_count,
                snapshot->source_flags,
                snapshot->source_description);
        fflush(snapshot_log_file);
    }
    
    // Update buffer position (circular buffer)
    buffer_position = (buffer_position + 1) % SNAPSHOT_BUFFER_SIZE;
    
    pthread_mutex_unlock(&snapshot_mutex);
    
    return 0;
}

/**
 * Dump the current snapshot buffer to a binary file
 */
int snapshot_logger_dump(void) {
    if (!logging_initialized) {
        return -1;
    }
    
    pthread_mutex_lock(&snapshot_mutex);
    
    FILE *binary_file = fopen(SNAPSHOT_BINARY_PATH, "wb");
    if (!binary_file) {
        pthread_mutex_unlock(&snapshot_mutex);
        return -1;
    }
    
    // Write header
    uint32_t header[4] = {
        0x4348524D,  // "CHRM"
        0x534E4150,  // "SNAP"
        SNAPSHOT_BUFFER_SIZE,
        (uint32_t)current_sequence_id
    };
    
    fwrite(header, sizeof(uint32_t), 4, binary_file);
    
    // Write snapshots in chronological order
    uint32_t start_pos = (current_sequence_id >= SNAPSHOT_BUFFER_SIZE) ? 
                         buffer_position : 0;
    
    for (uint32_t i = 0; i < SNAPSHOT_BUFFER_SIZE; i++) {
        uint32_t pos = (start_pos + i) % SNAPSHOT_BUFFER_SIZE;
        fwrite(&snapshot_buffer[pos], sizeof(entropy_snapshot_t), 1, binary_file);
    }
    
    fclose(binary_file);
    pthread_mutex_unlock(&snapshot_mutex);
    
    return 0;
}

/**
 * Clean up the snapshot logger resources
 */
void snapshot_logger_cleanup(void) {
    pthread_mutex_lock(&snapshot_mutex);
    
    if (snapshot_log_file) {
        fprintf(snapshot_log_file, 
                "--- CHARM ENTROPY SNAPSHOT LOG CLOSED %s ---\n",
                ctime(&(time_t){time(NULL)}));
        fclose(snapshot_log_file);
        snapshot_log_file = NULL;
    }
    
    logging_initialized = false;
    pthread_mutex_unlock(&snapshot_mutex);
}

/**
 * Get the latest entropy snapshot
 */
entropy_snapshot_t *snapshot_logger_get_latest(void) {
    if (!logging_initialized) {
        return NULL;
    }
    
    pthread_mutex_lock(&snapshot_mutex);
    
    // Get previous position (most recent snapshot)
    uint32_t latest_pos = (buffer_position == 0) ? 
                         SNAPSHOT_BUFFER_SIZE - 1 : buffer_position - 1;
    
    entropy_snapshot_t *snapshot = &snapshot_buffer[latest_pos];
    pthread_mutex_unlock(&snapshot_mutex);
    
    return snapshot;
}

/**
 * Log a critical entropy event with additional context
 */
void snapshot_logger_critical_event(const char *event_type, 
                                   const char *description,
                                   charm_system_state_t state) {
    if (!logging_initialized) {
        if (snapshot_logger_init() != 0) {
            return;
        }
    }
    
    pthread_mutex_lock(&snapshot_mutex);
    
    if (snapshot_log_file) {
        fprintf(snapshot_log_file, 
                "!!! CRITICAL EVENT [%s] at %s"
                "    State: %d, Description: %s\n",
                event_type,
                ctime(&(time_t){time(NULL)}),
                state,
                description);
        fflush(snapshot_log_file);
    }
    
    pthread_mutex_unlock(&snapshot_mutex);
    
    // Force a snapshot dump on critical events
    snapshot_logger_dump();
}
