# CHARM Security Suite

An enterprise-grade, entropy‑native security platform providing comprehensive cryptographic services, identity & access management, data protection, threat detection, and security monitoring. Built on CHARM's entropy-adaptive hashing framework, it delivers high performance and security capabilities for modern enterprise environments.

## Security Suite Overview

CHARM is a comprehensive security suite that addresses all critical pillars of enterprise security:

### Implemented Capabilities
- **Cryptographic Services**: Entropy-native hashing, key generation, AEAD encryption
- **PBKDF2 Key Derivation**: RFC 2898 compliant password-based key derivation with HMAC-SHA256
- **Identity & Access**: Authentication adapters, RBAC authorization, session management  
- **Audit Logging**: Tamper-evident logs with hash chains for integrity protection
- **Configuration Security**: Secure config management with validation and access control
- **Threat Detection**: Security incident reporting, entropy health monitoring
- **Health Monitoring**: Real-time system health and security posture assessment
- **Supply Chain Security**: SBOM generation, dependency scanning, signed artifacts
- **Incident Response**: Alerting mechanisms, forensic retention, compliance reporting
- **Secure Execution**: Sandboxing, memory safety, supply chain validation

## Quick Start

### Easy Access with Launch_CHARM

```bash
# Easy launcher for CHARM Security Suite
./Launch_CHARM --help        # Show launcher help
./Launch_CHARM --demo        # Run comprehensive demo
./Launch_CHARM --test-pbkdf2 # Test PBKDF2 functionality
./Launch_CHARM --status      # Quick status check
```

### Build and Test the Security Suite

```bash
# Build the complete security suite
make security_suite

# Run comprehensive security suite test
make test_security_suite_minimal

# Run health check
./build/test_security_suite
```

### Basic Security Suite Usage

```c
#include "charm_security_suite.h"

// Initialize the security suite
charm_security_suite_init(NULL);  // Use default secure config

// Generate and manage keys
charm_key_generation_params_t params = {
    .type = CHARM_KEY_TYPE_SYMMETRIC_AES256,
    .usage_flags = CHARM_KEY_USAGE_ENCRYPT | CHARM_KEY_USAGE_DECRYPT,
    .rotation_days = 30
};
char key_id[64];
charm_key_generate(&params, key_id);

// Authenticate users
charm_auth_context_t auth_context;
charm_auth_authenticate(CHARM_AUTH_METHOD_PASSWORD, "user:pass", &auth_context);

// Secure encryption with automatic key management
uint8_t encrypted[1024];
size_t encrypted_len;
charm_secure_encrypt(data, data_len, key_id, encrypted, sizeof(encrypted), &encrypted_len);

// Security health monitoring
char issues[10][256];
size_t issue_count;
int health_score = charm_security_suite_health_check(issues, 10, &issue_count);
printf("Security Health: %d/100\n", health_score);

// PBKDF2 key derivation
#include "PBKDF2/pbkdf2.h"

pbkdf2_config_t pbkdf2_config;
pbkdf2_context_t pbkdf2_ctx;
pbkdf2_init_default_config(&pbkdf2_config);
pbkdf2_init(&pbkdf2_config, &pbkdf2_ctx);

const char* password = "user_password";
pbkdf2_derive_with_context(&pbkdf2_ctx, (uint8_t*)password, strlen(password));
// Use pbkdf2_ctx.derived_key for encryption keys
pbkdf2_cleanup(&pbkdf2_ctx);
```

## Security Suite Applications

### Enterprise Security
- **Zero Trust Architecture**: Identity verification, access control, and continuous monitoring
- **Data Protection**: Encryption, key management, and secure configuration storage
- **Compliance & Audit**: Tamper-evident logging, incident tracking, and regulatory reporting
- **Threat Detection**: Real-time security monitoring and anomaly detection

### High-Performance Use Cases  
- **Financial Systems**: Ultra-fast transaction security with CHARM-B optimization
- **IoT & Edge**: Lightweight security for constrained environments
- **Blockchain**: High-performance transaction validation and integrity checking
- **Cloud Security**: Scalable key management and audit logging for cloud workloads

### Research & Development
- **Security Research**: Comprehensive testing frameworks and validation harnesses
- **Cryptographic Innovation**: Entropy-native algorithm development and benchmarking
- **Performance Analysis**: Detailed metrics and comparison against established standards

## Security Suite Architecture

CHARM Security Suite is built on a layered architecture that integrates multiple security subsystems:

### Core Security Layer
- **Cryptographic Engine**: Entropy-native CHARM algorithms (CAEDS/CEE/ECE)
- **Key Management**: Secure key generation, storage, rotation, and lifecycle management
- **PBKDF2 Implementation**: RFC 2898 compliant password-based key derivation with HMAC-SHA256
- **Identity & Access**: Authentication adapters and RBAC authorization engine
- **Audit System**: Tamper-evident logging with hash chain integrity protection

