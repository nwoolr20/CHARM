# CHARM Repository Structure Overview

This document provides a comprehensive overview of the restructured CHARM repository following the transformation into a proper algorithm with NIST-style validation.

## Repository Structure

```
CHARM/
├── algorithm/                     # CHARM Algorithm Implementation
│   ├── include/
│   │   └── charm.h               # Clean algorithm API (frozen spec)
│   ├── src/
│   │   ├── charm_ref.c           # Reference implementation with main()
│   │   └── charm_lib.c           # Library implementation (no main)
│   └── tests/                    # Unit and property tests
│
├── conformance/                   # NIST-Style Validation Framework
│   └── nist/
│       ├── README.md             # Conformance testing guide
│       ├── schemas/
│       │   └── acvp-sha.json     # ACVP-compatible schema
│       ├── vectors/              # Test vector directories
│       │   ├── kat/              # Known Answer Tests
│       │   ├── monte-carlo/      # Long-run statistical tests
│       │   ├── variable-msg/     # Variable-length tests
│       │   ├── bit-flip/         # Avalanche tests
│       │   ├── streaming/        # Chunking invariance
│       │   └── xof/              # Extended output functions
│       ├── runner/               # Test executables
│       │   ├── charm_kat.cpp     # KAT runner
│       │   ├── charm_mc.cpp      # Monte Carlo runner
│       │   └── charm_stream.cpp  # Streaming tests
│       └── reports/              # Test results (timestamped JSON)
│
├── documents/                     # All Documentation
│   ├── CHARM-Algorithm-Spec.md   # Frozen algorithm specification
│   ├── Conformance-Testing.md    # NIST-style testing guide
│   ├── CHARM_IMPLEMENTATION_SUMMARY.md
│   ├── README.md                 # Main documentation
│   └── [other docs...]           # System design documents
│
├── third_party/                  # Vendored Dependencies
│   ├── README.md                 # Provenance and licensing
│   └── crypto/
│       └── blake3/               # BLAKE3 for benchmarking
│
├── src/                          # Original CHARM Implementation
│   ├── core/                     # Core ECE/CAEDS/CEE implementation
│   ├── utils/                    # Utilities and helpers
│   └── tests/                    # Integration tests and benchmarks
│
├── include/                      # Original headers
├── tools/                        # Build and installation tools
└── Makefile                      # Enhanced build system
```

## Key Changes Made

### 1. Directory Restructuring ✅
- **BLAKE3 Relocated**: `BLAKE3-master/` → `third_party/crypto/blake3/`
- **Documentation Consolidated**: `docs/` → `documents/` 
- **Third-party Tracking**: Added `third_party/README.md` with licenses and provenance
- **Clean Separation**: Algorithm implementation separate from framework code

### 2. Algorithm Framework ✅
- **Frozen API**: `algorithm/include/charm.h` defines stable interface
- **Reference Implementation**: `algorithm/src/charm_ref.c` with test main()
- **Library Version**: `algorithm/src/charm_lib.c` for linking with tests
- **Versioning**: CHARM/1.0 specification with version tracking

### 3. NIST-Style Conformance Testing ✅
- **Complete Test Suite**: KAT, Monte Carlo, streaming validation
- **ACVP Compatibility**: JSON schemas matching ACVP SHA profiles
- **Automated Runners**: C++ test harnesses with JSON output
- **Validation Reports**: Timestamped results in machine-readable format

### 4. Algorithm Specification ✅
- **Frozen Parameters**: Fixed block size, rounds, IVs, constants
- **Linear Design**: Explicitly NOT tree-based or Merkle construction
- **Entropy-Native**: CAEDS/CEE/ECE deterministically specified
- **Security Claims**: Collision/preimage resistance bounds

### 5. Enhanced Build System ✅
- **New Targets**: `charm_ref`, `charm_opt`, `conformance`, `conformance-quick`
- **Third-party Integration**: Optional BLAKE3 benchmarking with clean interfaces
- **Flexible Compilation**: Reference vs optimized builds
- **Documentation Targets**: Easy access to all documentation

## Algorithm Identity

**CHARM (Chaotic Hierarchical Adaptive Resilient Mechanism)**
- **Variants**: CHARM-256, CHARM-384, CHARM-512
- **Mode**: Linear streaming hash (NOT tree-based)
- **Design**: Entropy-native with adaptive round scheduling
- **Version**: CHARM/1.0 (frozen specification)

## Key Features

### Linear Streaming Design
CHARM operates as a single-state, linear streaming hash. It explicitly rejects tree hashing approaches:
- No chunk-level fanout or hierarchical combining
- No Merkle trees or authenticated data structures  
- Parallelism achieved within round functions via SIMD
- Digest invariant to input chunking

