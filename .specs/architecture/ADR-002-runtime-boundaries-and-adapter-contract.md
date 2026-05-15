# ADR-002 - Runtime Boundaries and Adapter Contract

## Status

Accepted

## Context

US4 V6 must support multiple model families without duplicating the scheduler, probe, memory policy, or correctness machinery. The repo also needs docs that stop agents from treating this as a generic app stack.

## Decision

The runtime is divided into these stable boundaries:

- Interface
- Runtime Core
- Model Adapters
- Execution Backends
- Memory System
- Validation and Telemetry

All model-family differences enter through the adapter contract. Backends execute work but do not define product policy.

## Consequences

- Architecture docs describe runtime-specific boundaries only.
- New adapters must declare capabilities, memory plans, and quant strategy through a shared contract.
- Core logic remains reusable across dense, MoE, and low-memory families.