### Management Layer  
- **Configuration Security**: Secure config management with validation and access control
- **Health Monitoring**: Real-time security posture and entropy quality assessment
- **Incident Response**: Security event tracking and automated response capabilities
- **Unified API**: Single interface for all security operations

### CHARM Algorithm Foundation

The underlying CHARM algorithm provides the entropy-native foundation: CHARM/1.0 is a non-linear, sequential streaming hash with fixed 64-byte blocks and 16KB processing chunks, offering 256/384/512-bit outputs. It uses adaptive round scheduling driven by lightweight entropy diagnostics, applying chaotic-style mixing and "entropic collapse" finalization. The framework includes **CHARM-B** for ultra-small input optimization (≤64 bytes) with significant performance gains, and **AEAD/AEAS** for authenticated encryption.

## Security Suite Modules

### 1. **Key Management Framework**
   Comprehensive key lifecycle management with entropy-quality-aware generation, secure storage, and automatic rotation policies.

### 2. **Identity & Access Management**  
   Multi-method authentication (password, token, JWT) with role-based authorization and session management.

### 3. **PBKDF2 Key Derivation Framework**
   RFC 2898 compliant password-based key derivation with HMAC-SHA256, configurable iterations, and CHARM entropy integration.

### 4. **Audit & Compliance System**
   Tamper-evident audit logging with hash chain integrity, security incident tracking, and compliance reporting.

### 4. **Configuration Security**
   Secure configuration management with validation, access control, and integrity protection.

### 5. **Health & Monitoring**
   Real-time security posture assessment, entropy quality monitoring, and automated health checks.

## CHARM Core Modules

### 1. **CAEDS – Chaotic Anomaly & Entropy Diagnostic Subsystem**  
   Continuously monitors system entropy, detects irregularities, and provides adaptive feedback for stability.

### 2. **CEE – Chaotic Entropy Expansion**  
   Generates and amplifies structured entropy to enhance unpredictability, performance, and cryptographic resilience.

### 3. **ECE – Entropic Collapse Engine**  
   Stabilizes over-chaotic states by compressing excessive entropy into coherent outputs, ensuring consistent and secure operation.

### 4. **Entropy Bus**
   Routes entropy between system components, manages entropy sources and consumers, and ensures optimal distribution of entropy resources.

Together, these modules make CHARM a self-regulating, high-performance hashing framework.

## Functional Roles

- **Cryptographic Security**: High-performance, entropy-conditioned hashing with resistance to both classical and quantum attacks.
- **Adaptive Optimization**: Dynamically adjusts internal rounds, memory usage, and feature activation based on input size, avoiding performance bottlenecks without compromising security.
- **Stability Control**: Balances randomness and structure to prevent collisions, interference, or weak outputs, ensuring speed and reliability across all use cases.
- **Enterprise Integration**: Provides unified security services through a comprehensive API that integrates cryptography, key management, authentication, and monitoring.
- **Real-time Monitoring**: Continuously assesses system entropy health, security posture, and performance metrics with automated alerting.
- **Compliance Support**: Delivers audit trails, incident tracking, and reporting capabilities to meet regulatory requirements.

## Distinctive Features

- Actively shapes entropy rather than relying solely on passive randomness sources.
- The combination of CAEDS, CEE, ECE, and Entropy Bus forms a self-regulating cryptographic engine that adapts to input conditions and threat models.
- High efficiency across input sizes, with optimized performance for both small data packets and large datasets.
- Comprehensive security suite capabilities integrated with entropy-native cryptographic operations.
- Tamper-evident audit logging with hash-chain integrity protection for enterprise accountability.
- Multi-framework architecture supporting various cryptographic needs from high-performance hashing to authenticated encryption.
- Real-time entropy quality assessment and adaptive algorithm selection based on system conditions.

## Performance and Benchmark Results

### CHARM-B Ultra-Small Input Performance
| Input Size | CHARM-B Performance | vs SHA-256 Improvement |
|------------|-------------------|----------------------|
| 8 bytes    | 642.3 MB/s       | +686.2%             |
| 16 bytes   | 1,527.6 MB/s     | +489.3%             |
| 32 bytes   | 2,120.5 MB/s     | +281.9%             |
| 64 bytes   | 3,112.8 MB/s     | +351.4%             |

### Enterprise Security Suite Performance
- **Key Generation**: >10,000 keys/second with entropy quality validation
- **Authentication**: <1ms response time for credential validation
- **Audit Logging**: >50,000 events/second with tamper-evident integrity
- **Health Monitoring**: Real-time entropy assessment with <100µs overhead

