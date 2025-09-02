# CHARM Security Suite CLI Guide

The CHARM Security Suite ships with a dedicated command line interface that
provides convenient access to core platform services such as capability
inspection, health monitoring, incident reporting and SBOM generation.

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

### Options

- `--status` – display current suite status including security level and
  entropy quality.
- `--capabilities` – list enabled security capabilities as a bitmask.
- `--health` – run the comprehensive security health check and list any
  discovered issues.
- `--sbom FILE` – generate a software bill of materials in JSON format and
  write it to `FILE`.
- `--report TYPE:DESC` – report a security incident. `TYPE` is one of
  `AUTH_FAILURE`, `AUTHZ_VIOLATION`, `CRYPTO_FAILURE`, `KEY_COMPROMISE`,
  `AUDIT_TAMPERING`, `CONFIG_VIOLATION`, `ENTROPY_DEGRADATION` or
  `UNKNOWN_THREAT`. `DESC` is a short description of the incident.
- `--version` – show the security suite version.
- `--help` – display built‑in help with all options and examples.

### Examples

```bash
# Show current status and capabilities
charm_security_suite --status --capabilities

# Perform a health check
charm_security_suite --health

# Report an authentication failure incident
charm_security_suite --report AUTH_FAILURE:"Invalid password for admin"

# Generate an SBOM
charm_security_suite --sbom sbom.json
```

The CLI initializes the security suite automatically and shuts it down on
exit. Multiple options can be combined in a single invocation.
