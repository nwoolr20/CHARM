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
 * @file charm_avalanche.cpp
 * @brief CHARM Algorithm Avalanche Effect Testing
 * 
 * Tests for cryptographic avalanche effect - single bit changes should cause
 * significant changes in output hash values.
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <random>
#include <bitset>
#include <cmath>
#include <algorithm>

extern "C" {
#include "charm.h"
}

// Calculate Hamming distance between two byte arrays
size_t hamming_distance(const uint8_t* a, const uint8_t* b, size_t len) {
    size_t distance = 0;
    for (size_t i = 0; i < len; i++) {
        uint8_t xor_result = a[i] ^ b[i];
        distance += __builtin_popcount(xor_result);
    }
    return distance;
}

// Calculate avalanche statistics
struct AvalancheStats {
    double min_ratio;
    double max_ratio;
    double mean_ratio;
    double std_dev;
    size_t total_tests;
    size_t passed_tests; // Tests with ratio between 0.4 and 0.6
};

AvalancheStats calculate_avalanche_stats(const std::vector<double>& ratios) {
    AvalancheStats stats;
    stats.total_tests = ratios.size();
    stats.passed_tests = 0;
    
    if (ratios.empty()) {
        stats.min_ratio = 0.0;
        stats.max_ratio = 0.0;
        stats.mean_ratio = 0.0;
        stats.std_dev = 0.0;
        return stats;
    }
    
    stats.min_ratio = *std::min_element(ratios.begin(), ratios.end());
    stats.max_ratio = *std::max_element(ratios.begin(), ratios.end());
    
    double sum = 0.0;
    for (double ratio : ratios) {
        sum += ratio;
        // Good avalanche effect should be between 40% and 60%
        if (ratio >= 0.4 && ratio <= 0.6) {
            stats.passed_tests++;
        }
    }
    stats.mean_ratio = sum / ratios.size();
    
    // Calculate standard deviation
    double variance = 0.0;
    for (double ratio : ratios) {
        variance += (ratio - stats.mean_ratio) * (ratio - stats.mean_ratio);
    }
    stats.std_dev = std::sqrt(variance / ratios.size());
    
    return stats;
}

// Test avalanche effect for a specific input
void test_avalanche_for_input(const std::vector<uint8_t>& input, 
                             charm_variant_t variant,
                             std::vector<double>& bit_flip_ratios) {
    
    size_t output_len = (variant == CHARM_256) ? 32 : 
                       (variant == CHARM_384) ? 48 : 64;
    size_t output_bits = output_len * 8;
    
    // Compute original hash
    std::vector<uint8_t> original_hash(output_len);
    charm_hash(variant, input.data(), input.size(), original_hash.data());
    
    // Test flipping each bit in the input
    for (size_t byte_idx = 0; byte_idx < input.size(); byte_idx++) {
        for (int bit_idx = 0; bit_idx < 8; bit_idx++) {
            // Create modified input with one bit flipped
            std::vector<uint8_t> modified_input = input;
            modified_input[byte_idx] ^= (1 << bit_idx);
            
            // Compute hash of modified input
            std::vector<uint8_t> modified_hash(output_len);
            charm_hash(variant, modified_input.data(), modified_input.size(), modified_hash.data());
            
            // Calculate hamming distance
            size_t hamming_dist = hamming_distance(original_hash.data(), modified_hash.data(), output_len);
            double flip_ratio = static_cast<double>(hamming_dist) / output_bits;
            
            bit_flip_ratios.push_back(flip_ratio);
        }
    }
}

// Run avalanche effect tests
int run_avalanche_tests() {
    std::vector<charm_variant_t> variants = {CHARM_256, CHARM_384, CHARM_512};
    
    // Test inputs of various lengths
    std::vector<size_t> test_lengths = {8, 16, 32, 64, 128, 256, 512};
    
    std::cout << "{\n";
    std::cout << "  \"timestamp\": \"" << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() << "\",\n";
    std::cout << "  \"test_type\": \"Avalanche_Effect\",\n";
    std::cout << "  \"algorithm\": \"CHARM\",\n";
    std::cout << "  \"version\": \"1.0\",\n";
    std::cout << "  \"avalanche_results\": [\n";
    
    bool first_result = true;
    int total_tests = 0;
    int passed_tests = 0;
    
    for (auto variant : variants) {
        for (auto length : test_lengths) {
            total_tests++;
            
            // Generate test input with pattern
            std::vector<uint8_t> test_input(length);
            std::random_device rd;
            std::mt19937 gen(42); // Fixed seed for reproducibility
            std::uniform_int_distribution<uint8_t> dis(0, 255);
            for (size_t i = 0; i < length; i++) {
                test_input[i] = dis(gen);
            }
            
            std::vector<double> bit_flip_ratios;
            
            auto start_time = std::chrono::high_resolution_clock::now();
            test_avalanche_for_input(test_input, variant, bit_flip_ratios);
            auto end_time = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            
            AvalancheStats stats = calculate_avalanche_stats(bit_flip_ratios);
            
            if (!first_result) {
                std::cout << ",\n";
            }
            first_result = false;
            
            // Test passes if >80% of bit flips result in good avalanche (40-60% bits changed)
            bool test_passed = (static_cast<double>(stats.passed_tests) / stats.total_tests) >= 0.8;
            if (test_passed) {
                passed_tests++;
            }
            
            std::cout << "    {\n";
            std::cout << "      \"variant\": \"CHARM-" << variant << "\",\n";
            std::cout << "      \"input_length\": " << length << ",\n";
            std::cout << "      \"input_bits_tested\": " << (length * 8) << ",\n";
            std::cout << "      \"total_bit_flips\": " << stats.total_tests << ",\n";
            std::cout << "      \"good_avalanche_count\": " << stats.passed_tests << ",\n";
            std::cout << "      \"avalanche_ratio\": " << std::fixed << std::setprecision(4) 
                      << (static_cast<double>(stats.passed_tests) / stats.total_tests) << ",\n";
            std::cout << "      \"min_bit_change_ratio\": " << std::fixed << std::setprecision(4) 
                      << stats.min_ratio << ",\n";
            std::cout << "      \"max_bit_change_ratio\": " << std::fixed << std::setprecision(4) 
                      << stats.max_ratio << ",\n";
            std::cout << "      \"mean_bit_change_ratio\": " << std::fixed << std::setprecision(4) 
                      << stats.mean_ratio << ",\n";
            std::cout << "      \"std_deviation\": " << std::fixed << std::setprecision(4) 
                      << stats.std_dev << ",\n";
            std::cout << "      \"execution_time_ms\": " << duration.count() << ",\n";
            std::cout << "      \"status\": \"" << (test_passed ? "PASS" : "FAIL") << "\"\n";
            std::cout << "    }";
        }
    }
    
    std::cout << "\n  ],\n";
    std::cout << "  \"summary\": {\n";
    std::cout << "    \"passed\": " << passed_tests << ",\n";
    std::cout << "    \"total\": " << total_tests << ",\n";
    std::cout << "    \"status\": \"" << (passed_tests == total_tests ? "PASS" : "FAIL") << "\",\n";
    std::cout << "    \"avalanche_criteria\": \"80% of bit flips must result in 40-60% output bit changes\"\n";
    std::cout << "  }\n";
    std::cout << "}\n";
    
    return (passed_tests == total_tests) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    return run_avalanche_tests();
}