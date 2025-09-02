# CHARM Security Suite CLI Guide

The CHARM Security Suite ships with a comprehensive command line interface that
provides convenient access to all platform services including capability
inspection, health monitoring, incident reporting, SBOM generation, and support
for all CHARM variants including CHARM-B, AEAD implementations, and experimental features.

## Building

```bash
make security_suite
```

This command builds the `charm_security_suite` executable in the `build/`
directory along with all required security suite components.

## Usage

```bash
charm_security_suite [OPTIONS]
```

### Basic Operations

- `--status` – display current suite status including security level and
  entropy quality.
- `--capabilities` – list enabled security capabilities as a bitmask.
- `--health` – run the comprehensive security health check and list any
  discovered issues.
- `--version` – show the security suite version.
- `--help` – display built‑in help with all options and examples.

### Security Operations

- `--sbom FILE` – generate a software bill of materials in JSON format and
  write it to `FILE`.
- `--report TYPE:DESC` – report a security incident. `TYPE` is one of
  `AUTH_FAILURE`, `AUTHZ_VIOLATION`, `CRYPTO_FAILURE`, `KEY_COMPROMISE`,
  `AUDIT_TAMPERING`, `CONFIG_VIOLATION`, `ENTROPY_DEGRADATION` or
  `UNKNOWN_THREAT`. `DESC` is a short description of the incident.
- `--incidents` – list all security incidents with details.
- `--incidents-unresolved` – list only unresolved security incidents.
- `--resolve ID:ACTIONS` – resolve incident by ID with response actions.
- `--rotate-keys` – perform automatic key rotation for all eligible keys.
- `--verify-integrity` – verify integrity of all security components.
- `--scan-vulns` – scan for vulnerabilities in dependencies.
- `--lockdown REASON` – activate emergency security lockdown.
- `--restore TOKEN` – restore from emergency lockdown with auth token.

### CHARM Variants Support

The CLI provides comprehensive testing and monitoring for all CHARM implementations:

- `--charm-core` – test core CHARM functionality including digest computation.
- `--charm-b` – test CHARM-B implementation with enhanced entropy processing.
- `--charm-aead` – test CHARM AEAD functionality and cipher suites.
- `--experimental` – test experimental variants including fractal-down algorithms and hybrid entropy sources.

### Performance & Monitoring

- `--benchmark` – run performance benchmarks across all CHARM variants.
- `--entropy-quality` – display current entropy quality metrics.
- `--trace-start` – start entropy tracing and monitoring.
- `--trace-stop` – stop entropy tracing.
- `--export FORMAT:FILE` – export compliance data in specified format (json/xml/csv).

### Configuration Management

- `--config-get KEY` – get secure configuration value.
- `--config-set KEY:VALUE` – set secure configuration value with audit logging.
- `--enable CAP` – enable specific security capability.
- `--disable CAP` – disable specific security capability.

### Examples

```bash
# Show current status and capabilities
charm_security_suite --status --capabilities

# Perform a comprehensive health check
charm_security_suite --health --entropy-quality

# Test all CHARM variants with benchmarking
charm_security_suite --charm-core --charm-b --charm-aead --benchmark

# Report an authentication failure incident
charm_security_suite --report AUTH_FAILURE:"Invalid password for admin"

# Generate an SBOM covering all components
charm_security_suite --sbom sbom.json

# Check for vulnerabilities and verify integrity
charm_security_suite --scan-vulns --verify-integrity

# Export compliance data
charm_security_suite --export json:compliance.json

# Test experimental features
charm_security_suite --experimental --trace-start

# Emergency operations
charm_security_suite --lockdown "Security breach detected"
charm_security_suite --restore admin_restore_token

# Key management
charm_security_suite --rotate-keys --incidents
```

## Supported CHARM Components

The security suite CLI provides comprehensive coverage for:

### Core CHARM
- Entropy Collapse Engine (ECE) functionality
- Core digest computation and verification
- System entropy monitoring
- Basic cryptographic operations

### CHARM-B
- Enhanced entropy processing algorithms
- Improved performance optimizations
- Advanced AEAD cipher suite integration
- Extended security feature set

### CHARM AEAD
- ChaCha20-Poly1305 variant implementations
- Authenticated encryption with associated data
- Performance-optimized cipher suites
- Security test vector validation

### Experimental Variants
- Fractal-down algorithm implementations
- Hybrid entropy source integration
- AEAS (Advanced Entropy Analysis System) experimental features
- Research and development components

## Security Features

The CLI initializes the security suite automatically and provides access to:

- **Incident Management**: Report, track, and resolve security incidents
- **Key Management**: Automatic key rotation and lifecycle management
- **Integrity Verification**: Component tampering detection and validation
- **Vulnerability Scanning**: Dependency security analysis
- **Compliance Reporting**: Export audit trails and compliance data
- **Emergency Controls**: Lockdown and recovery mechanisms
- **Performance Monitoring**: Benchmarking across all variants

Multiple options can be combined in a single invocation for comprehensive analysis and monitoring.
