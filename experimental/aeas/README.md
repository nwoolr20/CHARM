# AEAS Integration for CHARM

## Overview

This implementation integrates Advanced Encryption with Adaptive Streaming (AEAS) techniques into CHARM to optimize performance for small to medium payloads while maintaining full security properties.

## Key Optimizations

### 1. Batched Keystream Generation
- Pre-generate multiple keystream blocks for small payloads (≤128B)
- Reduce per-operation overhead through batch processing
- Cache-friendly memory access patterns

### 2. 64-bit SIMD Operations
- Vectorized XOR operations using SSE2/AVX2
- Process 8 bytes per instruction vs. byte-by-byte
- Significant performance improvement for small data

### 3. Stack-based HMAC Optimization
- Pre-allocated contexts for HMAC operations
- Eliminate malloc/free overhead for small computations
- Stack allocation for contexts ≤256 bytes

### 4. Poly1305 Integration
- RFC 7539 compatible authentication option
- Alternative to CHARM's native HMAC for specific use cases
- Optimized for modern CPU architectures

## Performance Targets

Based on AEAS exploration findings:
- **16-64 bytes**: 100-200 MB/s (5-10x improvement)
- **64-256 bytes**: 200-400 MB/s (10-20x improvement)  
- **256-1024 bytes**: 300-500 MB/s (15-25x improvement)

## Implementation Status

- [x] Architecture designed
- [ ] Batched keystream generation
- [ ] SIMD XOR operations
- [ ] Stack-based HMAC
- [ ] Poly1305 integration
- [ ] Performance benchmarking
- [ ] Security validation

## Files

- `charm_aeas.h` - AEAS-optimized CHARM interface
- `charm_aeas.c` - Core implementation
- `simd_ops.c` - SIMD-optimized operations
- `benchmark.c` - Performance benchmarking
- `tests.c` - Security and correctness testing