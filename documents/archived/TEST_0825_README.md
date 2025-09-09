# CHARM 0825 Test Series - Documentation

## Overview

The 0825 Test Series provides specialized experimental testing and benchmarking for the CHARM algorithm, focusing on research and development applications beyond standard performance testing.

## Components

### 1. Experimental Test Suite v2 (`experimental_test_0825_v2.sh`)
Advanced experimental testing framework providing comprehensive analysis of CHARM algorithm behavior under various research conditions.

#### Test Categories:
1. **Extreme Input Size Stress Testing** - Performance across unusual input sizes (1B to 65535B)
2. **Algorithmic Pattern Analysis** - Behavior with specific bit patterns and avalanche effect quality
3. **Performance Regression Analysis** - Consistency and timing variations across multiple runs
4. **Entropy Quality Assessment** - Output randomness quality using Shannon entropy calculations
5. **Advanced Comparative Benchmarking** - Extended performance comparison with reference algorithms
6. **Memory Usage Profiling** - Resource consumption analysis during processing
7. **Edge Case Testing** - Boundary conditions and error handling verification

### 2. Test Launcher (`test_0825_launcher.sh`)
User-friendly interface for running and managing 0825 experimental tests.

#### Available Options:
- **Option 1**: Run comprehensive experimental test suite
- **Option 2**: Run quick validation mode for rapid testing
- **Option 3**: View previous test results and reports
- **Option 4**: Clean up experimental test data
- **Option 5**: Show detailed documentation

## Usage

### Quick Start
```bash
# Run comprehensive experimental tests
./test_0825_launcher.sh 1

# Run quick validation
./test_0825_launcher.sh 2

# View previous results
./test_0825_launcher.sh 3
```

### Direct Execution
```bash
# Run experimental test suite directly
./benchmarks/experimental_test_0825_v2.sh
```

### Integration with Existing Build System
```bash
# Build required components
make clean && make core && make bench

# Run experimental tests
./test_0825_launcher.sh 1
```

## Output Files

### Main Report
- `EXPERIMENTAL_ANALYSIS_REPORT_20250825_HHMMSS.md` - Comprehensive analysis report

### Raw Data Files (CSV Format)
- `extreme_sizes_20250825_HHMMSS.csv` - Input size performance matrix
- `pattern_analysis_20250825_HHMMSS.csv` - Algorithmic pattern testing results
- `regression_analysis_20250825_HHMMSS.csv` - Performance consistency data
- `entropy_assessment_20250825_HHMMSS.csv` - Output quality metrics
- `memory_profile_20250825_HHMMSS.csv` - Resource usage analysis
- `edge_cases_20250825_HHMMSS.csv` - Boundary condition testing results

### Benchmark Data
- `comparative_benchmark_20250825_HHMMSS.txt` - Multi-algorithm performance comparison

## Key Features

### Research-Grade Analysis
- Statistical performance analysis with multiple runs
- Entropy quality assessment using cryptographic standards
- Pattern resistance testing with known problematic inputs
- Memory efficiency profiling for resource planning

### Experimental Insights
- Performance scaling behavior across extreme input ranges
- Algorithm consistency validation through regression testing
- Cryptographic strength assessment via entropy analysis
- Implementation robustness verification

### Integration Capabilities
- Compatible with existing CHARM build system
- Integrates with unified benchmark framework
- Generates timestamped results for historical tracking
- Provides machine-readable data formats for analysis tools

## System Requirements

### Dependencies
- GCC compiler with AVX2 support
- OpenSSL development libraries
- Python 3 (for entropy calculations)
- Standard Unix utilities (bc, dd, ps, etc.)

### Build Dependencies
- CHARM core system built successfully
- Benchmark components available
- Sufficient disk space for test data generation

## Example Results

### Performance Highlights
- **Small Inputs (1-63B)**: Detailed scaling analysis across irregular boundaries
- **Pattern Analysis**: Avalanche effect quality measurements (e.g., 62/64 bits changed)
- **Regression Testing**: Performance consistency validation with statistical analysis
- **Entropy Assessment**: Shannon entropy scores for cryptographic quality validation

### Research Applications
- Algorithm optimization research
- Security analysis and validation
- Performance characterization studies
- Comparative algorithm analysis

## Troubleshooting

### Common Issues
1. **Build Failures**: Ensure `make clean && make core && make bench` completes successfully
2. **Missing Dependencies**: Install required system packages (gcc, openssl-dev, python3)
3. **Insufficient Permissions**: Ensure scripts are executable (`chmod +x`)
4. **Disk Space**: Ensure sufficient space for temporary test files

### Error Recovery
- Use `./test_0825_launcher.sh 4` to clean up test data
- Check build system with `make help` for available targets
- Verify CHARM binary functionality with `echo "test" | ./build/charm`

## Development Notes

### Customization
The experimental test suite can be modified to add new test categories or adjust existing parameters:
- Test size ranges in extreme size testing
- Pattern sets for algorithmic analysis
- Statistical parameters for regression testing
- Entropy calculation methods

### Extension Points
- Additional algorithm comparisons
- Custom performance metrics
- Extended edge case scenarios
- Advanced statistical analysis

## Version History

- **v2 (0825)**: Initial experimental test suite with 7 test categories
- Focus on research and development applications
- Integration with existing CHARM build system
- Comprehensive data collection and analysis

---

*For more information, see the main CHARM documentation in `documents/` directory.*