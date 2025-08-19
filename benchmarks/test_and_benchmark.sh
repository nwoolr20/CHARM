#!/bin/bash
# test_and_benchmark.sh - Legacy script redirecting to unified benchmark system
# This script now calls the unified benchmark system for comprehensive testing

echo "CHARM Test & Benchmark Suite"
echo "=============================" 
echo "Redirecting to unified benchmark system..."
echo

# Call the unified benchmark script
exec "$(dirname "$0")/unified_benchmark.sh" "$@"