### Real-World Application Performance
- **Blockchain Transaction ID (32 bytes)**: 2,148.6 MB/s
- **API Authentication Token (16 bytes)**: 1,527.9 MB/s  
- **IoT Sensor Reading (8 bytes)**: 1,130.0 MB/s
- **Database Index Key (64 bytes)**: 3,137.0 MB/s

### System Requirements and Capabilities
- **Entropy Density**: >7.95 bits/byte on all digests
- **Throughput**: ≥1.0 GB/s on AVX2 hardware for digest operations
- **Latency**: <1ms for standard cryptographic operations
- **Security Suite Health Score**: 85-100/100 typical operational range

## Applications

### Enterprise and Production Environments
1. **Enterprise Security Suites**: Complete security platform integration with unified API for cryptography, key management, authentication, and audit logging.
2. **Zero Trust Architecture**: Identity verification, continuous authentication, and real-time security posture monitoring.
3. **Compliance and Regulatory**: Tamper-evident audit trails, incident tracking, and automated compliance reporting for financial and healthcare sectors.
4. **Cloud Security**: Scalable key management, secure configuration storage, and distributed security monitoring for cloud workloads.

### High-Performance Computing Applications
5. **Blockchain Security**: Entropy-conditioned hashing enhances block verification and signature security, maintaining low-latency performance for small transactions.
6. **Distributed Systems**: Ensures data integrity and consistency across nodes under varying load conditions, resilient to tampering or network noise.
7. **Financial Transaction Processing**: Ultra-fast cryptographic operations for high-frequency trading, payment processing, and transaction validation.
8. **Database Systems**: High-performance integrity checking, secure indexing, and cryptographic key derivation for large-scale databases.

### Specialized and Constrained Environments  
9. **IoT and Embedded Systems**: CHARM-B provides ultra-efficient hashing for constrained devices, with CHARM-B AEAD offering authenticated encryption without hardware dependencies.
10. **Edge Computing**: Lightweight security operations with real-time entropy health monitoring for distributed edge deployments.
11. **Mobile and Automotive**: Resource-efficient cryptographic operations with adaptive performance based on device capabilities.
12. **Industrial Control Systems**: Secure communications and integrity validation for SCADA, manufacturing, and critical infrastructure.

### Research and Development
13. **Cryptographic Research**: Comprehensive testing frameworks, benchmarking tools, and validation harnesses support algorithm development and security analysis.
14. **Security Protocol Development**: Entropy-native foundation for developing new cryptographic protocols and security mechanisms.
15. **Performance Analysis and Optimization**: Detailed metrics, comparative benchmarking, and algorithm selection for specialized use cases.
16. **Academic and Standards Development**: Reference implementations and validation frameworks for cryptographic research and standardization efforts.

## Installation and Setup

### System Requirements

#### Hardware Requirements
- **CPU**: x86_64 or ARM64 architecture
- **Memory**: Minimum 2GB RAM for building, 64MB for runtime
- **Storage**: 1GB free space for complete build with tests
- **SIMD Support**: AVX2 (Intel/AMD) or NEON (ARM) for optimal performance

#### Software Requirements
- **Compiler**: GCC 4.8+ or Clang 3.4+ with C99 support
- **Operating System**: Linux, macOS, or UNIX-compatible system
- **Build Tools**: Make, standard POSIX utilities
- **Libraries**: OpenSSL development headers (for benchmarking)

### Installation Methods

#### Quick Installation
```bash
# Clone and build
git clone https://github.com/nwoolr20/CHARM.git
cd CHARM
make all

# Install system-wide (requires sudo)
sudo make install

# Or install to custom prefix
make install PREFIX=/usr/local/charm
```

#### Development Build
```bash
# Full development environment
git clone https://github.com/nwoolr20/CHARM.git
cd CHARM

# Build core CHARM framework
make core

# Build complete security suite  
make security_suite

# Build all components and tests
make all

# Run comprehensive test suite
make test_all
```

#### Verification and Testing
```bash
# Verify installation
./build/charm --help

# Quick functionality test
echo "test" | ./build/charm

# Complete system verification
./Launch_CHARM --test

# Performance benchmark
make benchmark

# Security suite validation
make test_security_suite_minimal
```

### Configuration

#### Environment Variables
- **`CHARM_LOG_LEVEL`**: Set logging verbosity (ERROR, WARN, INFO, DEBUG)
- **`CHARM_ENTROPY_SOURCE`**: Override default entropy source
- **`CHARM_SIMD_DISABLE`**: Disable SIMD optimizations (for debugging)

#### Configuration Files
- **System Config**: `/etc/charm/charm.conf` (global settings)
- **User Config**: `~/.config/charm/config` (user preferences)  
- **Runtime Config**: Current directory `.charm_config` (project-specific)

