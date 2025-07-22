/**
 * @file entropy_trace.c
 * @brief Real-time entropy visualizer/logger
 * 
 * This module implements a real-time entropy visualization and logging system
 * that monitors the entropy bus and provides visual feedback on entropy quality,
 * anomalies, and system state.
 * 
 * It can output to console, log files, or generate visual representations of
 * entropy flow and quality over time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define ENTROPY_TRACE_BUFFER_SIZE 1024
#define ENTROPY_TRACE_DEFAULT_LOG_FILE "entropy_trace.log"
#define ENTROPY_TRACE_HISTORY_SIZE 60  // Keep 60 samples for visualization

// Trace visualization modes
typedef enum {
    TRACE_MODE_TEXT,      // Text-based visualization
    TRACE_MODE_HISTOGRAM, // ASCII histogram
    TRACE_MODE_HEATMAP    // ASCII heatmap
} entropy_trace_mode_t;

// Trace configuration
typedef struct {
    bool enabled;
    bool log_to_file;
    bool log_to_console;
    char log_file[256];
    unsigned int sample_interval_ms;
    entropy_trace_mode_t mode;
} entropy_trace_config_t;

// Trace state
typedef struct {
    pthread_t thread_id;
    bool running;
    FILE* log_file;
    entropy_trace_config_t config;
    
    // History buffer for visualization
    double quality_history[ENTROPY_TRACE_HISTORY_SIZE];
    uint64_t sample_count;
    time_t last_sample_time;
    
    // Statistics
    double min_quality;
    double max_quality;
    double avg_quality;
    unsigned int anomaly_count;
    
    // Mutex for thread safety
    pthread_mutex_t mutex;
} entropy_trace_state_t;

// Global trace state
static entropy_trace_state_t g_trace = {
    .running = false,
    .log_file = NULL,
    .config = {
        .enabled = true,
        .log_to_file = true,
        .log_to_console = true,
        .log_file = ENTROPY_TRACE_DEFAULT_LOG_FILE,
        .sample_interval_ms = 1000,  // Default: sample every second
        .mode = TRACE_MODE_TEXT
    },
    .sample_count = 0,
    .last_sample_time = 0,
    .min_quality = 1.0,
    .max_quality = 0.0,
    .avg_quality = 0.0,
    .anomaly_count = 0
};

// Forward declarations of external functions
extern double entropy_bus_get_quality(void);
extern uint64_t entropy_bus_get_bytes_processed(void);
extern int entropy_bus_get_source_count(void);
extern charm_system_state_t watchdog_daemon_get_state(void);

/**
 * @brief Initialize the trace system
 * 
 * @return int 0 on success, non-zero on failure
 */
int entropy_trace_init(void) {
    // Initialize mutex
    if (pthread_mutex_init(&g_trace.mutex, NULL) != 0) {
        return -1;
    }
    
    // Initialize history buffer
    for (int i = 0; i < ENTROPY_TRACE_HISTORY_SIZE; i++) {
        g_trace.quality_history[i] = 0.0;
    }
    
    // Open log file if enabled
    if (g_trace.config.log_to_file) {
        g_trace.log_file = fopen(g_trace.config.log_file, "a");
        if (g_trace.log_file == NULL) {
            pthread_mutex_destroy(&g_trace.mutex);
            return -1;
        }
        
        // Write header
        time_t now = time(NULL);
        fprintf(g_trace.log_file, "\n--- Entropy Trace Session Started: %s", ctime(&now));
        fprintf(g_trace.log_file, "Time,Quality,BytesProcessed,SourceCount,SystemState\n");
        fflush(g_trace.log_file);
    }
    
    return 0;
}

/**
 * @brief Clean up the trace system
 */
void entropy_trace_cleanup(void) {
    // Close log file if open
    if (g_trace.log_file != NULL) {
        time_t now = time(NULL);
        fprintf(g_trace.log_file, "--- Entropy Trace Session Ended: %s", ctime(&now));
        fclose(g_trace.log_file);
        g_trace.log_file = NULL;
    }
    
    // Destroy mutex
    pthread_mutex_destroy(&g_trace.mutex);
}

