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
 * @file charm_sidechannel.cpp
 * @brief CHARM Algorithm Side-Channel Analysis
 * 
 * Tests for timing attack resistance - verify constant-time operations
 * and analyze timing variance patterns.
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>
#include <cmath>

extern "C" {
#include "charm.h"
}

// High-precision timing measurement
struct TimingMeasurement {
    uint64_t duration_ns;
    size_t input_length;
    uint8_t input_pattern;  // 0=zeros, 1=ones, 2=random, 3=alternating
};

// Statistical analysis of timing measurements
struct TimingStats {
    uint64_t min_time_ns;
    uint64_t max_time_ns;
    double mean_time_ns;
    double std_dev_ns;
    double variance_ns;
    double coefficient_variation;
    size_t num_measurements;
    size_t outlier_count;  // Measurements > 3 std devs from mean
};

TimingStats calculate_timing_stats(const std::vector<TimingMeasurement>& measurements) {
    TimingStats stats;
    stats.num_measurements = measurements.size();
    
    if (measurements.empty()) {
        return stats;
    }
    
    // Extract durations
    std::vector<uint64_t> durations;
    for (const auto& m : measurements) {
        durations.push_back(m.duration_ns);
    }
    
    stats.min_time_ns = *std::min_element(durations.begin(), durations.end());
    stats.max_time_ns = *std::max_element(durations.begin(), durations.end());
    
    // Calculate mean
    uint64_t sum = 0;
    for (uint64_t duration : durations) {
        sum += duration;
    }
    stats.mean_time_ns = static_cast<double>(sum) / durations.size();
    
    // Calculate variance and standard deviation
    double variance_sum = 0.0;
    for (uint64_t duration : durations) {
        double diff = static_cast<double>(duration) - stats.mean_time_ns;
        variance_sum += diff * diff;
    }
    stats.variance_ns = variance_sum / durations.size();
    stats.std_dev_ns = std::sqrt(stats.variance_ns);
    
    // Coefficient of variation (relative standard deviation)
    stats.coefficient_variation = stats.std_dev_ns / stats.mean_time_ns;
    
    // Count outliers (more than 3 standard deviations from mean)
    stats.outlier_count = 0;
    for (uint64_t duration : durations) {
        double diff = std::abs(static_cast<double>(duration) - stats.mean_time_ns);
        if (diff > 3.0 * stats.std_dev_ns) {
            stats.outlier_count++;
        }
    }
    
    return stats;
}

// Generate test input with specified pattern
std::vector<uint8_t> generate_timing_test_input(size_t length, uint8_t pattern) {
    std::vector<uint8_t> input(length);
    
    switch (pattern) {
        case 0: // All zeros
            std::fill(input.begin(), input.end(), 0x00);
            break;
        case 1: // All ones
            std::fill(input.begin(), input.end(), 0xFF);
            break;
        case 2: // Random
            {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<uint8_t> dis(0, 255);
                for (size_t i = 0; i < length; i++) {
                    input[i] = dis(gen);
                }
            }
            break;
        case 3: // Alternating
            for (size_t i = 0; i < length; i++) {
                input[i] = (i % 2) ? 0xFF : 0x00;
            }
            break;
        default:
            std::fill(input.begin(), input.end(), 0x55);
            break;
    }
    
    return input;
}

