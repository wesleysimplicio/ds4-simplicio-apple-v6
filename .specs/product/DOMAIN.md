# Domain - US4 V6 Apple Edition

## Core concepts

- **Adapter**: family-specific implementation behind the shared runtime contract.
- **Backend**: execution path used for a given op or phase, such as MLX, Metal, NEON, scalar CPU, or optional ANE.
- **Runtime mode**: memory and performance policy chosen from `FULL`, `BALANCED_PLUS`, `DEGRADED`, `ULTRA_LOW`, `MICRO`, `MICRO_PLUS`, `NANO`.
- **Prefill**: prompt ingestion phase that builds attention state before decode.
- **Decode loop**: token-by-token generation phase after prefill.
- **KV cache**: attention state reused across decode steps.
- **Hot/Warm/Cold/Summary KV**: tiered lifecycle for KV state across unified memory, compressed memory, SSD, and summarized state.
- **Continuous batching**: scheduling decode steps from multiple sessions together.
- **Expert pager**: on-demand MoE expert loading and eviction manager.
- **Speculative expert prefetch**: predicting experts likely to be needed soon and loading them early.
- **Correctness drift**: numeric or token-output deviation versus a safer reference path.

## Primary entities

- `RuntimeContext`
- `RuntimeSession`
- `HardwareProbe`
- `RuntimeModeSelector`
- `AdapterRegistry`
- `IUS4V6Adapter`
- `BackendSelector`
- `BackendExecutor`
- `KvPager`
- `PrefixCache`
- `SessionSummarizer`
- `ExpertPager`
- `ContinuousBatcher`
- `CorrectnessGuard`
- `TelemetrySink`

## Runtime invariants

- Greedy decoding with fixed seed must remain deterministic for the same backend and adapter configuration.
- Any optimization path must be disableable.
- Any backend path that exceeds drift tolerance must fall back to a safer path automatically.
- Session isolation is mandatory: no cross-session KV contamination.
- Tier migration must preserve semantic equivalence of the session state.

## Backend selection rules

Preferred order:

1. `MLX`
2. `Metal`
3. `Metal + NEON`
4. `NEON`
5. scalar CPU

ANE is opt-in and only used for explicitly supported lightweight paths.

## Compatibility matrix

| Memory tier | Main use |
|---|---|
| 128 GB | frontier dense and MoE, long context, multi-session |
| 96 GB | broad high-end coverage with fewer constraints |
| 64 GB | practical dense and selective MoE |
| 48 GB | degraded frontier experimentation |
| 32 GB | small and medium dense, strong BitNet/Ternary |
| 24 GB | 1B to 4B dense and low-memory adapters |
| 16 GB | `NANO` mode, smallest dense plus low-memory adapters |

## Terms to avoid

- "fast" without hardware, model, and metric.
- "auto" without saying what is automatic.
- "offload" without naming the destination tier or device.
- "supported" when the real meaning is "planned."
