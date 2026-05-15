# Vision - US4 V6 Apple Edition

## Problem

Local LLM inference on Apple Silicon is fragmented.

- Dense, MoE, and ternary model families need different runtime behavior.
- Existing tools usually optimize one family or one workflow at a time.
- Apple-specific capabilities such as MLX, unified memory, Metal, NEON, and ANE are often underused or exposed as low-level tuning work for the user.

US4 V6 Apple Edition exists to provide one runtime core with specialized adapters and hardware-aware execution on real Macs.

## Who it is for

- ML engineers benchmarking local models on MacBook and Mac Studio hardware.
- Researchers comparing dense, MoE, and ternary architectures with a shared correctness framework.
- App developers embedding a privacy-first local runtime into Apple apps and tools.
- Platform engineers operating local inference across managed Mac fleets.

## Product thesis

The winning local runtime on Apple Silicon will not be the most generic one. It will be the one that:

- understands model architecture differences;
- understands Apple hardware tiers;
- treats correctness as a hard gate;
- and can degrade gracefully across memory pressure instead of failing abruptly.

## Differentiators

- Universal runtime core with specialized adapters per model family.
- MLX-first execution path, with Metal kernels only where measured gaps justify them.
- Multi-tier KV lifecycle: hot, warm, cold, and summary.
- MoE-aware expert paging and speculative expert prefetch.
- Continuous batching for multiple local sessions.
- Optional ANE offload on supported hardware for selected lightweight paths.
- One CLI and one runtime contract across all supported families.

## Success metrics

These are **targets to validate**, not benchmark claims.

- Correctness drift remains within task-specific tolerance before any optimization is enabled by default.
- Dense and MoE adapters share a stable runtime contract by Sprint 08.
- Runtime mode auto-selection covers 16 GB, 24 GB, 32 GB, 48 GB, 64 GB, 96 GB, and 128 GB memory tiers.
- Apple-specific execution paths are measurable independently: MLX time, Metal time, NEON time, ANE time when available.
- Public prerelease quality is reached by Sprint 07 alpha and Sprint 10 beta without breaking deterministic greedy decoding.

## Runtime modes

Canonical runtime modes:

- `FULL`
- `BALANCED_PLUS`
- `DEGRADED`
- `ULTRA_LOW`
- `MICRO`
- `MICRO_PLUS`
- `NANO`

Default RAM mapping:

| Unified memory | Default mode |
|---|---|
| 128 GB | `FULL` |
| 96 GB | `BALANCED_PLUS` |
| 64 GB | `DEGRADED` |
| 48 GB | `ULTRA_LOW` |
| 32 GB | `MICRO` |
| 24 GB | `MICRO_PLUS` |
| 16 GB | `NANO` |

## Non-goals

- training and fine-tuning;
- cloud or distributed inference;
- non-Apple hardware in this edition;
- claiming universal speed wins without benchmark and correctness evidence;
- GUI-first product surface before CLI and library are stable.
