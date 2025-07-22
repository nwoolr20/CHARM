#!/bin/bash
# test_all.sh - Comprehensive test suite for CHARM system

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test directory
TEST_DIR="$(pwd)/test_output"
mkdir -p "$TEST_DIR"

echo -e "${YELLOW}Starting CHARM test suite...${NC}"
echo "Test results will be saved to $TEST_DIR"

# Function to run a test and check result
run_test() {
    local test_name="$1"
    local test_cmd="$2"
    local expected_result="$3"
    
    echo -e "\n${YELLOW}Running test: ${test_name}${NC}"
    echo "Command: $test_cmd"
    
    # Run the test and capture output
    if eval "$test_cmd" > "$TEST_DIR/${test_name}.log" 2>&1; then
        if [[ -n "$expected_result" ]] && ! grep -q "$expected_result" "$TEST_DIR/${test_name}.log"; then
            echo -e "${RED}✗ Test failed: Expected output not found${NC}"
            echo "Expected to find: $expected_result"
            echo "Check $TEST_DIR/${test_name}.log for details"
            return 1
        else
            echo -e "${GREEN}✓ Test passed${NC}"
            return 0
        fi
    else
        echo -e "${RED}✗ Test failed: Command returned non-zero exit code${NC}"
        echo "Check $TEST_DIR/${test_name}.log for details"
        return 1
    fi
}

# Function to test entropy density
test_entropy_density() {
    local input_file="$1"
    local min_entropy="7.95"
    
    echo -e "\n${YELLOW}Testing entropy density on $input_file${NC}"
    
    # Generate digest and measure entropy
    ./charm --digest "$input_file" --analyze > "$TEST_DIR/entropy_density.log" 2>&1
    
    # Extract entropy density from output
    local density=$(grep "Entropy density:" "$TEST_DIR/entropy_density.log" | awk '{print $3}')
    
    if (( $(echo "$density >= $min_entropy" | bc -l) )); then
        echo -e "${GREEN}✓ Entropy density test passed: $density bits/byte (minimum: $min_entropy)${NC}"
        return 0
    else
        echo -e "${RED}✗ Entropy density test failed: $density bits/byte (minimum: $min_entropy)${NC}"
        return 1
    fi
}

# Function to test collision avoidance
test_collision_avoidance() {
    echo -e "\n${YELLOW}Testing digest collision avoidance${NC}"
    
    # Create test files with 1-bit differences
    echo "Original test data" > "$TEST_DIR/collision_test_1.txt"
    echo "Original tEst data" > "$TEST_DIR/collision_test_2.txt" # 1-bit change
    
    # Generate digests
    ./charm --digest "$TEST_DIR/collision_test_1.txt" > "$TEST_DIR/digest_1.log" 2>&1
    ./charm --digest "$TEST_DIR/collision_test_2.txt" > "$TEST_DIR/digest_2.log" 2>&1
    
    # Extract digests
    local digest1=$(grep "Digest:" "$TEST_DIR/digest_1.log" | awk '{print $2}')
    local digest2=$(grep "Digest:" "$TEST_DIR/digest_2.log" | awk '{print $2}')
    
    if [[ "$digest1" != "$digest2" ]]; then
        echo -e "${GREEN}✓ Collision avoidance test passed: Digests are different${NC}"
        return 0
    else
        echo -e "${RED}✗ Collision avoidance test failed: Digests are identical${NC}"
        return 1
    fi
}

# Function to test entropy health transitions
test_entropy_health_transitions() {
    echo -e "\n${YELLOW}Testing entropy health transitions${NC}"
    
    # Start with clean state
    ./charm --reset-entropy-state > /dev/null 2>&1 || true
    
    # Test INIT state
    ./charm --entropy-status > "$TEST_DIR/entropy_init.log" 2>&1
    if ! grep -q "INIT" "$TEST_DIR/entropy_init.log"; then
        echo -e "${RED}✗ Failed to enter INIT state${NC}"
        return 1
    fi
    
    # Transition to MONITORING
    ./charm --seed-entropy > "$TEST_DIR/entropy_seed.log" 2>&1
    sleep 1
    ./charm --entropy-status > "$TEST_DIR/entropy_monitoring.log" 2>&1
    if ! grep -q "MONITORING" "$TEST_DIR/entropy_monitoring.log"; then
        echo -e "${RED}✗ Failed to transition to MONITORING state${NC}"
        return 1
    fi
    
    # Simulate degraded entropy
    ./charm --simulate-entropy-degradation > "$TEST_DIR/entropy_degrade.log" 2>&1
    sleep 1
    ./charm --entropy-status > "$TEST_DIR/entropy_degraded.log" 2>&1
    if ! grep -q "DEGRADED" "$TEST_DIR/entropy_degraded.log"; then
        echo -e "${RED}✗ Failed to transition to DEGRADED state${NC}"
        return 1
    fi
    
    # Simulate critical entropy failure
    ./charm --simulate-entropy-failure > "$TEST_DIR/entropy_failure.log" 2>&1
    sleep 1
    ./charm --entropy-status > "$TEST_DIR/entropy_failover.log" 2>&1
    if ! grep -q "FAILOVER" "$TEST_DIR/entropy_failover.log"; then
        echo -e "${RED}✗ Failed to transition to FAILOVER state${NC}"
        return 1
    fi
    
    echo -e "${GREEN}✓ Entropy health transitions test passed${NC}"
    return 0
}