### Platform-Specific Notes

#### Linux (Ubuntu/Debian)
```bash
# Install build dependencies
sudo apt-get update
sudo apt-get install build-essential libssl-dev

# Build and install
make all && sudo make install
```

#### Linux (CentOS/RHEL)
```bash
# Install build dependencies  
sudo yum groupinstall "Development Tools"
sudo yum install openssl-devel

# Build and install
make all && sudo make install
```

#### macOS
```bash
# Install Xcode command line tools
xcode-select --install

# Build (uses system OpenSSL or Homebrew)
make all

# Install to /usr/local
sudo make install
```

### Troubleshooting

#### Build Issues
- **Missing OpenSSL**: Install development headers or set `OPENSSL_DIR`
- **Compiler Errors**: Ensure C99 support and update compiler if needed
- **Permission Issues**: Use `PREFIX` for non-root installation

#### Runtime Issues  
- **SIMD Errors**: Disable with `export CHARM_SIMD_DISABLE=1`
- **Entropy Warnings**: Check `/dev/urandom` availability
- **Performance Issues**: Verify AVX2/NEON support with `./build/charm --cpu-info`

## Security Suite Usage

```bash
# Initialize and test the complete security suite
./build/charm_security_suite

# PBKDF2 Key Derivation Operations
./build/charm_security_suite --pbkdf2-config        # Show PBKDF2 configuration
./build/charm_security_suite --pbkdf2-benchmark     # Benchmark system performance
./build/charm_security_suite --pbkdf2-salt 32       # Generate 32-byte salt
./build/charm_security_suite --pbkdf2-derive "password:100000"  # Derive key

# Use the easy launcher
./Launch_CHARM --test-pbkdf2                       # Test all PBKDF2 features
./Launch_CHARM --demo                              # Complete demonstration

# Example output:
# CHARM Security Suite Test
# ========================
# 1. Initializing CHARM Security Suite...
#    ✅ Security suite initialized successfully
# 2. Security Suite Information:
#    Version: 1.0.0
#    Enabled capabilities: 0x00000027
#      - Cryptographic Services ✅
#      - Key Management ✅  
#      - Identity & Access ✅
#      - Audit Logging ✅
#      - Configuration Management ✅
# 3. Security Health Check:
#    Health Score: 85/100
```

## Legacy CHARM Usage Examples

```bash
# Generate a cryptographic digest
./build/charm --digest input.txt

# Process data in streaming mode
./build/charm --stream-mode large_file.bin

# Check entropy status
./build/charm --entropy-status

# Enable debug output
./build/charm --debug
```

## Documentation

Detailed documentation is available in the `documents` directory:

- [System Overview](documents/SYSTEM_OVERVIEW.md)
- [CAEDS Documentation](documents/CAEDS.md)
- [CEE Documentation](documents/CEE.md)
- [ECE Documentation](documents/ECE.md)
- [Entropy Bus Documentation](documents/entropy_bus.md)

### CHARM Framework Components

- **[Foundational CHARM (src/)](src/)**: Core entropy-native cryptographic framework with ECE, CAEDS, and CEE implementations
- **[CHARM Algorithm (algorithm/)](algorithm/)**: Reference and library implementations of the CHARM hash algorithm with frozen specification
- **[CHARM AEAD (algorithm/aead/)](algorithm/aead/)**: Enterprise-grade authenticated encryption with misuse-resistant SIV variant using CHARM-512
- **[CHARM-B (CHARM-B/)](CHARM-B/)**: Ultra-small input optimized hash with comprehensive performance analysis
- **[CHARM-B AEAD (CHARM-B/AEAD/)](CHARM-B/AEAD/)**: Ultra-fast AEAD for constrained environments with SIV support
- **[AEAD/AEAS (AEAD/AEAS/)](AEAD/AEAS/)**: Authenticated encryption experiments with performance studies  
- **[PBKDF2 Implementation (PBKDF2/)](PBKDF2/)**: RFC 2898 compliant password-based key derivation with CHARM integration
- **[Security Suite (src/crypto/)](src/crypto/)**: Enterprise-grade security platform with comprehensive security services
- **[Conformance Testing (documents/)](documents/)**: NIST-style validation framework

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to CHARM. All contributions must be backed by empirical test results demonstrating performance, stability, or security improvements.

## License

This project is licensed under the CHARM License 2025. See [LICENSE](LICENSE) for details.

---

CHARM is more than a hash function—it is a comprehensive cryptographic framework that integrates entropy management, high-performance hashing, authenticated encryption experiments, and rigorous validation. The framework provides tools for research, development, and specialized applications where entropy-native cryptography offers advantages over conventional approaches.