/**
 * @brief Generate ASCII histogram of entropy quality
 * 
 * @param buffer Buffer to store the histogram
 * @param size Size of the buffer
 */
static void generate_histogram(char* buffer, size_t size) {
    const int width = 50;  // Width of histogram
    const int height = 10; // Height of histogram
    
    // Clear buffer
    memset(buffer, 0, size);
    
    // Create histogram grid
    char grid[height][width + 1];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            grid[i][j] = ' ';
        }
        grid[i][width] = '\0';
    }
    
    // Fill grid with history data
    for (int i = 0; i < ENTROPY_TRACE_HISTORY_SIZE && i < width; i++) {
        double quality = g_trace.quality_history[(g_trace.sample_count - i - 1) % ENTROPY_TRACE_HISTORY_SIZE];
        if (quality <= 0.0) continue;
        
        int bar_height = (int)(quality * (height - 1));
        for (int j = 0; j <= bar_height; j++) {
            grid[height - 1 - j][width - 1 - i] = '#';
        }
    }
    
    // Build output string
    int pos = 0;
    pos += snprintf(buffer + pos, size - pos, "Entropy Quality Histogram (last %d samples)\n", ENTROPY_TRACE_HISTORY_SIZE);
    pos += snprintf(buffer + pos, size - pos, "1.0 +%s+\n", "-------------------------------------------------");
    
    for (int i = 0; i < height; i++) {
        if (i == 0) {
            pos += snprintf(buffer + pos, size - pos, "    |%s|\n", grid[i]);
        } else if (i == height - 1) {
            pos += snprintf(buffer + pos, size - pos, "0.0 |%s|\n", grid[i]);
        } else {
            pos += snprintf(buffer + pos, size - pos, "    |%s|\n", grid[i]);
        }
    }
    
    pos += snprintf(buffer + pos, size - pos, "    +%s+\n", "-------------------------------------------------");
    pos += snprintf(buffer + pos, size - pos, "      Now %*s Time %*s %d sec ago\n", 
                   width/2 - 5, "", width/2 - 10, "", ENTROPY_TRACE_HISTORY_SIZE);
}

/**
 * @brief Generate ASCII heatmap of entropy distribution
 * 
 * @param buffer Buffer to store the heatmap
 * @param size Size of the buffer
 */
static void generate_heatmap(char* buffer, size_t size) {
    const int width = 16;  // 16 columns for byte values
    const int height = 16; // 16 rows for byte values
    
    // Clear buffer
    memset(buffer, 0, size);
    
    // Create heatmap grid (placeholder - in a real implementation, this would
    // analyze actual entropy byte distribution)
    char grid[height][width + 1];
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            // Simulate entropy distribution - in real implementation,
            // this would be based on actual entropy data
            double quality = g_trace.quality_history[g_trace.sample_count % ENTROPY_TRACE_HISTORY_SIZE];
            double value = quality * (1.0 - 0.3 * ((i ^ j) % 3));
            
            if (value > 0.8) grid[i][j] = '#';
            else if (value > 0.6) grid[i][j] = '+';
            else if (value > 0.4) grid[i][j] = '-';
            else if (value > 0.2) grid[i][j] = '.';
            else grid[i][j] = ' ';
        }
        grid[i][width] = '\0';
    }
    
    // Build output string
    int pos = 0;
    pos += snprintf(buffer + pos, size - pos, "Entropy Byte Distribution Heatmap\n");
    pos += snprintf(buffer + pos, size - pos, "    0 1 2 3 4 5 6 7 8 9 A B C D E F\n");
    pos += snprintf(buffer + pos, size - pos, "   +--------------------------------+\n");
    
    for (int i = 0; i < height; i++) {
        pos += snprintf(buffer + pos, size - pos, "%X  |%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c|\n", 
                       i, grid[i][0], grid[i][1], grid[i][2], grid[i][3],
                       grid[i][4], grid[i][5], grid[i][6], grid[i][7],
                       grid[i][8], grid[i][9], grid[i][10], grid[i][11],
                       grid[i][12], grid[i][13], grid[i][14], grid[i][15]);
    }
    
    pos += snprintf(buffer + pos, size - pos, "   +--------------------------------+\n");
    pos += snprintf(buffer + pos, size - pos, "Legend: # (>80%%) + (>60%%) - (>40%%) . (>20%%)\n");
}

