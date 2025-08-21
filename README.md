# CHARM

# CHARM Security Suite

An enterprise-grade, entropy‑native security platform providing comprehensive cryptographic services, identity & access management, data protection, threat detection, and security monitoring. Built on CHARM's revolutionary entropy-adaptive hashing framework, it delivers both exceptional performance and advanced security capabilities.

## 🛡️ Security Suite Overview

CHARM has evolved from a specialized entropy-native hashing framework into a **full security suite** that addresses the 9 pillars of enterprise security:

### ✅ Implemented Capabilities (67% Complete)
- **🔐 Cryptographic Services**: Entropy-native hashing, key generation, AEAD encryption
- **👤 Identity & Access**: Authentication adapters, RBAC authorization, session management  
- **📊 Audit Logging**: Tamper-evident logs with hash chains for integrity protection
- **⚙️ Configuration Security**: Secure config management with validation and access control
- **📈 Threat Detection**: Security incident reporting, entropy health monitoring
- **🏥 Health Monitoring**: Real-time system health and security posture assessment

### 🚧 Planned Capabilities  
- **🔗 Supply Chain Security**: SBOM generation, dependency scanning, signed artifacts
- **🚨 Incident Response**: Alerting mechanisms, forensic retention, compliance reporting
- **🔒 Secure Execution**: Sandboxing, memory safety, supply chain validation

## 🚀 Quick Start

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
```

## 🎯 Security Suite Applications

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

## 🏗️ Security Suite Architecture

CHARM Security Suite is built on a layered architecture that integrates multiple security subsystems:

### Core Security Layer
- **🔐 Cryptographic Engine**: Entropy-native CHARM algorithms (CAEDS/CEE/ECE)
- **🔑 Key Management**: Secure key generation, storage, rotation, and lifecycle management
- **👤 Identity & Access**: Authentication adapters and RBAC authorization engine
- **📊 Audit System**: Tamper-evident logging with hash chain integrity protection

### Management Layer  
- **⚙️ Configuration Security**: Secure config management with validation and access control
- **📈 Health Monitoring**: Real-time security posture and entropy quality assessment
- **🚨 Incident Response**: Security event tracking and automated response capabilities
- **🛡️ Unified API**: Single interface for all security operations

### Original CHARM Algorithm Foundation

The underlying CHARM algorithm provides the entropy-native foundation: CHARM/1.0 is a non-linear, sequential streaming hash with fixed 64-byte blocks and 16KB processing chunks, offering 256/384/512-bit outputs. It uses adaptive round scheduling driven by lightweight entropy diagnostics, applying chaotic-style mixing and "entropic collapse" finalization. The framework includes **CHARM-B** for ultra-small input optimization (≤64 bytes) with revolutionary performance gains, and **AEAD/AEAS** for authenticated encryption.

## Security Suite Modules

### 1. **Key Management Framework**
   Comprehensive key lifecycle management with entropy-quality-aware generation, secure storage, and automatic rotation policies.

### 2. **Identity & Access Management**  
   Multi-method authentication (password, token, JWT) with role-based authorization and session management.

### 3. **Audit & Compliance System**
   Tamper-evident audit logging with hash chain integrity, security incident tracking, and compliance reporting.

### 4. **Configuration Security**
   Secure configuration management with validation, access control, and integrity protection.

### 5. **Health & Monitoring**
   Real-time security posture assessment, entropy quality monitoring, and automated health checks.

## Original CHARM Core Modules

### 1. **CAEDS – Chaotic Anomaly & Entropy Diagnostic Subsystem**  
   Continuously monitors system entropy, detects irregularities, and provides adaptive feedback for stability.

### 2. **CEE – Chaotic Entropy Expansion**  
   Generates and amplifies structured entropy to enhance unpredictability, performance, and cryptographic resilience.

3. **ECE – Entropic Collapse Engine**  
   Stabilizes over-chaotic states by compressing excessive entropy into coherent outputs, ensuring consistent and secure operation.

Together, these modules make CHARM a self-regulating, high-performance hashing framework.

## Functional Roles

- **Cryptographic Security**: High-performance, entropy-conditioned hashing with resistance to both classical and quantum attacks.
- **Adaptive Optimization**: Dynamically adjusts internal rounds, memory usage, and feature activation based on input size, avoiding performance bottlenecks without compromising security.
- **Stability Control**: Balances randomness and structure to prevent collisions, interference, or weak outputs, ensuring speed and reliability across all use cases.

## Distinctive Features

- Actively shapes entropy rather than relying solely on passive randomness sources.
- The combination of CAEDS, CEE, and ECE forms a self-regulating cryptographic engine that adapts to input conditions and threat models.
- High efficiency across input sizes, with optimized performance for both small data packets and large datasets.

## Applications

1. **Blockchain Security**: Entropy-conditioned hashing enhances block verification and signature security, maintaining low-latency performance for small transactions.
2. **Distributed Systems**: Ensures data integrity and consistency across nodes under varying load conditions, resilient to tampering or network noise.
3. **Secure Storage & Archiving**: Provides consistent, high-performance hashing across files of all sizes, with long-term quantum-resistant verification.
4. **Cryptographic Protocols**: Serves as a core component in key derivation functions, secure communication channels, and authentication mechanisms.
5. **IoT and Embedded Systems**: CHARM-B provides ultra-efficient hashing for constrained devices, with CHARM-B AEAD offering authenticated encryption without hardware dependencies.
6. **Research and Development**: Comprehensive testing frameworks and benchmarking tools support cryptographic research and algorithm development.

## Installation and Setup

### Prerequisites

- C99-compatible compiler
- POSIX-compliant operating system
- AVX2 or NEON support for full performance (optional)
- 64-bit architecture recommended

### Building CHARM

```bash
# Clone the repository
git clone https://github.com/nwoolr20/CHARM.git
cd CHARM

# Build the core system
make core

# Build the complete security suite  
make security_suite

# Build benchmarks
make bench

# Run core tests
make test

# Run security suite test
make test_security_suite_minimal

# Run performance benchmarks
make benchmark
```

## Security Suite Usage

```bash
# Initialize and test the complete security suite
./build/test_security_suite

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

- **[CHARM-B](CHARM-B/README.md)**: Ultra-small input optimized hash with comprehensive performance analysis
- **[AEAD/AEAS](AEAD/AEAS/README.md)**: Authenticated encryption experiments with performance studies
- **[CHARM-B AEAD](CHARM-B/AEAD/README.md)**: Entropy-native AEAD for constrained environments
- **[Conformance Testing](documents/Conformance-Testing.md)**: NIST-style validation framework

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on how to contribute to CHARM. All contributions must be backed by empirical test results demonstrating performance, stability, or security improvements.

## License

This project is licensed under the CHARM License 2025. See [LICENSE](LICENSE) for details.

---

CHARM is more than a hash function—it is a comprehensive cryptographic framework that integrates entropy management, high-performance hashing, authenticated encryption experiments, and rigorous validation. The framework provides tools for research, development, and specialized applications where entropy-native cryptography offers advantages over conventional approaches.