# Function to run benchmarks
run_benchmarks() {
    echo -e "\n${YELLOW}Running performance benchmarks${NC}"
    
    # Create a large test file
    dd if=/dev/urandom of="$TEST_DIR/benchmark.dat" bs=1M count=10 2> /dev/null
    
    # Run benchmark
    ./charm --bench-digest "$TEST_DIR/benchmark.dat" > "$TEST_DIR/benchmark.log" 2>&1
    
    # Extract throughput
    local throughput=$(grep "Throughput:" "$TEST_DIR/benchmark.log" | awk '{print $2}')
    local unit=$(grep "Throughput:" "$TEST_DIR/benchmark.log" | awk '{print $3}')
    
    echo -e "Benchmark results: $throughput $unit"
    
    # Check if throughput meets minimum requirement (1.0 GB/s on AVX2)
    if [[ "$unit" == "GB/s" ]] && (( $(echo "$throughput >= 1.0" | bc -l) )); then
        echo -e "${GREEN}✓ Performance benchmark passed: $throughput $unit (minimum: 1.0 GB/s)${NC}"
        return 0
    elif [[ "$unit" == "MB/s" ]] && (( $(echo "$throughput >= 1000.0" | bc -l) )); then
        echo -e "${GREEN}✓ Performance benchmark passed: $throughput $unit (minimum: 1000.0 MB/s)${NC}"
        return 0
    else
        echo -e "${YELLOW}⚠ Performance benchmark below target: $throughput $unit${NC}"
        echo "Note: Performance targets are for AVX2-enabled hardware. Your system may have different capabilities."
        return 0  # Don't fail the test suite for performance reasons
    fi
}

# Main test sequence
echo -e "\n${YELLOW}=== Unit Tests ===${NC}"

# Test each entropy source
run_test "hardware_rng" "./charm --test-entropy-source=hardware" "Hardware RNG test passed"
run_test "cpu_jitter" "./charm --test-entropy-source=jitter" "CPU jitter test passed"
run_test "fallback_entropy" "./charm --test-entropy-source=fallback" "Fallback entropy test passed"

# Test core components
run_test "caeds_flux" "./charm --test-component=caeds_flux" "CAEDS flux test passed"
run_test "caeds_anomaly" "./charm --test-component=caeds_anomaly" "CAEDS anomaly test passed"
run_test "caeds_predict" "./charm --test-component=caeds_predict" "CAEDS predict test passed"
run_test "caeds_notify" "./charm --test-component=caeds_notify" "CAEDS notify test passed"

run_test "cee_mix" "./charm --test-component=cee_mix" "CEE mix test passed"
run_test "cee_whiten" "./charm --test-component=cee_whiten" "CEE whiten test passed"
run_test "cee_buffer" "./charm --test-component=cee_buffer" "CEE buffer test passed"

run_test "ece_core" "./charm --test-component=ece_core" "ECE core test passed"
run_test "ece_digest" "./charm --test-component=ece_digest" "ECE digest test passed"
run_test "ece_seed" "./charm --test-component=ece_seed" "ECE seed test passed"
run_test "ece_stream" "./charm --test-component=ece_stream" "ECE stream test passed"

run_test "entropy_bus" "./charm --test-component=entropy_bus" "Entropy bus test passed"
run_test "watchdog_daemon" "./charm --test-component=watchdog" "Watchdog daemon test passed"
run_test "snapshot_logger" "./charm --test-component=snapshot" "Snapshot logger test passed"

echo -e "\n${YELLOW}=== Integration Tests ===${NC}"

# Create test files
echo "Test data for CHARM digest" > "$TEST_DIR/test_file.txt"
dd if=/dev/urandom of="$TEST_DIR/random_data.bin" bs=1K count=64 2> /dev/null

# Test CLI functionality
run_test "cli_digest" "./charm --digest $TEST_DIR/test_file.txt" "Digest:"
run_test "cli_stream" "./charm --stream-mode $TEST_DIR/random_data.bin" "Stream processing complete"
run_test "cli_entropy_status" "./charm --entropy-status" "Entropy Status"

# Test advanced features
test_collision_avoidance
test_entropy_health_transitions
test_entropy_density "$TEST_DIR/random_data.bin"
run_benchmarks

# Count test results
TOTAL_TESTS=$(grep -c "Test passed\|Test failed" "$TEST_DIR/test_summary.log" 2>/dev/null || echo 0)
PASSED_TESTS=$(grep -c "Test passed" "$TEST_DIR/test_summary.log" 2>/dev/null || echo 0)

# Print summary
echo -e "\n${YELLOW}=== Test Summary ===${NC}"
echo "Total tests: $TOTAL_TESTS"
echo "Passed tests: $PASSED_TESTS"
echo "Failed tests: $((TOTAL_TESTS - PASSED_TESTS))"

if [[ $((TOTAL_TESTS - PASSED_TESTS)) -eq 0 ]]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Check logs in $TEST_DIR for details.${NC}"
    exit 1
fi
