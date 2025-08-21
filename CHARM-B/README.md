# CHARM-B (CHARMbit): Ultra-Small Input Optimized Hash

CHARM-B is a specialized variant of CHARM optimized exclusively for ultra-small inputs (≤64B) where every byte and cycle matters.

## Performance Targets

| Input Size | CHARM-B Target | vs SHA-256 | vs Regular CHARM |
|------------|----------------|------------|------------------|
| 8B         | >2,000,000 MB/s | +300%     | +1000%+         |
| 16B        | >6,000,000 MB/s | +300%     | +1000%+         |
| 32B        | >13,000,000 MB/s| +300%     | +1000%+         |
| 64B        | >24,000,000 MB/s| +600%     | +1500%+         |

## Key Innovations

### 1. Bit-Level Optimization Engine
- Specialized bit-level mixing for maximum entropy diffusion
- SIMD-accelerated processing with AVX2 support
- Direct 64-bit operations for 8-byte inputs

### 2. Size-Specific Algorithm Dispatch
- Automatic optimization based on exact input size
- Ultra-optimized specialized functions for each target size
- Zero branching overhead

### 3. Zero-Overhead Architecture
- Thread-local static state eliminates malloc/free costs
- Single-function calls with no context initialization
- Minimal CPU cycles for maximum throughput

### 4. Dual Digest Modes
- 256-bit output for full compatibility
- 128-bit output for micro-hashing applications

## Components

- **Core Entropy Engine**: Ultra-fast entropy processing for small inputs
- **CHARM-B Algorithm**: Bit-optimized hash algorithm
- **AEAD/AEAS**: Authenticated encryption using CHARM-B
- **Comprehensive Testing**: Extensive benchmarks and real-world tests

## Build Instructions

```bash
cd CHARM-B
make all                    # Build all components
make test                   # Run comprehensive tests
make benchmark              # Run performance benchmarks
```

## Real-World Applications

- Blockchain & Crypto: Transaction IDs, wallet addresses, block headers
- Microservices: High-frequency small message processing, API tokens
- IoT & Embedded: Sensor data integrity, device authentication
- Database Systems: Small key hashing, cache tags, index optimization