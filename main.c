/**
 * @file main.c
 * @brief Unified CLI for the CHARM v2.0 system
 * 
 * This module implements the main command-line interface for the CHARM system,
 * providing access to all functionality including digest generation, entropy
 * tracing, system health monitoring, and debugging.
 * 
 * The CLI supports various modes of operation and provides a unified interface
 * for interacting with all components of the CHARM system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>

// CHARM system state
typedef enum {
    CHARM_STATE_INIT,      // System initializing
    CHARM_STATE_RUN,       // Normal operation
    CHARM_STATE_DEGRADED,  // Degraded entropy quality
    CHARM_STATE_FAILOVER   // Using fallback entropy sources
} charm_system_state_t;

// CLI operation modes
typedef enum {
    MODE_DIGEST,   // Generate digest of input
    MODE_TRACE,    // Monitor entropy quality
    MODE_HEALTH,   // Check system health
    MODE_DEBUG,    // Debug mode
    MODE_HELP      // Show help
} cli_mode_t;

// CLI configuration
typedef struct {
    cli_mode_t mode;
    char input_file[256];
    char output_file[256];
    bool verbose;
    bool continuous;
    int duration_seconds;
    bool use_fallback;
} cli_config_t;

// Global variables
static bool g_running = true;
static cli_config_t g_config = {
    .mode = MODE_HELP,
    .input_file = "",
    .output_file = "",
    .verbose = false,
    .continuous = false,
    .duration_seconds = 0,
    .use_fallback = false
};

// Forward declarations of external functions
extern int charmctl_init(void);
extern int charmctl_shutdown(void);
extern charm_system_state_t charmctl_get_state(void);
extern const char* charmctl_state_to_string(charm_system_state_t state);

extern int ece_digest_compute(const void* data, size_t len, uint8_t* digest);
extern void ece_digest_to_hex(const uint8_t* digest, char* hex_str);

extern int entropy_trace_init(void);
extern int entropy_trace_start(void);
extern int entropy_trace_stop(void);
extern double entropy_trace_sample(void);
extern void entropy_trace_get_stats(double* min_quality, double* max_quality, 
                                  double* avg_quality, unsigned int* anomaly_count);

extern int fallback_entropy_activate(void);
extern int fallback_entropy_deactivate(void);

extern double entropy_bus_get_quality(void);
extern uint64_t entropy_bus_get_bytes_processed(void);

/**
 * @brief Signal handler for clean shutdown
 * 
 * @param sig Signal number
 */
static void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("\nReceived signal %d, shutting down...\n", sig);
        g_running = false;
    }
}

/**
 * @brief Print help message
 */
static void print_help(void) {
    printf("CHARM v2.0 - Cryptographic Hash with Advanced Randomness Mechanisms\n\n");
    printf("Usage: charm [OPTIONS] [FILE]\n\n");
    printf("Options:\n");
    printf("  --digest, -d           Generate digest of input (default mode)\n");
    printf("  --trace, -t            Monitor entropy quality\n");
    printf("  --health, -h           Check system health\n");
    printf("  --debug                Debug mode\n");
    printf("  --output, -o FILE      Write output to FILE\n");
    printf("  --verbose, -v          Verbose output\n");
    printf("  --continuous, -c       Run continuously (for trace mode)\n");
    printf("  --duration, -s SECONDS Run for specified duration in seconds\n");
    printf("  --fallback, -f         Use fallback entropy source\n");
    printf("  --help                 Show this help message\n\n");
    printf("Examples:\n");
    printf("  charm -d file.txt                  Generate digest of file.txt\n");
    printf("  charm -t -c                        Monitor entropy quality continuously\n");
    printf("  charm -h -v                        Check system health with verbose output\n");
    printf("  charm --debug                      Run in debug mode\n");
}

/**
 * @brief Parse command-line arguments
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int 0 on success, non-zero on failure
 */
static int parse_args(int argc, char* argv[]) {
    static struct option long_options[] = {
        {"digest",     no_argument,       0, 'd'},
        {"trace",      no_argument,       0, 't'},
        {"health",     no_argument,       0, 'h'},
        {"debug",      no_argument,       0, 'D'},
        {"output",     required_argument, 0, 'o'},
        {"verbose",    no_argument,       0, 'v'},
        {"continuous", no_argument,       0, 'c'},
        {"duration",   required_argument, 0, 's'},
        {"fallback",   no_argument,       0, 'f'},
        {"help",       no_argument,       0, 'H'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "dtho:vcs:fH", long_options, &option_index)) != -1) {
        switch (c) {
            case 'd':
                g_config.mode = MODE_DIGEST;
                break;
                
            case 't':
                g_config.mode = MODE_TRACE;
                break;
                
            case 'h':
                g_config.mode = MODE_HEALTH;
                break;
                
            case 'D':
                g_config.mode = MODE_DEBUG;
                break;
                
            case 'o':
                strncpy(g_config.output_file, optarg, sizeof(g_config.output_file) - 1);
                break;
                
            case 'v':
                g_config.verbose = true;
                break;
                
            case 'c':
                g_config.continuous = true;
                break;
                
            case 's':
                g_config.duration_seconds = atoi(optarg);
                break;
                
            case 'f':
                g_config.use_fallback = true;
                break;
                
            case 'H':
                g_config.mode = MODE_HELP;
                break;
                
            case '?':
                return -1;
                
            default:
                return -1;
        }
    }
    
    // Get input file if provided
    if (optind < argc) {
        strncpy(g_config.input_file, argv[optind], sizeof(g_config.input_file) - 1);
    }
    
    return 0;
}

