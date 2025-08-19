/**
 * @file charm_stream.cpp
 * @brief CHARM Streaming Tests Runner
 * 
 * Validates that CHARM produces identical results regardless of input chunking.
 */

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <cstring>

extern "C" {
#include "charm.h"
}

// Convert bytes to hex string
std::string bytes_to_hex(const uint8_t* data, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

// Test streaming vs single-shot for a given input
bool test_streaming_pattern(charm_variant_t variant, const std::vector<uint8_t>& input, 
                           const std::vector<size_t>& chunk_sizes) {
    const size_t digest_size = variant / 8;
    
    // Single-shot computation
    std::vector<uint8_t> single_shot(digest_size);
    int result = charm_hash(variant, input.data(), input.size(), single_shot.data());
    if (result != 0) return false;
    
    // Streaming computation
    charm_params_t params = {
        .version = 1,
        .out_bits = static_cast<uint16_t>(variant),
        .flags = 0,
        .reserved = {0}
    };
    
    char ctx_buffer[1024]; // Assume context fits in 1KB
    charm_ctx_t* ctx = reinterpret_cast<charm_ctx_t*>(ctx_buffer);
    
    result = charm_init(ctx, &params, nullptr, 0, nullptr, 0);
    if (result != 0) return false;
    
    // Update in chunks
    size_t pos = 0;
    for (size_t chunk_size : chunk_sizes) {
        if (pos >= input.size()) break;
        
        size_t actual_size = std::min(chunk_size, input.size() - pos);
        result = charm_update(ctx, input.data() + pos, actual_size);
        if (result != 0) return false;
        
        pos += actual_size;
    }
    
    // Process any remaining data
    if (pos < input.size()) {
        result = charm_update(ctx, input.data() + pos, input.size() - pos);
        if (result != 0) return false;
    }
    
    std::vector<uint8_t> streaming(digest_size);
    result = charm_final(ctx, streaming.data());
    if (result != 0) return false;
    
    // Compare results
    return std::memcmp(single_shot.data(), streaming.data(), digest_size) == 0;
}

int main(int argc, char* argv[]) {
    std::cout << "{\n";
    std::cout << "  \"timestamp\": \"" << std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() << "\",\n";
    std::cout << "  \"test_type\": \"Streaming\",\n";
    std::cout << "  \"algorithm\": \"CHARM\",\n";
    std::cout << "  \"version\": \"1.0\",\n";
    
    // Test patterns
    std::vector<std::pair<std::string, std::vector<size_t>>> patterns = {
        {"single_byte", {1, 1, 1, 1, 1}},
        {"small_chunks", {7, 13, 7, 13, 7}},
        {"block_boundary", {63, 64, 65}},
        {"mixed_sizes", {1, 31, 127, 1024}},
        {"large_chunk", {16384}}
    };
    
    // Test input (larger than typical block size)
    std::vector<uint8_t> test_input(1000);
    for (size_t i = 0; i < test_input.size(); ++i) {
        test_input[i] = static_cast<uint8_t>((i * 17 + 42) % 256);
    }
    
    int passed = 0;
    int total = 0;
    
    for (auto variant : {256, 384, 512}) {
        std::cout << "  \"CHARM-" << variant << "\": {\n";
        
        int variant_passed = 0;
        int variant_total = patterns.size();
        
        for (size_t i = 0; i < patterns.size(); ++i) {
            const auto& pattern = patterns[i];
            total++;
            
            bool result = test_streaming_pattern(static_cast<charm_variant_t>(variant), 
                                               test_input, pattern.second);
            
            std::cout << "    \"" << pattern.first << "\": \"" << (result ? "PASS" : "FAIL") << "\"";
            
            if (i < patterns.size() - 1) {
                std::cout << ",";
            }
            std::cout << "\n";
            
            if (result) {
                passed++;
                variant_passed++;
            }
        }
        
        std::cout << "  }";
        if (variant != 512) {
            std::cout << ",";
        }
        std::cout << "\n";
    }
    
    std::cout << "  \"summary\": {\n";
    std::cout << "    \"passed\": " << passed << ",\n";
    std::cout << "    \"total\": " << total << ",\n";
    std::cout << "    \"status\": \"" << (passed == total ? "PASS" : "FAIL") << "\"\n";
    std::cout << "  }\n";
    std::cout << "}\n";
    
    return (passed == total) ? 0 : 1;
}