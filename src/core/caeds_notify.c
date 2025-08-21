/**
 * @file caeds_notify.c
 * @brief Implementation of the CAEDS notification system
 * 
 * This file implements the notification system for the CAEDS (Charm Advanced
 * Entropy Diagnostic System) subsystem, providing mechanisms to broadcast
 * entropy anomalies and status updates to various output channels.
 */

#include "caeds_notify.h"
#include "charm_status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define _GNU_SOURCE
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

/**
 * @brief Maximum message queue size
 */
#define CAEDS_NOTIFY_MAX_QUEUE_SIZE 1024

/**
 * @brief Maximum message length
 */
#define CAEDS_NOTIFY_MAX_MESSAGE_LENGTH 1024

/**
 * @brief Internal notification queue entry
 */
typedef struct caeds_notify_queue_entry {
    caeds_notify_message_t message;
    char message_text[CAEDS_NOTIFY_MAX_MESSAGE_LENGTH];
    char source_text[256];
    struct caeds_notify_queue_entry* next;
} caeds_notify_queue_entry_t;

/**
 * @brief Internal notification queue
 */
typedef struct {
    caeds_notify_queue_entry_t* head;
    caeds_notify_queue_entry_t* tail;
    size_t size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int running;
    pthread_t thread;
} caeds_notify_queue_t;

/**
 * @brief Extended notification context
 */
typedef struct {
    caeds_notify_context_t public;
    caeds_notify_queue_t queue;
    FILE* log_file_handle;
} caeds_notify_context_internal_t;

/**
 * @brief Get current timestamp in microseconds
 * 
 * @return uint64_t Timestamp
 */
static uint64_t caeds_notify_get_timestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + (uint64_t)tv.tv_usec;
}

/**
 * @brief Format timestamp for output
 * 
 * @param timestamp Timestamp in microseconds
 * @param buffer Output buffer
 * @param size Buffer size
 */
static void caeds_notify_format_timestamp(uint64_t timestamp, char* buffer, size_t size) {
    time_t seconds = timestamp / 1000000;
    int microseconds = timestamp % 1000000;
    struct tm* tm_info = localtime(&seconds);
    
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
    char micro_buffer[16];
    snprintf(micro_buffer, sizeof(micro_buffer), ".%06d", microseconds);
    strncat(buffer, micro_buffer, size - strlen(buffer) - 1);
}

/**
 * @brief Process notification queue
 * 
 * @param context Notification context
 */
static void caeds_notify_process_queue(caeds_notify_context_internal_t* context) {
    caeds_notify_queue_entry_t* entry;
    
    pthread_mutex_lock(&context->queue.mutex);
    entry = context->queue.head;
    if (entry) {
        context->queue.head = entry->next;
        if (context->queue.head == NULL) {
            context->queue.tail = NULL;
        }
        context->queue.size--;
    }
    pthread_mutex_unlock(&context->queue.mutex);
    
    if (entry) {
        // Process entry
        if (entry->message.severity >= context->public.min_severity) {
            // CLI output
            if (context->public.channels & CAEDS_NOTIFY_CHANNEL_CLI) {
                char timestamp_buffer[32];
                caeds_notify_format_timestamp(entry->message.timestamp, timestamp_buffer, sizeof(timestamp_buffer));
                
                const char* severity_str = caeds_notify_severity_to_string(entry->message.severity);
                fprintf(stderr, "[%s] %s [%s] %s\n", 
                        timestamp_buffer, severity_str, entry->message.source, entry->message.message);
            }
            
            // Log file output
            if ((context->public.channels & CAEDS_NOTIFY_CHANNEL_LOG) && context->log_file_handle) {
                char timestamp_buffer[32];
                caeds_notify_format_timestamp(entry->message.timestamp, timestamp_buffer, sizeof(timestamp_buffer));
                
                const char* severity_str = caeds_notify_severity_to_string(entry->message.severity);
                fprintf(context->log_file_handle, "[%s] %s [%s] %s\n", 
                        timestamp_buffer, severity_str, entry->message.source, entry->message.message);
                fflush(context->log_file_handle);
            }
            
            // Syslog output
            if (context->public.channels & CAEDS_NOTIFY_CHANNEL_SYSLOG) {
                // Syslog implementation would go here
                // For now, we'll just print to stderr as a placeholder
                fprintf(stderr, "SYSLOG: [%s] %s\n", entry->message.source, entry->message.message);
            }
            
            // Callback output
            if ((context->public.channels & CAEDS_NOTIFY_CHANNEL_CALLBACK) && context->public.callback) {
                context->public.callback(&entry->message, context->public.callback_data);
            }
        }
        
        free(entry);
    }
}

/**
 * @brief Notification queue processing thread
 * 
 * @param arg Notification context
 * @return void* NULL
 */
