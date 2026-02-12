# hft-concurrency-playground

A compact C++20 playground for practical concurrency patterns used in low-latency systems.

## Contents

- Lock-free SPSC ring buffer (`include/hft/spsc_ring_buffer.hpp`)
- Cacheline alignment helpers (`include/hft/cacheline.hpp`)
- Lightweight latency histogram (`include/hft/histogram.hpp`)
- Benchmarks:
  - `bench_spsc` (lock-free SPSC)
  - `bench_mutex_queue` (mutex + condition variable queue)
- Demos:
  - `false_sharing_demo`
  - `memory_order_demo`
- Unit test:
  - `test_spsc`

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run

```bash
./build/bench_spsc
./build/bench_mutex_queue
./build/false_sharing_demo
./build/memory_order_demo
./build/test_spsc
```

On Windows, executables are in `build\Release\` for multi-config generators.

## Notes

- `spsc_ring_buffer` stores `uint64_t` for predictable benchmark behavior.
- Capacity is rounded up to the next power of two for bitmask indexing.
- SPSC correctness depends on one producer thread and one consumer thread only.