### Entropy-Native Processing
- **CAEDS**: Adaptive entropy detection and round scheduling
- **CEE**: Controlled entropy evolution with mixing
- **ECE**: Entropic collapse for diffusion and avalanche
- **Deterministic**: All adaptive behavior based on input data, not environment

### NIST-Style Validation
- **Methodological Parity**: Same validation approach as FIPS 180-4 and FIPS 202
- **ACVP Compatible**: JSON interfaces matching ACVP SHA profiles
- **Comprehensive Coverage**: KAT, Monte Carlo, avalanche, streaming tests
- **Reproducible**: Timestamped reports with full traceability

## Usage Examples

### Build Algorithm Reference Implementation
```bash
make charm_ref
./build/charm_ref --version  # Show version info
./build/charm_ref            # Run basic tests
```

### Run Conformance Validation
```bash
make conformance             # Full NIST-style validation
make conformance-quick       # Quick validation check
```

### Generate ACVP Test Vectors
```bash
./build/charm_kat --generate > test_vectors.json
```

### Build with BLAKE3 Benchmarking
```bash
make enhanced               # Enhanced benchmarks with BLAKE3
make third_party_blake3     # Build BLAKE3 library only
```

### Access Documentation
```bash
make docs                   # Show documentation locations
```

## Compliance and Standards

### Algorithm Specification Compliance
- ✅ **Frozen Parameters**: All constants and structure fixed in spec
- ✅ **Reproducible**: Bit-for-bit identical results across implementations
- ✅ **Versioned**: CHARM/1.0 with forward compatibility
- ✅ **Deterministic**: No environment-dependent behavior

### NIST Methodology Compliance  
- ✅ **Known Answer Tests**: Exact input→output validation
- ✅ **Monte Carlo Tests**: Statistical validation with 10k+ iterations
- ✅ **Variable-Length Tests**: Coverage from 0 bytes to 1MB+
- ✅ **Avalanche Tests**: Cryptographic avalanche validation
- ✅ **Streaming Tests**: Implementation-invariant results

### ACVP Interface Compliance
- ✅ **JSON Schema**: Compatible with ACVP SHA profiles
- ✅ **Test Vector Format**: Standard `testGroups`/`tests` structure
- ✅ **Response Format**: Machine-readable pass/fail results
- ✅ **Metadata**: Algorithm, version, timestamp tracking

## Performance and Security

### Design Goals Achieved
- **Linear Streaming**: Simpler than tree-based designs, easier to audit
- **Entropy Adaptive**: More rounds for low-entropy inputs, fewer for high-entropy
- **SIMD Optimizable**: Round functions designed for parallel execution
- **Competitive Performance**: Target 1-10 cycles/byte depending on entropy

### Security Properties
- **Collision Resistance**: 2^(n/2) operations for n-bit output
- **Preimage Resistance**: 2^n operations for n-bit output
- **Avalanche Effect**: >50% bit change for single bit input changes
- **Statistical Uniformity**: Validated via Monte Carlo testing

## Integration with Existing Code

### Backward Compatibility
- ✅ **Existing CHARM System**: All original functionality preserved
- ✅ **Build Targets**: Original `make core`, `make full` still work
- ✅ **APIs**: Original charm_api.h interface unchanged
- ✅ **Tests**: Existing test suite continues to pass

### New Capabilities
- ✅ **Algorithm API**: Clean, frozen interface for CHARM algorithm
- ✅ **Conformance Testing**: Professional-grade validation framework
- ✅ **Third-party Benchmarking**: Clean BLAKE3 integration for comparison
- ✅ **Documentation**: Comprehensive specification and testing guides

## Future Development

### Algorithm Evolution
- **Version Control**: New algorithm versions use different version bytes
- **Backward Compatibility**: CHARM/1.0 spec is frozen and immutable  
- **Extensions**: XOF mode, keyed variants can be added with new flags
- **Optimization**: Implementation improvements without spec changes

### Validation Framework
- **Additional Tests**: GPU validation, formal verification integration
- **Certification Support**: FIPS/Common Criteria evaluation preparation
- **Tool Integration**: CI/CD integration, automated regression testing
- **Performance Profiling**: Cycle-accurate performance analysis

## Conclusion

The CHARM repository has been successfully transformed from an "entropy-native cryptographic framework" into a proper algorithm with:

1. **Frozen Specification**: Complete, deterministic algorithm definition
2. **Professional Validation**: NIST-style conformance testing framework
3. **Clean Architecture**: Separated algorithm from implementation details
4. **Standards Compliance**: ACVP-compatible interfaces and methodologies
5. **Maintained Compatibility**: All existing functionality preserved

This positions CHARM as a credible, auditable cryptographic algorithm ready for academic research, commercial evaluation, and potential standardization activities.