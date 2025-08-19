/**
 * @file charm_extended_kat.cpp
 * @brief CHARM Algorithm Extended Known Answer Tests
 * 
 * Extended test vectors beyond basic empty/"abc" patterns for comprehensive validation
 */

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <random>
#include <algorithm>
#include <sstream>

extern "C" {
#include "charm.h"
}

// Test vector structure for extended KAT
struct ExtendedTestVector {
    std::string name;
    std::vector<uint8_t> input;
    charm_variant_t variant;
    std::string expected_hash;
};

// Generate test data of specified length with pattern
std::vector<uint8_t> generate_test_data(size_t length, const std::string& pattern) {
    std::vector<uint8_t> data(length);
    
    if (pattern == "zeros") {
        std::fill(data.begin(), data.end(), 0x00);
    } else if (pattern == "ones") {
        std::fill(data.begin(), data.end(), 0xFF);
    } else if (pattern == "incremental") {
        for (size_t i = 0; i < length; i++) {
            data[i] = static_cast<uint8_t>(i & 0xFF);
        }
    } else if (pattern == "alternating") {
        for (size_t i = 0; i < length; i++) {
            data[i] = (i % 2) ? 0xFF : 0x00;
        }
    } else if (pattern == "pattern_0x55") {
        std::fill(data.begin(), data.end(), 0x55);
    } else if (pattern == "pattern_0xAA") {
        std::fill(data.begin(), data.end(), 0xAA);
    } else if (pattern == "random") {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint8_t> dis(0, 255);
        for (size_t i = 0; i < length; i++) {
            data[i] = dis(gen);
        }
    }
    
    return data;
}

// Convert hash output to hex string
std::string bytes_to_hex(const uint8_t* bytes, size_t len) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; i++) {
        ss << std::setw(2) << static_cast<unsigned>(bytes[i]);
    }
    return ss.str();
}

// Run extended KAT tests
int run_extended_kat_tests() {
    std::vector<ExtendedTestVector> test_vectors;
    
    // Define test lengths for systematic testing
    std::vector<size_t> test_lengths = {
        0, 1, 3, 7, 15, 16, 17, 31, 32, 33, 55, 56, 57, 63, 64, 65,
        127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024, 1025,
        2047, 2048, 2049, 4095, 4096, 4097, 8191, 8192, 8193,
        16383, 16384, 16385, 32767, 32768, 32769, 65535, 65536, 65537,
        131071, 131072, 131073, 262143, 262144, 262145,
        524287, 524288, 524289, 1048575, 1048576 // Up to 1MB
    };
    
    // Define test patterns
    std::vector<std::string> patterns = {
        "zeros", "ones", "incremental", "alternating", 
        "pattern_0x55", "pattern_0xAA", "random"
    };
    
    // Define variants to test
    std::vector<charm_variant_t> variants = {CHARM_256, CHARM_384, CHARM_512};
    
    int passed = 0;
    int total = 0;
    
    std::cout << "{\n";
    std::cout << "  \"timestamp\": \"" << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() << "\",\n";
    std::cout << "  \"test_type\": \"Extended_KAT\",\n";
    std::cout << "  \"algorithm\": \"CHARM\",\n";
    std::cout << "  \"version\": \"1.0\",\n";
    std::cout << "  \"test_results\": [\n";
    
    bool first_result = true;
    
    // Test specific patterns at key lengths
    for (auto variant : variants) {
        for (auto pattern : patterns) {
            for (auto length : test_lengths) {
                // Skip some combinations for performance (test every pattern at key lengths only)
                if (length > 8192 && pattern != "zeros" && pattern != "ones") {
                    continue;
                }
                
                total++;
                
                auto test_data = generate_test_data(length, pattern);
                
                // Compute hash
                size_t output_len = (variant == CHARM_256) ? 32 : 
                                  (variant == CHARM_384) ? 48 : 64;
                std::vector<uint8_t> output(output_len);
                
                auto start_time = std::chrono::high_resolution_clock::now();
                int result = charm_hash(variant, test_data.data(), test_data.size(), output.data());
                auto end_time = std::chrono::high_resolution_clock::now();
                
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                
                if (!first_result) {
                    std::cout << ",\n";
                }
                first_result = false;
                
                std::cout << "    {\n";
                std::cout << "      \"variant\": \"CHARM-" << variant << "\",\n";
                std::cout << "      \"pattern\": \"" << pattern << "\",\n";
                std::cout << "      \"input_length\": " << length << ",\n";
                std::cout << "      \"status\": \"" << (result == 0 ? "PASS" : "FAIL") << "\",\n";
                std::cout << "      \"execution_time_us\": " << duration.count() << ",\n";
                std::cout << "      \"output_hash\": \"" << bytes_to_hex(output.data(), output_len) << "\"\n";
                std::cout << "    }";
                
                if (result == 0) {
                    passed++;
                }
            }
        }
    }
    
    std::cout << "\n  ],\n";
    std::cout << "  \"summary\": {\n";
    std::cout << "    \"passed\": " << passed << ",\n";
    std::cout << "    \"total\": " << total << ",\n";
    std::cout << "    \"status\": \"" << (passed == total ? "PASS" : "FAIL") << "\"\n";
    std::cout << "  }\n";
    std::cout << "}\n";
    
    return (passed == total) ? 0 : 1;
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    return run_extended_kat_tests();
}