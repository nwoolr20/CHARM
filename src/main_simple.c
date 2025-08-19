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
 * @file main_simple.c
 * @brief Simplified CHARM CLI focusing on core ECE functionality
 * 
 * This simplified version focuses on the Entropy Collapse Engine (ECE)
 * for hashing and digest operations, demonstrating CHARM's core capabilities
 * while the full system is being developed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <time.h>

#include "charm.h"
#include "ece_core.h"

// Command line options
typedef struct {
    char input_file[256];
    char output_file[256];
    bool verbose;
    bool benchmark;
    bool help;
} options_t;

static options_t g_options = {0};

static void show_help(void) {
    printf("CHARM - Chaotic Hierarchical Adaptive Resilient Mechanism\n");
    printf("Entropy-native cryptographic hashing system\n\n");
    printf("Usage: charm [OPTIONS] [FILE]\n\n");
    printf("Options:\n");
    printf("  -i, --input FILE      Input file (default: stdin)\n");
    printf("  -o, --output FILE     Output file (default: stdout)\n");
    printf("  -v, --verbose         Verbose output\n");
    printf("  -b, --benchmark       Run benchmark mode\n");
    printf("  -h, --help            Show this help\n\n");
    printf("Examples:\n");
    printf("  charm -i input.txt              # Hash file and output digest\n");
    printf("  echo 'test' | charm             # Hash from stdin\n");
    printf("  charm -b                        # Run performance benchmark\n");
}

static int parse_options(int argc, char* argv[]) {
    int opt;
    static struct option long_options[] = {
        {"input",     required_argument, 0, 'i'},
        {"output",    required_argument, 0, 'o'},
        {"verbose",   no_argument,       0, 'v'},
        {"benchmark", no_argument,       0, 'b'},
        {"help",      no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "i:o:vbh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'i':
                strncpy(g_options.input_file, optarg, sizeof(g_options.input_file) - 1);
                break;
            case 'o':
                strncpy(g_options.output_file, optarg, sizeof(g_options.output_file) - 1);
                break;
            case 'v':
                g_options.verbose = true;
                break;
            case 'b':
                g_options.benchmark = true;
                break;
            case 'h':
                g_options.help = true;
                break;
            default:
                return -1;
        }
    }
    
    // Handle positional argument as input file
    if (optind < argc && strlen(g_options.input_file) == 0) {
        strncpy(g_options.input_file, argv[optind], sizeof(g_options.input_file) - 1);
    }
    
    return 0;
}

static int hash_data(const uint8_t* data, size_t size, uint8_t* digest) {
    // Initialize ECE with default configuration
    ece_config_t config = {
        .collapse_rounds = 20,
        .use_ternary_logic = true,
        .use_trampoline = true,
        .use_avalanche = true,
        .entropy_quality = 0.8,
        // Note: constant_time is now always enabled for timing attack mitigation
    };
    
    ece_handle_t handle = ece_init(&config);
    if (!handle) {
        fprintf(stderr, "Error: Failed to initialize ECE\n");
        return -1;
    }
    
    if (g_options.verbose) {
        printf("Initializing Entropy Collapse Engine...\n");
        printf("Configuration: rounds=%u, ternary=%s, trampoline=%s, avalanche=%s\n",
               config.collapse_rounds,
               config.use_ternary_logic ? "yes" : "no",
               config.use_trampoline ? "yes" : "no",
               config.use_avalanche ? "yes" : "no");
    }
    
    // Process the data
    ece_status_t status = ece_process_block(handle, data, size);
    if (status != ECE_STATUS_OK) {
        fprintf(stderr, "Error: Failed to process data (status: %d)\n", status);
        ece_shutdown(handle);
        return -1;
    }
    
    // Finalize and get digest
    status = ece_finalize(handle, digest, 32);
    if (status != ECE_STATUS_OK) {
        fprintf(stderr, "Error: Failed to finalize digest (status: %d)\n", status);
        ece_shutdown(handle);
        return -1;
    }
    
    if (g_options.verbose) {
        ece_stats_t stats;
        if (ece_get_stats(handle, &stats) == ECE_STATUS_OK) {
            printf("ECE Statistics:\n");
            printf("  Bytes processed: %lu\n", stats.bytes_processed);
            printf("  Collapses performed: %lu\n", stats.collapses_performed);
            printf("  Average entropy quality: %.3f\n", stats.avg_entropy_quality);
            printf("  Operations count: %lu\n", stats.operations_count);
        }
    }
    
    ece_shutdown(handle);
    return 0;
}