/**
 * @brief Sample entropy and update trace
 */
static void sample_entropy(void) {
    pthread_mutex_lock(&g_trace.mutex);
    
    // Get current entropy quality and stats
    double quality = entropy_bus_get_quality();
    uint64_t bytes = entropy_bus_get_bytes_processed();
    int sources = entropy_bus_get_source_count();
    charm_system_state_t state = watchdog_daemon_get_state();
    
    // Update history buffer
    g_trace.quality_history[g_trace.sample_count % ENTROPY_TRACE_HISTORY_SIZE] = quality;
    g_trace.sample_count++;
    g_trace.last_sample_time = time(NULL);
    
    // Update statistics
    if (quality < g_trace.min_quality) g_trace.min_quality = quality;
    if (quality > g_trace.max_quality) g_trace.max_quality = quality;
    g_trace.avg_quality = (g_trace.avg_quality * (g_trace.sample_count - 1) + quality) / g_trace.sample_count;
    
    // Detect anomalies (simple threshold-based detection)
    if (quality < 0.5) {
        g_trace.anomaly_count++;
    }
    
    // Log to file if enabled
    if (g_trace.config.log_to_file && g_trace.log_file != NULL) {
        fprintf(g_trace.log_file, "%ld,%.4f,%lu,%d,%d\n", 
                time(NULL), quality, bytes, sources, state);
        fflush(g_trace.log_file);
    }
    
    // Log to console if enabled
    if (g_trace.config.log_to_console) {
        char buffer[4096] = {0};
        int pos = 0;
        
        // Basic info
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                       "\n=== Entropy Trace: %s ===\n", ctime(&g_trace.last_sample_time));
        pos += snprintf(buffer + pos, sizeof(buffer) - pos, 
                       "Quality: %.4f  Bytes: %lu  Sources: %d  State: %d\n",
                       quality, bytes, sources, state);
        pos += snprintf(buffer + pos, sizeof(buffer) - pos,
                       "Statistics: Min=%.4f  Max=%.4f  Avg=%.4f  Anomalies=%u\n",
                       g_trace.min_quality, g_trace.max_quality, 
                       g_trace.avg_quality, g_trace.anomaly_count);
        
        // Visualization based on selected mode
        switch (g_trace.config.mode) {
            case TRACE_MODE_HISTOGRAM:
                generate_histogram(buffer + pos, sizeof(buffer) - pos);
                break;
                
            case TRACE_MODE_HEATMAP:
                generate_heatmap(buffer + pos, sizeof(buffer) - pos);
                break;
                
            case TRACE_MODE_TEXT:
            default:
                // Simple text-based visualization
                pos += snprintf(buffer + pos, sizeof(buffer) - pos, "\nEntropy Quality: ");
                int bar_width = (int)(quality * 50.0);
                for (int i = 0; i < 50; i++) {
                    if (i < bar_width) {
                        buffer[pos++] = '#';
                    } else {
                        buffer[pos++] = '-';
                    }
                }
                buffer[pos++] = '\n';
                buffer[pos] = '\0';
                break;
        }
        
        printf("%s\n", buffer);
    }
    
    pthread_mutex_unlock(&g_trace.mutex);
}

/**
 * @brief Trace thread function
 * 
 * @param arg Thread argument (unused)
 * @return void* Return value (unused)
 */
static void* trace_thread(void* arg) {
    (void)arg; // Unused
    
    while (g_trace.running) {
        // Sample entropy
        sample_entropy();
        
        // Sleep until next sample
        usleep(g_trace.config.sample_interval_ms * 1000);
    }
    
    return NULL;
}