static void* caeds_notify_thread(void* arg) {
    caeds_notify_context_internal_t* context = (caeds_notify_context_internal_t*)arg;
    
    pthread_mutex_lock(&context->queue.mutex);
    while (context->queue.running) {
        if (context->queue.head) {
            pthread_mutex_unlock(&context->queue.mutex);
            caeds_notify_process_queue(context);
            pthread_mutex_lock(&context->queue.mutex);
        } else {
            pthread_cond_wait(&context->queue.cond, &context->queue.mutex);
        }
    }
    pthread_mutex_unlock(&context->queue.mutex);
    
    return NULL;
}

charm_status_t caeds_notify_init(caeds_notify_context_t** context, const caeds_notify_context_t* config) {
    if (!context) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)malloc(sizeof(caeds_notify_context_internal_t));
    if (!internal_context) {
        return CHARM_STATUS_ERROR_MEMORY;
    }
    
    memset(internal_context, 0, sizeof(caeds_notify_context_internal_t));
    
    if (config) {
        internal_context->public.channels = config->channels;
        internal_context->public.min_severity = config->min_severity;
        internal_context->public.callback = config->callback;
        internal_context->public.callback_data = config->callback_data;
        internal_context->public.flags = config->flags;
        
        if (config->log_file && (config->channels & CAEDS_NOTIFY_CHANNEL_LOG)) {
            internal_context->public.log_file = strdup(config->log_file);
            internal_context->log_file_handle = fopen(config->log_file, "a");
            if (!internal_context->log_file_handle) {
                free(internal_context);
                return CHARM_STATUS_ERROR_IO;
            }
        }
    } else {
        // Default configuration
        internal_context->public.channels = CAEDS_NOTIFY_CHANNEL_CLI;
        internal_context->public.min_severity = CAEDS_NOTIFY_INFO;
    }
    
    // Initialize queue
    internal_context->queue.head = NULL;
    internal_context->queue.tail = NULL;
    internal_context->queue.size = 0;
    pthread_mutex_init(&internal_context->queue.mutex, NULL);
    pthread_cond_init(&internal_context->queue.cond, NULL);
    internal_context->queue.running = 1;
    
    // Start processing thread
    if (pthread_create(&internal_context->queue.thread, NULL, caeds_notify_thread, internal_context) != 0) {
        if (internal_context->log_file_handle) {
            fclose(internal_context->log_file_handle);
        }
        if (internal_context->public.log_file) {
            free((void*)internal_context->public.log_file);
        }
        pthread_mutex_destroy(&internal_context->queue.mutex);
        pthread_cond_destroy(&internal_context->queue.cond);
        free(internal_context);
        return CHARM_STATUS_ERROR_GENERAL;
    }
    
    *context = (caeds_notify_context_t*)internal_context;
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_send(caeds_notify_context_t* context, 
                               caeds_notify_severity_t severity,
                               const char* source,
                               uint32_t code,
                               const char* format, ...) {
    if (!context || !source || !format) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    // Check if severity is below minimum
    if (severity < internal_context->public.min_severity) {
        return CHARM_STATUS_SUCCESS;
    }
    
    // Check if queue is full
    pthread_mutex_lock(&internal_context->queue.mutex);
    if (internal_context->queue.size >= CAEDS_NOTIFY_MAX_QUEUE_SIZE) {
        pthread_mutex_unlock(&internal_context->queue.mutex);
        return CHARM_STATUS_ERROR_OVERFLOW;
    }
    pthread_mutex_unlock(&internal_context->queue.mutex);
    
    // Create queue entry
    caeds_notify_queue_entry_t* entry = (caeds_notify_queue_entry_t*)malloc(sizeof(caeds_notify_queue_entry_t));
    if (!entry) {
        return CHARM_STATUS_ERROR_MEMORY;
    }
    
    memset(entry, 0, sizeof(caeds_notify_queue_entry_t));
    
    // Format message
    va_list args;
    va_start(args, format);
    vsnprintf(entry->message_text, CAEDS_NOTIFY_MAX_MESSAGE_LENGTH, format, args);
    va_end(args);
    
    // Copy source
    strncpy(entry->source_text, source, sizeof(entry->source_text) - 1);
    
    // Fill message
    entry->message.severity = severity;
    entry->message.timestamp = caeds_notify_get_timestamp();
    entry->message.source = entry->source_text;
    entry->message.message = entry->message_text;
    entry->message.code = code;
    entry->message.context = NULL;
    
    // Add to queue
    pthread_mutex_lock(&internal_context->queue.mutex);
    entry->next = NULL;
    if (internal_context->queue.tail) {
        internal_context->queue.tail->next = entry;
    } else {
        internal_context->queue.head = entry;
    }
    internal_context->queue.tail = entry;
    internal_context->queue.size++;
    pthread_cond_signal(&internal_context->queue.cond);
    pthread_mutex_unlock(&internal_context->queue.mutex);
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_send_with_context(caeds_notify_context_t* context, 
                                            caeds_notify_severity_t severity,
                                            const char* source,
                                            uint32_t code,
                                            void* user_context,
                                            const char* format, ...) {
    if (!context || !source || !format) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    // Check if severity is below minimum
    if (severity < internal_context->public.min_severity) {
        return CHARM_STATUS_SUCCESS;
    }
    
    // Check if queue is full
    pthread_mutex_lock(&internal_context->queue.mutex);
    if (internal_context->queue.size >= CAEDS_NOTIFY_MAX_QUEUE_SIZE) {
        pthread_mutex_unlock(&internal_context->queue.mutex);
        return CHARM_STATUS_ERROR_OVERFLOW;
    }
    pthread_mutex_unlock(&internal_context->queue.mutex);
    
    // Create queue entry
    caeds_notify_queue_entry_t* entry = (caeds_notify_queue_entry_t*)malloc(sizeof(caeds_notify_queue_entry_t));
    if (!entry) {
        return CHARM_STATUS_ERROR_MEMORY;
    }
    
    memset(entry, 0, sizeof(caeds_notify_queue_entry_t));
    
    // Format message
    va_list args;
    va_start(args, format);
    vsnprintf(entry->message_text, CAEDS_NOTIFY_MAX_MESSAGE_LENGTH, format, args);
    va_end(args);
    
    // Copy source
    strncpy(entry->source_text, source, sizeof(entry->source_text) - 1);
    
    // Fill message
    entry->message.severity = severity;
    entry->message.timestamp = caeds_notify_get_timestamp();
    entry->message.source = entry->source_text;
    entry->message.message = entry->message_text;
    entry->message.code = code;
    entry->message.context = user_context;
    
    // Add to queue
    pthread_mutex_lock(&internal_context->queue.mutex);
    entry->next = NULL;
    if (internal_context->queue.tail) {
        internal_context->queue.tail->next = entry;
    } else {
        internal_context->queue.head = entry;
    }
    internal_context->queue.tail = entry;
    internal_context->queue.size++;
    pthread_cond_signal(&internal_context->queue.cond);
    pthread_mutex_unlock(&internal_context->queue.mutex);
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_register_callback(caeds_notify_context_t* context,
                                            caeds_notify_callback_t callback,
                                            void* user_data) {
    if (!context || !callback) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    internal_context->public.callback = callback;
    internal_context->public.callback_data = user_data;
    internal_context->public.channels |= CAEDS_NOTIFY_CHANNEL_CALLBACK;
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_set_min_severity(caeds_notify_context_t* context,
                                           caeds_notify_severity_t min_severity) {
    if (!context) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    internal_context->public.min_severity = min_severity;
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_set_channels(caeds_notify_context_t* context,
                                       caeds_notify_channel_t channels) {
    if (!context) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    internal_context->public.channels = channels;
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_set_log_file(caeds_notify_context_t* context,
                                       const char* log_file) {
    if (!context || !log_file) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    // Close existing log file
    if (internal_context->log_file_handle) {
        fclose(internal_context->log_file_handle);
        internal_context->log_file_handle = NULL;
    }
    
    // Free existing log file path
    if (internal_context->public.log_file) {
        free((void*)internal_context->public.log_file);
        internal_context->public.log_file = NULL;
    }
    
    // Set new log file
    internal_context->public.log_file = strdup(log_file);
    if (!internal_context->public.log_file) {
        return CHARM_STATUS_ERROR_MEMORY;
    }
    
    // Open new log file
    internal_context->log_file_handle = fopen(log_file, "a");
    if (!internal_context->log_file_handle) {
        free((void*)internal_context->public.log_file);
        internal_context->public.log_file = NULL;
        return CHARM_STATUS_ERROR_IO;
    }
    
    // Enable log channel
    internal_context->public.channels |= CAEDS_NOTIFY_CHANNEL_LOG;
    
    return CHARM_STATUS_SUCCESS;
}

charm_status_t caeds_notify_shutdown(caeds_notify_context_t* context) {
    if (!context) {
        return CHARM_STATUS_ERROR_INVALID_PARAM;
    }
    
    caeds_notify_context_internal_t* internal_context = (caeds_notify_context_internal_t*)context;
    
    // Stop processing thread
    pthread_mutex_lock(&internal_context->queue.mutex);
    internal_context->queue.running = 0;
    pthread_cond_signal(&internal_context->queue.cond);
    pthread_mutex_unlock(&internal_context->queue.mutex);
    
    // Wait for processing thread to finish
    if (internal_context->queue.thread) {
        pthread_join(internal_context->queue.thread, NULL);
    }
    
    // Free any remaining queue entries
    while (internal_context->queue.head) {
        caeds_notify_queue_entry_t* entry = internal_context->queue.head;
        internal_context->queue.head = entry->next;
        free(entry);
    }
    
    // Clean up
    pthread_mutex_destroy(&internal_context->queue.mutex);
    pthread_cond_destroy(&internal_context->queue.cond);
    
    if (internal_context->public.log_file) {
        free((void*)internal_context->public.log_file);
    }
    
    free(internal_context);
    
    return CHARM_STATUS_SUCCESS;
}