# EquiTick

A production-grade engine for real-time pay equity enforcement.

## Tech Specs

### 1. Low-Latency Data Path
- **Lock-Free MPSC Ring Buffer**: Multi-producer, single-consumer queue for $O(1)$ event passing without kernel locks.
- **Zero-Allocation Hot Path**: Uses pre-allocated memory pools and cache-aligned structs (`alignas(64)`) to minimize cache misses and TLB pressure.
- **Zero-Copy Serialization**: Uses POD (Plain Old Data) structs and memory mapping for instant access to feed events.

### 2. Computational Efficiency
- **SIMD Kernels (AVX2)**: Batch processing of salary normalization and statistics using vector instructions.
- **Online Regression**: Incremental OLS updates in $O(1)$ time, avoiding matrix inversions during the hot path.
- **Welford’s Algorithm**: Numerically stable rolling variance and Z-score calculation.

### 3. Hardware-Level Optimization
- **Core Pinning (NUMA Awareness)**: Dedicated worker threads pinned to physical cores to eliminate context-switch jitter.
- **Cache-line Padding**: Explicitly designed to prevent False Sharing between producers and consumers.
- **Branch Prediction**: Optimized logic paths using `[[likely]]` / `[[unlikely]]` and minimal branching.

## UI / Monitoring
- **Real-time Dashboard**: Built with Vite + React + Tailwind + Chart.js.
- **Telemetry**: Sub-millisecond visualization of throughput (500k+ events/sec) and end-to-end latency.

## Getting Started
1. Build C++: `mkdir build && cd build && cmake .. && make`
2. Launch UI: `cd dashboard && npm install && npm run dev`
