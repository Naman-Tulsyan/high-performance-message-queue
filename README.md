# HMQ — High Performance Message Queue

HMQ is a message broker built from first principles in C++20.

## Project goals

- Understand messaging, networking, concurrency, and storage internals.
- Build a persistent, partitioned message broker.
- Implement a custom binary protocol over TCP.
- Establish correctness before optimizing performance.
- Measure throughput, latency, resource usage, and recovery behavior.

## Current status

Initial development: build system and test scaffolding.

## Requirements

- C++20 compiler
- CMake 3.20 or newer
- Git
- Docker with Linux container support

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

ctest --test-dir build --output-on-failure

./build/hmq
```