static int run_benchmark(void) {
    printf("CHARM Benchmark - Testing Entropy Collapse Engine\n");
    printf("================================================\n\n");
    
    // Test different data sizes
    size_t test_sizes[] = {64, 256, 1024, 4096, 16384, 65536};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    for (int i = 0; i < num_tests; i++) {
        size_t size = test_sizes[i];
        uint8_t* data = malloc(size);
        uint8_t digest[32];
        
        if (!data) {
            fprintf(stderr, "Error: Failed to allocate test data\n");
            continue;
        }
        
        // Fill with pseudo-random data
        for (size_t j = 0; j < size; j++) {
            data[j] = (uint8_t)(j * 17 + i * 23 + rand());
        }
        
        // Measure time
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        int result = hash_data(data, size, digest);
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        
        if (result == 0) {
            double elapsed = (end.tv_sec - start.tv_sec) + 
                           (end.tv_nsec - start.tv_nsec) / 1e9;
            double throughput = size / (elapsed * 1024 * 1024); // MB/s
            
            printf("Size: %6zu bytes | Time: %8.3f ms | Throughput: %8.2f MB/s\n",
                   size, elapsed * 1000, throughput);
            
            // Show digest for verification
            if (g_options.verbose) {
                printf("Digest: ");
                for (int k = 0; k < 32; k++) {
                    printf("%02x", digest[k]);
                }
                printf("\n\n");
            }
        } else {
            printf("Size: %6zu bytes | FAILED\n", size);
        }
        
        free(data);
    }
    
    return 0;
}

static int process_file(const char* filename) {
    FILE* fp = filename ? fopen(filename, "rb") : stdin;
    if (!fp) {
        perror("Error opening input");
        return -1;
    }
    
    // For stdin, read data incrementally
    uint8_t* data = NULL;
    size_t data_size = 0;
    size_t capacity = 0;
    
    if (filename) {
        // Read file all at once
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        if (file_size < 0 || file_size > 100 * 1024 * 1024) { // 100MB limit
            fprintf(stderr, "Error: Invalid file size\n");
            fclose(fp);
            return -1;
        }
        
        data = malloc(file_size);
        if (!data) {
            fprintf(stderr, "Error: Failed to allocate memory\n");
            fclose(fp);
            return -1;
        }
        
        data_size = fread(data, 1, file_size, fp);
        fclose(fp);
        
        if (data_size != (size_t)file_size) {
            fprintf(stderr, "Error: Failed to read complete file\n");
            free(data);
            return -1;
        }
    } else {
        // Read from stdin incrementally
        const size_t chunk_size = 4096;
        uint8_t buffer[chunk_size];
        size_t bytes_read;
        
        while ((bytes_read = fread(buffer, 1, chunk_size, fp)) > 0) {
            if (data_size + bytes_read > capacity) {
                capacity = capacity == 0 ? chunk_size : capacity * 2;
                uint8_t* new_data = realloc(data, capacity);
                if (!new_data) {
                    fprintf(stderr, "Error: Failed to allocate memory\n");
                    free(data);
                    return -1;
                }
                data = new_data;
            }
            memcpy(data + data_size, buffer, bytes_read);
            data_size += bytes_read;
        }
    }
    
    if (data_size == 0) {
        fprintf(stderr, "Error: No data to process\n");
        free(data);
        return -1;
    }
    
    // Hash the data
    uint8_t digest[32];
    int result = hash_data(data, data_size, digest);
    
    if (result == 0) {
        // Output digest
        FILE* out_fp = g_options.output_file[0] ? fopen(g_options.output_file, "w") : stdout;
        if (out_fp) {
            for (int i = 0; i < 32; i++) {
                fprintf(out_fp, "%02x", digest[i]);
            }
            fprintf(out_fp, "  %s\n", filename ? filename : "-");
            if (out_fp != stdout) fclose(out_fp);
        }
    }
    
    free(data);
    return result;
}

int main(int argc, char* argv[]) {
    // Parse command line options
    if (parse_options(argc, argv) != 0) {
        show_help();
        return 1;
    }
    
    if (g_options.help) {
        show_help();
        return 0;
    }
    
    // Seed random number generator
    srand((unsigned)time(NULL));
    
    if (g_options.benchmark) {
        return run_benchmark();
    } else {
        return process_file(g_options.input_file[0] ? g_options.input_file : NULL);
    }
}