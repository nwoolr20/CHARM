# Foundational CHARM Implementation

This directory contains the core entropy-native cryptographic framework implementation with ECE, CAEDS, and CEE subsystems.

## Core Components

### Entropy Management Subsystems
- **`core/`**: Core ECE, CAEDS, and CEE implementations
- **`crypto/`**: Enterprise-grade security platform with comprehensive security services
- **`utils/`**: Utilities and helper functions
- **`tests/`**: Integration tests and benchmarks

### Main Applications
- **`main.c`**: Full CHARM framework application
- **`main_simple.c`**: Simplified CHARM implementation for basic use cases
- **`charmctl.c`**: CHARM control utility for system management
- **`security_suite_cli.c`**: Security suite command-line interface
- **`watchdog_daemon.c`**: Entropy monitoring and system health daemon

### Framework Architecture

CHARM (Chaotic Hashing and Adaptive Resonance Module) is built on three core subsystems:

#### 1. ECE (Entropic Collapse Engine)
- **Location**: `core/ece_*.c`
- Stabilizes over-chaotic states by compressing excessive entropy
- Ensures consistent and secure cryptographic operations
- Provides core hashing and digest functionality

#### 2. CAEDS (Chaotic Anomaly & Entropy Diagnostic Subsystem)  
- **Location**: `core/caeds_*.c`
- Continuously monitors system entropy and detects irregularities
- Provides adaptive feedback to maintain entropy balance
- Implements anomaly detection and predictive analysis

#### 3. CEE (Chaotic Entropy Expansion)
- **Location**: `core/cee_*.c`
- Generates and amplifies structured entropy
- Enhances unpredictability and cryptographic resilience
- Provides entropy mixing and whitening capabilities

### Entropy Bus System
- **`core/entropy_bus.c`**: Central entropy distribution and coordination
- **`core/system_entropy.c`**: System-level entropy collection and management
- Provides unified entropy interface for all framework components

## Building

```bash
# Build complete CHARM framework
make core

# Build specific components
make charm_minimal_api
make security_suite
```

## Usage

### Framework Initialization
```c
#include "ece_core.h"
#include "entropy_bus.h"

// Initialize CHARM framework
ece_init();
entropy_bus_init();

// Generate cryptographic digest
uint8_t hash[32];
ece_digest(data, data_len, hash, sizeof(hash));
```

### Security Suite Integration
```c
#include "charm_security_suite.h"

// Initialize complete security suite
charm_security_suite_init(NULL);

// Use enterprise security services
charm_secure_encrypt(data, data_len, key_id, encrypted, encrypted_len);
```

## Architecture Features

- **Entropy-Native Design**: >7.95 bits/byte entropy density
- **SIMD Acceleration**: AVX2 and NEON optimizations for performance
- **Chaotic Dynamics**: Non-linear entropy processing with adaptive resonance
- **Self-Regulating**: Automatic entropy balance and system health monitoring
- **Modular Design**: Independent subsystems with clean interfaces

This implementation provides the foundational cryptographic framework for the entire CHARM Security Suite ecosystem.