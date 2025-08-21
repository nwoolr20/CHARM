# CHARM Versioning and API Stability Policy

## Semantic Versioning

CHARM follows [Semantic Versioning 2.0.0](https://semver.org/) with cryptographic considerations.

### Version Format: MAJOR.MINOR.PATCH

- **MAJOR**: Incompatible API changes or algorithm modifications
- **MINOR**: Backward-compatible functionality additions  
- **PATCH**: Backward-compatible bug fixes and optimizations

## API Stability Guarantees

### Stable APIs ✅

#### Core Algorithm (include/charm.h)
```c
// FROZEN: These APIs will not change in backwards-incompatible ways
int charm_init(charm_ctx_t* ctx, const charm_params_t* params, ...);
int charm_update(charm_ctx_t* ctx, const void* data, size_t len);
int charm_final(charm_ctx_t* ctx, uint8_t* out);
int charm_hash(charm_variant_t variant, const void* data, size_t len, uint8_t* out);
```

**Stability Promise**: The core hashing API is frozen and will maintain binary compatibility within major versions.

#### CHARM-B Core (CHARM-B/include/charmb.h)
```c
// STABLE: Core CHARM-B functions are stable within major versions
charmb_status_t charmb_hash(const uint8_t* data, size_t data_len, 
                           uint8_t* digest, charmb_digest_size_t size);
charmb_status_t charmb_hash_8b(const uint8_t data[8], uint8_t digest[32], 
                              charmb_digest_size_t size);
```

### Evolving APIs ⚠️

#### CHARM-B AEAD (CHARM-B/AEAD/include/charmb_aead.h)
```c
// EVOLVING: May change with minor version updates
charmb_aead_status_t charmb_aead_encrypt(...);
charmb_aead_status_t charmb_aead_decrypt(...);
```

**Current Status**: AEAD APIs may evolve based on security review feedback and performance optimization needs.

#### Crypto Suite (src/crypto/*)
```c
// EXPERIMENTAL: Subject to significant change
// APIs under active development
```

**Current Status**: All crypto suite APIs are experimental and may change significantly.

### Deprecated APIs ❌

Currently no deprecated APIs. When deprecation occurs:
- **Notice Period**: Minimum 6 months before removal
- **Migration Guide**: Clear upgrade path provided
- **Compile Warnings**: Deprecated functions will emit warnings

## Version History

### Current: 1.0.x Series
- **Status**: Experimental/Research
- **Algorithm**: CHARM hash function frozen
- **CHARM-B**: Core functionality stable, AEAD evolving
- **Security**: Not production-ready

### Future: 2.0.x Series (Planned)
- **Target**: Extended crypto suite foundation
- **New**: Unified AEAD API, KDF/MAC implementations
- **Breaking**: Possible AEAD API changes based on security review

### Future: 3.0.x Series (Long-term)
- **Target**: Production-ready candidate
- **New**: Full asymmetric crypto support, hardware acceleration
- **Security**: Third-party reviewed, potential standards submission

## Compatibility Guidelines

### Binary Compatibility
- **Patch versions**: Full binary compatibility guaranteed
- **Minor versions**: Source compatibility guaranteed, binary compatibility best effort
- **Major versions**: No compatibility guarantees

### Algorithm Compatibility  
- **Core CHARM**: Frozen specification, identical output guaranteed
- **CHARM-B**: Small optimizations possible, major changes trigger version bump
- **AEAD**: Construction may evolve, old versions will be supported during transition

### Platform Compatibility
Supported platforms (with CI testing):
- **Primary**: Linux x86_64, macOS x86_64/ARM64
- **Secondary**: Windows x86_64 (MinGW), Linux ARM64
- **Planned**: FreeBSD, additional embedded platforms

## Development Process

### Release Cycle
- **Patch releases**: As needed for critical fixes
- **Minor releases**: Every 3-6 months with new features
- **Major releases**: Annually or when significant changes accumulate

### Security Updates
- **Critical vulnerabilities**: Emergency patch release within 48 hours
- **Important vulnerabilities**: Patch release within 2 weeks  
- **Low-impact issues**: Next scheduled minor release

### Testing Requirements
All releases must pass:
- [ ] Full conformance test suite (KAT, Monte Carlo, streaming)
- [ ] Cross-platform build verification
- [ ] Performance regression testing
- [ ] Memory safety analysis (AddressSanitizer, Valgrind)
- [ ] API compatibility verification

## Migration Support

### Version Upgrade Path
1. **Review CHANGELOG.md** for breaking changes
2. **Update include paths** if necessary
3. **Recompile and test** with new version
4. **Update algorithm usage** if API changed

### Long-term Support (LTS)
- **Policy**: One LTS version per major series
- **Duration**: 2 years of security updates
- **Criteria**: Stable API, production testing, security review

Current LTS candidates: None (experimental phase)

## Future Considerations

### Standards Submission
When CHARM matures for standards consideration:
- Algorithm specification will be frozen permanently
- Reference implementation will maintain eternal compatibility
- Test vectors will be locked and published

### Hardware Acceleration
Future hardware acceleration (GPU, FPGA, ASIC) will:
- Maintain identical algorithm output
- Provide performance-optimized implementations
- Keep software fallback compatibility

---

**Document Version**: 1.0  
**Effective Date**: 2024-08-21  
**Next Review**: 2024-11-21

For questions about API stability or version compatibility, please open an issue in the repository.