/**
 * @brief Run in digest mode
 * 
 * @return int 0 on success, non-zero on failure
 */
static int run_digest_mode(void) {
    FILE* input_file = NULL;
    FILE* output_file = NULL;
    uint8_t* buffer = NULL;
    size_t buffer_size = 0;
    size_t bytes_read = 0;
    uint8_t digest[32];
    char hex_digest[65];
    int result = 0;
    
    // Open input file or use stdin
    if (strlen(g_config.input_file) > 0) {
        input_file = fopen(g_config.input_file, "rb");
        if (input_file == NULL) {
            fprintf(stderr, "Error: Could not open input file '%s'\n", g_config.input_file);
            return -1;
        }
        
        // Get file size
        fseek(input_file, 0, SEEK_END);
        buffer_size = ftell(input_file);
        fseek(input_file, 0, SEEK_SET);
        
        if (g_config.verbose) {
            printf("Input file: %s (%zu bytes)\n", g_config.input_file, buffer_size);
        }
    } else {
        input_file = stdin;
        buffer_size = 4096; // Initial buffer size for stdin
        
        if (g_config.verbose) {
            printf("Reading from stdin...\n");
        }
    }
    
    // Open output file or use stdout
    if (strlen(g_config.output_file) > 0) {
        output_file = fopen(g_config.output_file, "w");
        if (output_file == NULL) {
            fprintf(stderr, "Error: Could not open output file '%s'\n", g_config.output_file);
            if (input_file != stdin) {
                fclose(input_file);
            }
            return -1;
        }
        
        if (g_config.verbose) {
            printf("Output file: %s\n", g_config.output_file);
        }
    } else {
        output_file = stdout;
    }
    
    // Allocate buffer
    buffer = (uint8_t*)malloc(buffer_size);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        result = -1;
        goto cleanup;
    }
    
    // Read input
    if (input_file == stdin) {
        // Read from stdin in chunks
        size_t chunk_size = 4096;
        size_t total_size = 0;
        size_t bytes;
        
        while ((bytes = fread(buffer + total_size, 1, chunk_size, stdin)) > 0) {
            total_size += bytes;
            
            // Resize buffer if needed
            if (total_size + chunk_size > buffer_size) {
                buffer_size += chunk_size;
                buffer = (uint8_t*)realloc(buffer, buffer_size);
                if (buffer == NULL) {
                    fprintf(stderr, "Error: Memory allocation failed\n");
                    result = -1;
                    goto cleanup;
                }
            }
        }
        
        bytes_read = total_size;
    } else {
        // Read entire file
        bytes_read = fread(buffer, 1, buffer_size, input_file);
    }
    
    if (g_config.verbose) {
        printf("Read %zu bytes\n", bytes_read);
    }
    
    // Compute digest
    if (ece_digest_compute(buffer, bytes_read, digest) != 0) {
        fprintf(stderr, "Error: Digest computation failed\n");
        result = -1;
        goto cleanup;
    }
    
    // Convert to hex
    ece_digest_to_hex(digest, hex_digest);
    
    // Output digest
    fprintf(output_file, "%s", hex_digest);
    if (output_file == stdout) {
        fprintf(output_file, "\n");
    }
    
    if (g_config.verbose) {
        printf("Digest: %s\n", hex_digest);
    }
    
cleanup:
    // Clean up
    if (buffer != NULL) {
        free(buffer);
    }
    
    if (input_file != stdin) {
        fclose(input_file);
    }
    
    if (output_file != stdout) {
        fclose(output_file);
    }
    
    return result;
}

/**
 * @brief Run in trace mode
 * 
 * @return int 0 on success, non-zero on failure
 */
