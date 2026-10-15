Module 0 — Engineering foundation
2–3 days
Before implementing the broker
- Learn the repository structure, CMake, compiler flags, Git, and basic Linux development workflow.
- Set up C++20, formatting, warnings, unit tests, sanitizers, and continuous integration.
- Establish rules for commits, branches, issues, and design documentation.
Deliverable: a clean, buildable project with a passing test suite and a repeatable development workflow.




Module 1 — Messaging fundamentals and guarantees
3–4 days
- Understand queues versus logs, producers, consumers, topics, partitions, and offsets.
- Learn FIFO ordering, at-most-once, at-least-once, and the limits of delivery guarantees.
- Define message format, size limits, error behavior, and offset semantics.
Deliverable: a documented message model, a glossary, and written correctness invariants.




Module 2 — A correct in-memory queue
4–6 days
- Implement a bounded FIFO queue with a mutex and condition variables.
- Learn critical sections, race conditions, producer-consumer synchronization, and shutdown.
- Handle full queues, empty queues, blocking operations, and multiple threads.
Deliverable: a tested queue library with concurrent stress tests. This becomes our correctness baseline.




Module 3 — Topics, partitions, and offsets
3–5 days
- Design the broker's in-memory metadata model.
- Implement topic creation, partition selection, append order, and offset assignment.
- Define ownership and thread-safety rules for each component.
Deliverable: a broker core that manages multiple topics and partitions without networking or disk I/O yet.




Module 4 — TCP networking and wire protocol
5–7 days
- Learn sockets, TCP streams, partial reads/writes, byte order, and framing.
- Define a versioned binary protocol with request IDs, opcodes, length fields, and errors.
- Build the server connection handler and a basic C++ client.
Deliverable: a producer can publish to the broker process and a consumer can fetch messages through TCP.




Module 5 — Persistent append-only log
1–2 weeks
- Learn file I/O, append-only storage, binary serialization, and filesystem behavior.
- Store messages in partition log segments with offsets.
- Build an offset index and implement segment rolling and size limits.
Deliverable: messages survive a normal broker restart, and the broker can locate messages by offset.




Module 6 — Recovery and durability
1–2 weeks
- Distinguish an OS write, a successful write syscall, and data made durable with fsync.
- Recover indexes and offsets after crashes.
- Detect incomplete or corrupt trailing records and define safe truncation/recovery behavior.
- Specify when a producer receives an acknowledgement.
Deliverable: crash-recovery tests and clearly documented durability guarantees.




Module 7 — Consumer groups and acknowledgements
1–2 weeks
- Track committed offsets separately from read positions.
- Assign partitions to consumers within a group.
- Implement explicit acknowledgements and restart/resume behavior.
- Define what happens when consumers disconnect or process messages more than once.
Deliverable: consumers can resume from committed offsets, with documented at-least-once delivery behavior where supported.




Module 8 — Backpressure and runtime concurrency
1–2 weeks
- Prevent unbounded memory growth and slow-consumer overload.
- Introduce a controlled worker model, bounded in-flight requests, and graceful shutdown.
- Establish explicit ownership for connections, storage, and background tasks.
- Measure lock contention before changing synchronization strategies.
Deliverable: a stable multiclient broker that handles overload without uncontrolled resource consumption.




Module 9 — Benchmarking and optimization
1–2 weeks
- Build repeatable producer and consumer load generators.
- Measure throughput, p50/p95/p99 latency, CPU, memory, and disk behavior.
- Profile hot paths and optimize batching, allocations, system calls, and indexing where evidence supports it.
- Compare each optimization against the previous baseline.
Deliverable: reproducible performance reports with before/after results and explanations of bottlenecks.




Module 10 — Production-style hardening
1–2 weeks
- Test malformed packets, disconnects, disk-full conditions, resource limits, and corrupted logs.
- Add metrics, structured logging, configuration validation, and useful error messages.
- Package the broker for reproducible builds and local demos.
- Write architecture documentation, a benchmark report, and interview preparation notes.
Deliverable: a polished repository another engineer can build, run, test, and understand.




Module 11 — Advanced extensions
Optional
- Zero-copy or memory-mapped reads, batching improvements, and alternative I/O strategies.
- Replication and Raft, after the single-node semantics are reliable.
- Retention policies, log compaction, or a more advanced memory allocator if justified.
Deliverable: one or two well-measured advanced features—not a collection of half-finished features.