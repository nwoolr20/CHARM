#!/bin/bash
# test_0825_launcher.sh - Launcher for 0825 experimental testing
# Provides easy access to the experimental test suite v2

set -e

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

echo -e "${MAGENTA}CHARM 0825 Test Suite Launcher${NC}"
echo -e "${MAGENTA}==============================${NC}"
echo

# Show available test options
echo -e "${CYAN}Available 0825 Test Options:${NC}"
echo "1. Run Experimental Test Suite v2 (comprehensive)"
echo "2. Run Experimental Test Suite v2 (quick mode)"
echo "3. View previous 0825 test results"
echo "4. Clean up experimental test data"
echo "5. Show experimental test documentation"
echo

# Get user choice or use first argument
if [ "$1" ]; then
    choice="$1"
else
    echo -n "Select option [1-5]: "
    read choice
fi

case $choice in
    1)
        echo -e "${YELLOW}Running Experimental Test Suite v2 (comprehensive)...${NC}"
        echo
        ./benchmarks/experimental_test_0825_v2.sh
        ;;
    2)
        echo -e "${YELLOW}Running Experimental Test Suite v2 (quick mode)...${NC}"
        echo -e "${BLUE}Note: Quick mode runs subset of tests for rapid validation${NC}"
        echo
        # Set environment variable for quick mode
        export CHARM_QUICK_MODE=1
        ./benchmarks/experimental_test_0825_v2.sh
        ;;
    3)
        echo -e "${YELLOW}Displaying previous 0825 test results...${NC}"
        echo
        
        results_dir="benchmarks/test_results/experimental"
        if [ -d "$results_dir" ]; then
            echo -e "${CYAN}Available 0825 experimental test results:${NC}"
            ls -la "$results_dir" | grep -E "(EXPERIMENTAL|0825)" || echo "No 0825 experimental results found"
            echo
            
            # Show latest report if available
            latest_report=$(ls -t "$results_dir"/EXPERIMENTAL_ANALYSIS_REPORT_20250825_*.md 2>/dev/null | head -1)
            if [ "$latest_report" ]; then
                echo -e "${CYAN}Latest experimental report:${NC}"
                echo "$latest_report"
                echo
                echo -e "${YELLOW}Summary from latest report:${NC}"
                grep -E "(Executive Summary|Status:|Recommendation:)" "$latest_report" | head -10
            fi
        else
            echo "No experimental test results directory found. Run tests first."
        fi
        ;;
    4)
        echo -e "${YELLOW}Cleaning up experimental test data...${NC}"
        
        experimental_dir="benchmarks/test_results/experimental"
        if [ -d "$experimental_dir" ]; then
            echo "Removing experimental test data from: $experimental_dir"
            rm -rf "$experimental_dir"
            echo -e "${GREEN}✓ Experimental test data cleaned up${NC}"
        else
            echo "No experimental test data to clean up"
        fi
        
        # Clean up any temporary files
        rm -f /tmp/test_data_*.bin /tmp/large_result.txt 2>/dev/null || true
        echo -e "${GREEN}✓ Temporary files cleaned up${NC}"
        ;;
    5)
        echo -e "${YELLOW}Experimental Test Suite v2 Documentation${NC}"
        echo -e "${BLUE}=========================================${NC}"
        echo
        echo -e "${CYAN}Purpose:${NC}"
        echo "Advanced experimental testing for CHARM algorithm research and development"
        echo
        echo -e "${CYAN}Test Categories:${NC}"
        echo "1. Extreme Input Size Stress Testing - Performance across unusual input sizes"
        echo "2. Algorithmic Pattern Analysis - Behavior with specific bit patterns"  
        echo "3. Performance Regression Analysis - Consistency and timing variations"
        echo "4. Entropy Quality Assessment - Output randomness quality measurement"
        echo "5. Advanced Comparative Benchmarking - Extended performance comparison"
        echo "6. Memory Usage Profiling - Resource consumption analysis"
        echo "7. Edge Case Testing - Boundary conditions and error handling"
        echo
        echo -e "${CYAN}Output:${NC}"
        echo "- Comprehensive experimental analysis report"
        echo "- Raw data files in CSV format for analysis"
        echo "- Performance metrics and statistical analysis"
        echo "- Research insights and recommendations"
        echo
        echo -e "${CYAN}Usage:${NC}"
        echo "./test_0825_launcher.sh [1|2]"
        echo "  1 = Full comprehensive testing"
        echo "  2 = Quick validation mode"
        echo
        echo -e "${CYAN}Integration:${NC}"
        echo "- Works with existing CHARM build system"
        echo "- Generates timestamped results with 0825 pattern"
        echo "- Compatible with unified benchmark framework"
        echo "- Provides research-grade experimental data"
        ;;
    *)
        echo -e "${YELLOW}Invalid option. Please select 1-5.${NC}"
        exit 1
        ;;
esac

echo
echo -e "${GREEN}0825 Test Suite Launcher completed${NC}"