static int run_trace_mode(void) {
    int result = 0;
    time_t start_time = time(NULL);
    time_t end_time = (g_config.duration_seconds > 0) ? 
                      start_time + g_config.duration_seconds : 0;
    
    // Initialize entropy trace
    if (entropy_trace_init() != 0) {
        fprintf(stderr, "Error: Failed to initialize entropy trace\n");
        return -1;
    }
    
    // Start entropy trace
    if (g_config.continuous) {
        if (entropy_trace_start() != 0) {
            fprintf(stderr, "Error: Failed to start entropy trace\n");
            return -1;
        }
        
        printf("Entropy trace started. Press Ctrl+C to stop.\n");
        
        // Run until interrupted or duration expires
        while (g_running && (end_time == 0 || time(NULL) < end_time)) {
            sleep(1);
        }
        
        // Stop entropy trace
        entropy_trace_stop();
    } else {
        // Take a single sample
        double quality = entropy_trace_sample();
        uint64_t bytes = entropy_bus_get_bytes_processed();
        charm_system_state_t state = charmctl_get_state();
        
        printf("Entropy Quality: %.4f\n", quality);
        printf("Bytes Processed: %lu\n", bytes);
        printf("System State: %s\n", charmctl_state_to_string(state));
        
        // Get statistics
        double min_quality, max_quality, avg_quality;
        unsigned int anomaly_count;
        entropy_trace_get_stats(&min_quality, &max_quality, &avg_quality, &anomaly_count);
        
        printf("Min Quality: %.4f\n", min_quality);
        printf("Max Quality: %.4f\n", max_quality);
        printf("Avg Quality: %.4f\n", avg_quality);
        printf("Anomalies: %u\n", anomaly_count);
    }
    
    return result;
}

/**
 * @brief Run in health mode
 * 
 * @return int 0 on success, non-zero on failure
 */
static int run_health_mode(void) {
    charm_system_state_t state = charmctl_get_state();
    double quality = entropy_bus_get_quality();
    uint64_t bytes = entropy_bus_get_bytes_processed();
    
    printf("CHARM System Health Check\n");
    printf("------------------------\n");
    printf("System State: %s\n", charmctl_state_to_string(state));
    printf("Entropy Quality: %.4f\n", quality);
    printf("Bytes Processed: %lu\n", bytes);
    
    // Determine overall health
    const char* health_status;
    if (state == CHARM_STATE_RUN && quality >= 0.8) {
        health_status = "EXCELLENT";
    } else if (state == CHARM_STATE_RUN && quality >= 0.6) {
        health_status = "GOOD";
    } else if (state == CHARM_STATE_DEGRADED || (state == CHARM_STATE_RUN && quality >= 0.4)) {
        health_status = "DEGRADED";
    } else if (state == CHARM_STATE_FAILOVER) {
        health_status = "FAILOVER";
    } else {
        health_status = "CRITICAL";
    }
    
    printf("Overall Health: %s\n", health_status);
    
    // Detailed information if verbose
    if (g_config.verbose) {
        // Get statistics
        double min_quality, max_quality, avg_quality;
        unsigned int anomaly_count;
        entropy_trace_get_stats(&min_quality, &max_quality, &avg_quality, &anomaly_count);
        
        printf("\nDetailed Statistics:\n");
        printf("Min Quality: %.4f\n", min_quality);
        printf("Max Quality: %.4f\n", max_quality);
        printf("Avg Quality: %.4f\n", avg_quality);
        printf("Anomalies: %u\n", anomaly_count);
        
        // TODO: Add more detailed health information
    }
    
    return 0;
}

/**
 * @brief Run in debug mode
 * 
 * @return int 0 on success, non-zero on failure
 */
static int run_debug_mode(void) {
    printf("CHARM System Debug Mode\n");
    printf("----------------------\n");
    
    // System information
    printf("System State: %s\n", charmctl_state_to_string(charmctl_get_state()));
    printf("Entropy Quality: %.4f\n", entropy_bus_get_quality());
    printf("Bytes Processed: %lu\n", entropy_bus_get_bytes_processed());
    
    // Test fallback entropy if requested
    if (g_config.use_fallback) {
        printf("\nTesting Fallback Entropy Source:\n");
        
        if (fallback_entropy_activate() != 0) {
            printf("Failed to activate fallback entropy source\n");
        } else {
            printf("Fallback entropy source activated\n");
            
            // Sample entropy with fallback
            sleep(1); // Give it time to take effect
            printf("Entropy Quality with Fallback: %.4f\n", entropy_bus_get_quality());
            
            // Deactivate fallback
            fallback_entropy_deactivate();
            printf("Fallback entropy source deactivated\n");
        }
    }
    
    // TODO: Add more debug information and tests
    
    return 0;
}

/**
 * @brief Main function
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return int Exit code
 */
int main(int argc, char* argv[]) {
    int result = 0;
    
    // Set up signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Parse command-line arguments
    if (parse_args(argc, argv) != 0) {
        print_help();
        return 1;
    }
    
    // Show help if requested
    if (g_config.mode == MODE_HELP) {
        print_help();
        return 0;
    }
    
    // Initialize CHARM system
    if (charmctl_init() != 0) {
        fprintf(stderr, "Error: Failed to initialize CHARM system\n");
        return 1;
    }
    
    // Run in selected mode
    switch (g_config.mode) {
        case MODE_DIGEST:
            result = run_digest_mode();
            break;
            
        case MODE_TRACE:
            result = run_trace_mode();
            break;
            
        case MODE_HEALTH:
            result = run_health_mode();
            break;
            
        case MODE_DEBUG:
            result = run_debug_mode(
(Content truncated due to size limit. Use line ranges to read in chunks)