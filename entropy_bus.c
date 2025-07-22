/**
 * @file entropy_bus.c
 * @brief Central shared entropy routing system
 * 
 * This module implements the central entropy bus for the CHARM system,
 * providing a shared communication channel between all subsystems for
 * entropy distribution, quality monitoring, and system state coordination.
 * 
 * The entropy bus enables real-time entropy sharing between CAEDS, CEE, ECE,
 * and other components, with proper timestamping and access logging.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>

#define ENTROPY_BUS_BUFFER_SIZE 4096
#define ENTROPY_BUS_MAX_SOURCES 16
#define ENTROPY_BUS_MAX_CONSUMERS 16

// Entropy source information
typedef struct {
    char name[32];
    double quality;
    uint64_t bytes_contributed;
    time_t last_update;
    bool active;
} entropy_source_t;

// Entropy consumer information
typedef struct {
    char name[32];
    uint64_t bytes_consumed;
    time_t last_access;
    bool active;
} entropy_consumer_t;

// Entropy bus state
typedef struct {
    uint8_t buffer[ENTROPY_BUS_BUFFER_SIZE];
    size_t buffer_used;
    uint64_t total_bytes_processed;
    double current_quality;
    
    // Sources and consumers
    entropy_source_t sources[ENTROPY_BUS_MAX_SOURCES];
    int source_count;
    entropy_consumer_t consumers[ENTROPY_BUS_MAX_CONSUMERS];
    int consumer_count;
    
    // Access log (circular buffer)
    struct {
        time_t timestamp;
        char operation[16];
        char component[32];
        size_t bytes;
    } access_log[100];
    int access_log_index;
    int access_log_count;
    
    // Mutex for thread safety
    pthread_mutex_t mutex;
    bool initialized;
} entropy_bus_t;

// Global entropy bus
static entropy_bus_t g_bus = {
    .buffer_used = 0,
    .total_bytes_processed = 0,
    .current_quality = 1.0,
    .source_count = 0,
    .consumer_count = 0,
    .access_log_index = 0,
    .access_log_count = 0,
    .initialized = false
};

/**
 * @brief Initialize the entropy bus
 * 
 * @return int 0 on success, non-zero on failure
 */
int entropy_bus_init(void) {
    if (g_bus.initialized) {
        return 0; // Already initialized
    }
    
    // Initialize mutex
    if (pthread_mutex_init(&g_bus.mutex, NULL) != 0) {
        return -1;
    }
    
    // Clear buffer
    memset(g_bus.buffer, 0, ENTROPY_BUS_BUFFER_SIZE);
    g_bus.buffer_used = 0;
    
    // Initialize sources and consumers
    for (int i = 0; i < ENTROPY_BUS_MAX_SOURCES; i++) {
        g_bus.sources[i].active = false;
    }
    g_bus.source_count = 0;
    
    for (int i = 0; i < ENTROPY_BUS_MAX_CONSUMERS; i++) {
        g_bus.consumers[i].active = false;
    }
    g_bus.consumer_count = 0;
    
    g_bus.initialized = true;
    return 0;
}

/**
 * @brief Clean up the entropy bus
 */
void entropy_bus_cleanup(void) {
    if (!g_bus.initialized) {
        return;
    }
    
    pthread_mutex_destroy(&g_bus.mutex);
    g_bus.initialized = false;
}

/**
 * @brief Log an access to the entropy bus
 * 
 * @param operation Operation type (e.g., "push", "pull")
 * @param component Component name
 * @param bytes Number of bytes involved
 */
static void log_access(const char* operation, const char* component, size_t bytes) {
    pthread_mutex_lock(&g_bus.mutex);
    
    // Add to circular buffer
    int index = g_bus.access_log_index;
    g_bus.access_log[index].timestamp = time(NULL);
    strncpy(g_bus.access_log[index].operation, operation, sizeof(g_bus.access_log[index].operation) - 1);
    strncpy(g_bus.access_log[index].component, component, sizeof(g_bus.access_log[index].component) - 1);
    g_bus.access_log[index].bytes = bytes;
    
    // Update index and count
    g_bus.access_log_index = (g_bus.access_log_index + 1) % 100;
    if (g_bus.access_log_count < 100) {
        g_bus.access_log_count++;
    }
    
    pthread_mutex_unlock(&g_bus.mutex);
}

/**
 * @brief Register an entropy source
 * 
 * @param name Source name
 * @param initial_quality Initial quality estimate (0.0-1.0)
 * @return int Source ID on success, negative on failure
 */
