# CHARM Security Policy

## Security Status

**⚠️ EXPERIMENTAL SOFTWARE - NOT FOR PRODUCTION USE WITHOUT THOROUGH REVIEW**

CHARM is an experimental, entropy-native cryptographic framework under active research and development. While it demonstrates promising performance characteristics, it has not undergone the extensive cryptanalysis required for production deployment.

## Supported Versions

| Version | Status | Security Support |
|---------|--------|------------------|
| 1.0.x   | Experimental | Research only |
| CHARM-B | Active Development | Research only |

## Security Scope and Claims

### What CHARM Provides
- **Deterministic Algorithm**: Reference implementation provides bit-for-bit reproducible results
- **Streaming Invariance**: Single-shot vs incremental processing produces identical outputs  
- **SIMD Acceleration**: Performance optimizations with AVX2/NEON support
- **Small Input Optimization**: CHARM-B provides exceptional performance for ≤64 byte payloads
- **AEAD Construction**: Authenticated encryption with optimized encrypt-then-MAC design

### What CHARM Does NOT Claim
- **Production Security**: No formal security proofs or third-party cryptanalysis
- **Constant-Time Guarantees**: Implementation may contain data-dependent branches
- **Side-Channel Resistance**: Not specifically hardened against timing/cache attacks  
- **Quantum Resistance**: While entropy-native, no formal post-quantum security analysis
- **Compliance Certification**: No FIPS 140-2/3, Common Criteria, or other certifications

## Reporting Security Vulnerabilities

### Scope
We welcome security research on:
- Cryptographic algorithm design flaws
- Implementation vulnerabilities
- Side-channel attack vectors
- Entropy collection weaknesses
- Protocol design issues in AEAD/authentication

### How to Report
1. **Email**: Security issues should be reported privately to: [MAINTAINER_EMAIL_NEEDED]
2. **Timeframe**: We will acknowledge reports within 48 hours
3. **Disclosure**: Coordinated disclosure preferred - please allow 90 days for fixes

### What to Include
- Detailed description of the vulnerability
- Steps to reproduce (with test vectors if applicable)
- Potential impact assessment
- Suggested mitigations (if any)

## Current Security Considerations

### Algorithm Design
- **Entropy-Native**: CAEDS/CEE/ECE design is novel and lacks extensive cryptanalysis
- **Domain Separation**: AEAD modes use domain separation but need formal verification
- **Key Derivation**: CHARM-B AEAD uses simplified KDF - needs security analysis

### Implementation Security
- **Memory Safety**: C implementation requires careful memory management
- **Entropy Collection**: System entropy sources use non-cryptographic conditioning
- **Timing Attacks**: SIMD paths may introduce timing variations
- **Fault Injection**: No specific hardening against fault attacks

### Known Limitations
1. **CHARM-B AEAD**: Optimized for performance over conservative security margins
2. **Entropy Bus**: Diagnostic entropy collection not suitable for security purposes
3. **Side Channels**: AVX2/SIMD optimizations may leak information via cache/timing
4. **Key Management**: No secure key storage or hardware security module integration

## Development Security Practices

### Code Review
- All cryptographic code requires multiple reviews
- Security-critical changes require maintainer approval
- External security reviews welcomed and credited

### Testing
- Extensive conformance testing (KAT, Monte Carlo, streaming)
- Differential testing against reference implementations
- Fuzzing of parsers and AEAD APIs (planned)

### Dependencies
- Minimal external dependencies for core algorithm
- Optional dependencies clearly documented
- Third-party libraries isolated in third_party/

## Deployment Recommendations

### Research Use ✅
- Academic cryptographic research
- Performance benchmarking
- Algorithm comparison studies
- Educational purposes

### Production Use ❌ (Not Recommended)
- Mission-critical applications
- Financial systems  
- Medical devices
- Any system requiring formal security certification

### Evaluation Use ⚠️ (With Caution)
- Proof-of-concept implementations
- Non-security-critical applications
- Performance testing environments
- Development/testing systems

## Future Security Roadmap

### Short Term (Next Release)
- [ ] Formal algorithm specification document
- [ ] Known Answer Test (KAT) vectors for independent validation
- [ ] Constant-time implementation audit
- [ ] Memory safety analysis (Valgrind, AddressSanitizer)

### Medium Term
- [ ] Third-party cryptanalysis engagement
- [ ] Side-channel resistance evaluation
- [ ] Formal security model documentation
- [ ] Hardware acceleration security review

### Long Term  
- [ ] Standards body submission (if warranted)
- [ ] Formal verification of critical components
- [ ] Hardware security module integration
- [ ] Compliance certification pursuit

## Security Research Acknowledgments

We gratefully acknowledge security researchers who help improve CHARM's security posture. Responsible disclosure will be credited in release notes and security advisories.

---

**Last Updated**: 2024-08-21  
**Version**: 1.0  
**Contact**: [Repository Issues](https://github.com/nwoolr20/CHARM/issues) (for non-security bugs)