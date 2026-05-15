# TIMELINE - US4 V6 Apple Edition

## Roadmap window

- Start: 2026-05-14
- End target: 2026-10-28
- Cadence: 12 sprints of 2 weeks

## Sprint windows

| Sprint | Window | Theme |
|---|---|---|
| S01 | 2026-05-14 -> 2026-05-27 | Foundations and Skeleton |
| S02 | 2026-05-28 -> 2026-06-10 | CPU Scalar Baseline |
| S03 | 2026-06-11 -> 2026-06-24 | MLX and Metal Skeleton |
| S04 | 2026-06-25 -> 2026-07-08 | NEON Hot Paths |
| S05 | 2026-07-09 -> 2026-07-22 | BitNet and Ternary |
| S06 | 2026-07-23 -> 2026-08-05 | KV Memory Architecture |
| S07 | 2026-08-06 -> 2026-08-19 | Llama Adapter |
| S08 | 2026-08-20 -> 2026-09-02 | MoE Foundation |
| S09 | 2026-09-03 -> 2026-09-16 | MoE Advanced |
| S10 | 2026-09-17 -> 2026-09-30 | Continuous Batching and Speculative Decoding |
| S11 | 2026-10-01 -> 2026-10-14 | ANE M5+ Offload |
| S12 | 2026-10-15 -> 2026-10-28 | Auto-Tune and Release |

## Practical dependency graph

- `S01 -> S02`: runtime skeleton, CLI contract, probe, and mode selection first
- `S02 -> S03`: dense baseline before accelerated backends
- `S02 -> S04`: NEON can evolve from CPU baseline; it does not depend on Metal
- `S03 -> S06`: unified-memory and MLX shape KV planning
- `S06 -> S08 -> S09 -> S10`: MoE and batching/speculative stack
- `S02/S06 -> S07`: Llama depends on dense baseline and KV discipline, not BitNet
- `S07 -> S11`: ANE path depends on a stable dense adapter surface
- `S10 + S11 -> S12`: release hardening comes after broad capability coverage

## Release milestones

- `v0.1.0-alpha`: internal Sprint 01 artifact quality, not public runtime GA
- `v0.7.0-alpha`: earliest realistic public alpha target
- `v0.10.0-beta`: broader external beta target
- `v0.11.0-rc.1`: release candidate
- `v1.0.0`: GA target

These remain planned milestones, not current repo capabilities.
