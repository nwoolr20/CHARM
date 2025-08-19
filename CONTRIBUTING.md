# Contributing to CHARM

Thank you for your interest in contributing to CHARM! This document outlines how contributors can participate effectively while maintaining the integrity, performance, and security of the system.

## 1. Scope of Contributions

Contributors should focus exclusively on producing and validating real-world test results. All contributions must be supported by empirical evidence demonstrating measurable improvements in:

- **Performance** (throughput, latency, efficiency)
- **Stability** and consistency across varying input sizes
- **Cryptographic integrity** and resistance
- **Security properties**, including quantum-resistance verification

**Important**: Unverified optimizations, speculative code, or theoretical changes without testing are not permitted.

## 2. Code Style and Structure

- Follow the existing repository structure:

```
/src
    /core         # Core modules (CAEDS, CEE, ECE)
    /utils        # Utility functions
    /tests        # Unit and integration tests
/docs             # Documentation
/benchmarks       # Benchmark scripts and results
```

- Use clear, descriptive names for functions, variables, and modules.
- Include comprehensive comments for complex logic, especially entropy-handling operations.
- Keep commits small and focused, with meaningful commit messages describing the tested improvement.

## 3. Testing Requirements

- **All contributions must include benchmark data** demonstrating real-world performance or security effects.
- Use the provided benchmark suite for consistency.
- Include before-and-after comparisons for any performance improvements.
- Security-related contributions must include validation that no vulnerabilities were introduced.

## 4. Pull Request Guidelines

- PRs should reference specific test results, issues, or benchmark outcomes.
- Include detailed descriptions of what was changed, why, and how the change affects performance, stability, or security.
- Ensure all tests pass successfully before submission.
- Tag PRs clearly (e.g., `performance`, `security`, `stability`) to assist reviewers.

## 5. Review and Approval

- Contributions will be reviewed for empirical evidence, adherence to code style, and consistency with CHARM's modular architecture.
- **Only contributions with verified, reproducible test results will be merged.**

## 6. Best Practices

- Prioritize controlled experiments over exploratory changes.
- Document the environment and conditions under which benchmarks were performed.
- Avoid changes that compromise quantum resistance, entropy management, or stability for short-term performance gains.

## 7. License Header

All source files must include the following license header:

```c
/*
 * CHARM – High-Performance Entropy-Native Cryptographic Framework
 * Copyright (c) 2025 Nicholas Woolridge & NOCTRL™ (Nô)
 *
 * This software is licensed under the CHARM License 2025.
 * Use, modification, and distribution are permitted only with
 * verified, real-world test results demonstrating correct
 * functionality, performance, and security.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 * See LICENSE in the repository root for full license details.
 */
```

## 8. Development Workflow

1. **Fork** the repository and create a feature branch
2. **Implement** your changes with comprehensive testing
3. **Benchmark** your changes against the baseline
4. **Document** your results and methodology
5. **Submit** a pull request with test data and analysis
6. **Respond** to review feedback with additional data if requested

---

By following these guidelines, contributors ensure that CHARM remains a reliable, high-performance, and secure cryptographic framework that "works like a charm."