// Measure timing for a single hash operation with high precision
uint64_t measure_hash_timing(charm_variant_t variant, const std::vector<uint8_t>& input) {
    size_t output_len = (variant == CHARM_256) ? 32 : 
                       (variant == CHARM_384) ? 48 : 64;
    std::vector<uint8_t> output(output_len);
    
    // Warm up CPU and caches
    charm_hash(variant, input.data(), input.size(), output.data());
    
    // Actual measurement
    auto start = std::chrono::high_resolution_clock::now();
    charm_hash(variant, input.data(), input.size(), output.data());
    auto end = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

// Test timing consistency for side-channel resistance
void test_timing_consistency(charm_variant_t variant, size_t input_length, 
                           size_t iterations, std::vector<TimingMeasurement>& measurements) {
    
    for (size_t i = 0; i < iterations; i++) {
        for (uint8_t pattern = 0; pattern < 4; pattern++) {
            auto input = generate_timing_test_input(input_length, pattern);
            uint64_t timing = measure_hash_timing(variant, input);
            
            measurements.push_back({timing, input_length, pattern});
        }
    }
}

// Run side-channel analysis tests
int run_sidechannel_tests() {
    std::vector<charm_variant_t> variants = {CHARM_256, CHARM_384, CHARM_512};
    std::vector<size_t> test_lengths = {0, 16, 64, 256, 1024, 4096};
    size_t iterations_per_test = 100;  // Number of measurements per test
    
    std::cout << "{\n";
    std::cout << "  \"timestamp\": \"" << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() << "\",\n";
    std::cout << "  \"test_type\": \"Side_Channel_Analysis\",\n";
    std::cout << "  \"algorithm\": \"CHARM\",\n";
    std::cout << "  \"version\": \"1.0\",\n";
    std::cout << "  \"timing_analysis\": [\n";
    
    bool first_result = true;
    int total_tests = 0;
    int passed_tests = 0;
    
    for (auto variant : variants) {
        for (auto length : test_lengths) {
            total_tests++;
            
            std::vector<TimingMeasurement> measurements;
            test_timing_consistency(variant, length, iterations_per_test, measurements);
            
            TimingStats stats = calculate_timing_stats(measurements);
            
            // Test passes if coefficient of variation < 0.1 (10%) and outliers < 5%
            double outlier_ratio = static_cast<double>(stats.outlier_count) / stats.num_measurements;
            bool test_passed = (stats.coefficient_variation < 0.1) && (outlier_ratio < 0.05);
            
            if (test_passed) {
                passed_tests++;
            }
            
            if (!first_result) {
                std::cout << ",\n";
            }
            first_result = false;
            
            std::cout << "    {\n";
            std::cout << "      \"variant\": \"CHARM-" << variant << "\",\n";
            std::cout << "      \"input_length\": " << length << ",\n";
            std::cout << "      \"measurements_count\": " << stats.num_measurements << ",\n";
            std::cout << "      \"min_time_ns\": " << stats.min_time_ns << ",\n";
            std::cout << "      \"max_time_ns\": " << stats.max_time_ns << ",\n";
            std::cout << "      \"mean_time_ns\": " << std::fixed << std::setprecision(2) 
                      << stats.mean_time_ns << ",\n";
            std::cout << "      \"std_deviation_ns\": " << std::fixed << std::setprecision(2) 
                      << stats.std_dev_ns << ",\n";
            std::cout << "      \"coefficient_variation\": " << std::fixed << std::setprecision(4) 
                      << stats.coefficient_variation << ",\n";
            std::cout << "      \"outlier_count\": " << stats.outlier_count << ",\n";
            std::cout << "      \"outlier_ratio\": " << std::fixed << std::setprecision(4) 
                      << outlier_ratio << ",\n";
            std::cout << "      \"timing_range_ns\": " << (stats.max_time_ns - stats.min_time_ns) << ",\n";
            std::cout << "      \"status\": \"" << (test_passed ? "PASS" : "FAIL") << "\",\n";
            std::cout << "      \"security_notes\": {\n";
            std::cout << "        \"constant_time_requirement\": \"CV < 0.1 and outliers < 5%\",\n";
            std::cout << "        \"timing_variance_acceptable\": " << (stats.coefficient_variation < 0.1 ? "true" : "false") << ",\n";
            std::cout << "        \"outlier_level_acceptable\": " << (outlier_ratio < 0.05 ? "true" : "false") << "\n";
            std::cout << "      }\n";
            std::cout << "    }";
        }
    }
    
    std::cout << "\n  ],\n";
    std::cout << "  \"summary\": {\n";
    std::cout << "    \"passed\": " << passed_tests << ",\n";
    std::cout << "    \"total\": " << total_tests << ",\n";
    std::cout << "    \"status\": \"" << (passed_tests == total_tests ? "PASS" : "FAIL") << "\",\n";
    std::cout << "    \"side_channel_resistance\": {\n";
    std::cout << "      \"timing_variance_criteria\": \"Coefficient of variation < 10%\",\n";
    std::cout << "      \"outlier_criteria\": \"< 5% of measurements beyond 3 standard deviations\",\n";
    std::cout << "      \"security_implications\": \"Low timing variance indicates resistance to timing attacks\"\n";
    std::cout << "    }\n";
    std::cout << "  }\n";
    std::cout << "}\n";
    
    return (passed_tests == total_tests) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    return run_sidechannel_tests();
}