/**
 * @brief Start the trace system
 * 
 * @return int 0 on success, non-zero on failure
 */
int entropy_trace_start(void) {
    if (g_trace.running) {
        return 0; // Already running
    }
    
    // Initialize if not already done
    if (g_trace.log_file == NULL && g_trace.config.log_to_file) {
        if (entropy_trace_init() != 0) {
            return -1;
        }
    }
    
    g_trace.running = true;
    
    // Create trace thread
    if (pthread_create(&g_trace.thread_id, NULL, trace_thread, NULL) != 0) {
        g_trace.running = false;
        return -1;
    }
    
    return 0;
}

/**
 * @brief Stop the trace system
 * 
 * @return int 0 on success, non-zero on failure
 */
int entropy_trace_stop(void) {
    if (!g_trace.running) {
        return 0; // Already stopped
    }
    
    g_trace.running = false;
    
    // Wait for thread to exit
    pthread_join(g_trace.thread_id, NULL);
    
    // Clean up
    entropy_trace_cleanup();
    
    return 0;
}

/**
 * @brief Configure the trace system
 * 
 * @param config Configuration parameters
 * @return int 0 on success, non-zero on failure
 */
int entropy_trace_configure(const entropy_trace_config_t* config) {
    if (config == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_trace.mutex);
    
    // Check if log file path changed
    bool log_file_changed = strcmp(g_trace.config.log_file, config->log_file) != 0;
    
    // Copy configuration
    g_trace.config = *config;
    
    // Reopen log file if path changed
    if (log_file_changed && g_trace.config.log_to_file) {
        if (g_trace.log_file != NULL) {
            fclose(g_trace.log_file);
        }
        
        g_trace.log_file = fopen(g_trace.config.log_file, "a");
        if (g_trace.log_file == NULL) {
            pthread_mutex_unlock(&g_trace.mutex);
            return -1;
        }
        
        // Write header
        time_t now = time(NULL);
        fprintf(g_trace.log_file, "\n--- Entropy Trace Session Started: %s", ctime(&now));
        fprintf(g_trace.log_file, "Time,Quality,BytesProcessed,SourceCount,SystemState\n");
        fflush(g_trace.log_file);
    }
    
    pthread_mutex_unlock(&g_trace.mutex);
    
    return 0;
}

/**
 * @brief Take a manual entropy sample
 * 
 * @return double Current entropy quality
 */
double entropy_trace_sample(void) {
    sample_entropy();
    return g_trace.quality_history[g_trace.sample_count % ENTROPY_TRACE_HISTORY_SIZE];
}

/**
 * @brief Get trace statistics
 * 
 * @param min_quality Pointer to store minimum quality
 * @param max_quality Pointer to store maximum quality
 * @param avg_quality Pointer to store average quality
 * @param anomaly_count Pointer to store anomaly count
 */
void entropy_trace_get_stats(double* min_quality, double* max_quality, 
                            double* avg_quality, unsigned int* anomaly_count) {
    pthread_mutex_lock(&g_trace.mutex);
    
    if (min_quality) *min_quality = g_trace.min_quality;
    if (max_quality) *max_quality = g_trace.max_quality;
    if (avg_quality) *avg_quality = g_trace.avg_quality;
    if (anomaly_count) *anomaly_count = g_trace.anomaly_count;
    
    pthread_mutex_unlock(&g_trace.mutex);
}

/**
 * @brief Reset trace statistics
 */
void entropy_trace_reset_stats(void) {
    pthread_mutex_lock(&g_trace.mutex);
    
    g_trace.min_quality = 1.0;
    g_trace.max_quality = 0.0;
    g_trace.avg_quality = 0.0;
    g_trace.anomaly_count = 0;
    g_trace.sample_count = 0;
    
    // Clear history buffer
    for (int i = 0; i < ENTROPY_TRACE_HISTORY_SIZE; i++) {
        g_trace.quality_history[i] = 0.0;
    }
    
    pthread_mutex_unlock(&g_trace.mutex);
}