int entropy_bus_register_source(const char* name, double initial_quality) {
    if (!g_bus.initialized) {
        if (entropy_bus_init() != 0) {
            return -1;
        }
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    
    // Find an empty slot or existing source with the same name
    int source_id = -1;
    for (int i = 0; i < ENTROPY_BUS_MAX_SOURCES; i++) {
        if (!g_bus.sources[i].active) {
            // Empty slot
            if (source_id == -1) {
                source_id = i;
            }
        } else if (strcmp(g_bus.sources[i].name, name) == 0) {
            // Existing source with same name
            pthread_mutex_unlock(&g_bus.mutex);
            return i;
        }
    }
    
    if (source_id == -1) {
        // No empty slots
        pthread_mutex_unlock(&g_bus.mutex);
        return -1;
    }
    
    // Initialize source
    strncpy(g_bus.sources[source_id].name, name, sizeof(g_bus.sources[source_id].name) - 1);
    g_bus.sources[source_id].quality = initial_quality;
    g_bus.sources[source_id].bytes_contributed = 0;
    g_bus.sources[source_id].last_update = time(NULL);
    g_bus.sources[source_id].active = true;
    
    // Update source count
    g_bus.source_count++;
    
    pthread_mutex_unlock(&g_bus.mutex);
    
    log_access("register", name, 0);
    return source_id;
}

/**
 * @brief Register an entropy consumer
 * 
 * @param name Consumer name
 * @return int Consumer ID on success, negative on failure
 */
int entropy_bus_register_consumer(const char* name) {
    if (!g_bus.initialized) {
        if (entropy_bus_init() != 0) {
            return -1;
        }
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    
    // Find an empty slot or existing consumer with the same name
    int consumer_id = -1;
    for (int i = 0; i < ENTROPY_BUS_MAX_CONSUMERS; i++) {
        if (!g_bus.consumers[i].active) {
            // Empty slot
            if (consumer_id == -1) {
                consumer_id = i;
            }
        } else if (strcmp(g_bus.consumers[i].name, name) == 0) {
            // Existing consumer with same name
            pthread_mutex_unlock(&g_bus.mutex);
            return i;
        }
    }
    
    if (consumer_id == -1) {
        // No empty slots
        pthread_mutex_unlock(&g_bus.mutex);
        return -1;
    }
    
    // Initialize consumer
    strncpy(g_bus.consumers[consumer_id].name, name, sizeof(g_bus.consumers[consumer_id].name) - 1);
    g_bus.consumers[consumer_id].bytes_consumed = 0;
    g_bus.consumers[consumer_id].last_access = time(NULL);
    g_bus.consumers[consumer_id].active = true;
    
    // Update consumer count
    g_bus.consumer_count++;
    
    pthread_mutex_unlock(&g_bus.mutex);
    
    log_access("register", name, 0);
    return consumer_id;
}

/**
 * @brief Push entropy data to the bus
 * 
 * @param source_id Source ID
 * @param data Entropy data
 * @param size Size of data in bytes
 * @param quality Quality estimate (0.0-1.0)
 * @return int 0 on success, non-zero on failure
 */
int entropy_bus_push(int source_id, const uint8_t* data, size_t size, double quality) {
    if (!g_bus.initialized) {
        return -1;
    }
    
    if (source_id < 0 || source_id >= ENTROPY_BUS_MAX_SOURCES || !g_bus.sources[source_id].active) {
        return -1;
    }
    
    if (data == NULL || size == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    
    // Update source information
    g_bus.sources[source_id].quality = quality;
    g_bus.sources[source_id].bytes_contributed += size;
    g_bus.sources[source_id].last_update = time(NULL);
    
    // Calculate new overall quality (weighted average)
    double total_quality = 0.0;
    uint64_t total_bytes = 0;
    
    for (int i = 0; i < ENTROPY_BUS_MAX_SOURCES; i++) {
        if (g_bus.sources[i].active) {
            total_quality += g_bus.sources[i].quality * g_bus.sources[i].bytes_contributed;
            total_bytes += g_bus.sources[i].bytes_contributed;
        }
    }
    
    if (total_bytes > 0) {
        g_bus.current_quality = total_quality / total_bytes;
    }
    
    // Add data to buffer (circular)
    for (size_t i = 0; i < size; i++) {
        g_bus.buffer[g_bus.buffer_used] = data[i];
        g_bus.buffer_used = (g_bus.buffer_used + 1) % ENTROPY_BUS_BUFFER_SIZE;
    }
    
    // Update total bytes processed
    g_bus.total_bytes_processed += size;
    
    pthread_mutex_unlock(&g_bus.mutex);
    
    log_access("push", g_bus.sources[source_id].name, size);
    return 0;
}

/**
 * @brief Pull entropy data from the bus
 * 
 * @param consumer_id Consumer ID
 * @param data Buffer to store entropy data
 * @param size Size of data to pull in bytes
 * @return int Number of bytes pulled, negative on failure
 */
int entropy_bus_pull(int consumer_id, uint8_t* data, size_t size) {
    if (!g_bus.initialized) {
        return -1;
    }
    
    if (consumer_id < 0 || consumer_id >= ENTROPY_BUS_MAX_CONSUMERS || !g_bus.consumers[consumer_id].active) {
        return -1;
    }
    
    if (data == NULL || size == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    
    // Determine how much data we can pull
    size_t available = (g_bus.total_bytes_processed > 0) ? ENTROPY_BUS_BUFFER_SIZE : g_bus.buffer_used;
    size_t to_pull = (size < available) ? size : available;
    
    if (to_pull == 0) {
        pthread_mutex_unlock(&g_bus.mutex);
        return 0;
    }
    
    // Pull data from buffer
    // For simplicity, we'll just copy the most recent data
    size_t start_pos = (g_bus.buffer_used + ENTROPY_BUS_BUFFER_SIZE - to_pull) % ENTROPY_BUS_BUFFER_SIZE;
    
    for (size_t i = 0; i < to_pull; i++) {
        data[i] = g_bus.buffer[(start_pos + i) % ENTROPY_BUS_BUFFER_SIZE];
    }
    
    // Update consumer information
    g_bus.consumers[consumer_id].bytes_consumed += to_pull;
    g_bus.consumers[consumer_id].last_access = time(NULL);
    
    pthread_mutex_unlock(&g_bus.mutex);
    
    log_access("pull", g_bus.consumers[consumer_id].name, to_pull);
    return to_pull;
}

/**
 * @brief Get the current entropy quality
 * 
 * @return double Quality value between 0.0 (poor) and 1.0 (excellent)
 */
double entropy_bus_get_quality(void) {
    if (!g_bus.initialized) {
        return 0.0;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    double quality = g_bus.current_quality;
    pthread_mutex_unlock(&g_bus.mutex);
    
    return quality;
}

/**
 * @brief Get the total number of bytes processed
 * 
 * @return uint64_t Total bytes processed
 */
uint64_t entropy_bus_get_bytes_processed(void) {
    if (!g_bus.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    uint64_t bytes = g_bus.total_bytes_processed;
    pthread_mutex_unlock(&g_bus.mutex);
    
    return bytes;
}

/**
 * @brief Get the number of active entropy sources
 * 
 * @return int Number of active sources
 */
int entropy_bus_get_source_count(void) {
    if (!g_bus.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    int count = g_bus.source_count;
    pthread_mutex_unlock(&g_bus.mutex);
    
    return count;
}

/**
 * @brief Get the number of active entropy consumers
 * 
 * @return int Number of active consumers
 */
int entropy_bus_get_consumer_count(void) {
    if (!g_bus.initialized) {
        return 0;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    int count = g_bus.consumer_count;
    pthread_mutex_unlock(&g_bus.mutex);
    
    return count;
}

/**
 * @brief Get information about an entropy source
 * 
 * @param source_id Source ID
 * @param name Buffer to store source name
 * @param name_size Size of name buffer
 * @param quality Pointer to store quality
 * @param bytes_contributed Pointer to store bytes contributed
 * @param last_update Pointer to store last update time
 * @return int 0 on success, non-zero on failure
 */
int entropy_bus_get_source_info(int source_id, char* name, size_t name_size,
                               double* quality, uint64_t* bytes_contributed,
                               time_t* last_update) {
    if (!g_bus.initialized) {
        return -1;
    }
    
    if (source_id < 0 || source_id >= ENTROPY_BUS_MAX_SOURCES || !g_bus.sources[source_id].active) {
        return -1;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    
    if (name != NULL && name_size > 0) {
        strncpy(name, g_bus.sources[source_id].name, name_size - 1);
        name[name_size - 1] = '\0';
    }
    
    if (quality != NULL) {
        *quality = g_bus.sources[source_id].quality;
    }
    
    if (bytes_contributed != NULL) {
        *bytes_contributed = g_bus.sources[source_id].bytes_contributed;
    }
    
    if (last_update != NULL) {
        *last_update = g_bus.sources[source_id].last_update;
    }
    
    pthread_mutex_unlock(&g_bus.mutex);
    
    return 0;
}

/**
 * @brief Update the quality estimate for a source
 * 
 * @param source_id Source ID
 * @param quality New quality estimate (0.0-1.0)
 * @return int 0 on success, non-zero on failure
 */
int entropy_bus_update_quality(int source_id, double quality) {
    if (!g_bus.initialized) {
        return -1;
    }
    
    if (source_id < 0 || source_id >= ENTROPY_BUS_MAX_SOURCES || !g_bus.sources[source_id].active) {
        return -1;
    }
    
    pthread_mutex_lock(&g_bus.mutex);
    
    // Update source quality
    g_bus.sources[source_id].quality = quality;
    g_bus.sources[source_id].last_update = time(NULL);
    
    // Recalculate overall quality
    double total_quality = 0.0;
    uint64_t total_bytes = 0;
    
    for (int i = 0; i < ENTROPY_BUS_MAX_SOURCES; i++) {
        if (g_bus.sources[i].active) {
            total_quality += g_bus.sources[i].quality * g_bus.sources[i].bytes_contributed;
            total_bytes += g_bus.sources[i].bytes_contributed;
        }
    }
    
    if (total_bytes > 0) {
        g_bus.current_quality = total_quality / total_bytes;
    }
    
    pthread_mutex_unlock(&g_bus.mutex);
    
    log_access("quality", g_bus.sources[source_id].name, 0);
    return 0;
}
