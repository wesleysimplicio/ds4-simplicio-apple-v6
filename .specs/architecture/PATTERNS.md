# Patterns - US4 V6 Apple Edition

## 1. Naming

| Item | Convention |
|---|---|
| classes and structs | `PascalCase` |
| methods and functions | `camelCase` |
| constants | `kSnakeCase` |
| files | `snake_case.{h,cpp,mm}` |
| directories | `snake_case/` |

Examples:

- `RuntimeContext`
- `selectRuntimeMode`
- `kDefaultKvPageSize`
- `hardware_probe.h`
- `metal_device.mm`

## 2. Repo structure

Use the runtime tree, not a generic web stack.

```text
runtime/
  core/
  adapters/
  memory/
  kv/
  cache/
  moe/
  metal/
  mlx/
  neon/
  ane/
  speculative/
  tuning/
  telemetry/
  benchmarks/
tests/
  unit/
  integration/
  e2e/
```

## 3. Ownership and boundaries

- `core/` owns orchestration, not family-specific behavior.
- `adapters/` own model-family differences, not global scheduling policy.
- `metal/`, `mlx/`, `neon/`, `ane/` own execution details, not product semantics.
- `telemetry/` and correctness fixtures must stay readable from every backend path.

## 4. C++ rules

- `#pragma once` in headers.
- `std::unique_ptr` by default.
- `std::shared_ptr` only with true shared lifetime.
- No exceptions across adapter or backend ABI boundaries.
- Prefer `std::expected<T, Error>` style results where failure is part of the contract.
- Heavy tensor containers are move-first; borrowed views must be explicit.

## 5. Apple boundary rules

- `.mm` files are only for Objective-C++ or Apple framework boundaries.
- Keep MLX/Metal/CoreML boundary code thin and isolated.
- Do not leak Apple-specific types deep into runtime core interfaces.

## 6. Backend rules

- MLX is the primary path on Apple Silicon.
- Metal is used where benchmarks show a meaningful gap.
- NEON / Accelerate is the fallback path and may also own small low-memory hot loops.
- ANE is opt-in and limited to explicitly validated lightweight paths.
- Every optimization must be disableable.

## 7. Correctness rules

- Any new backend path needs a correctness fixture.
- Drift thresholds must be stated in the task or benchmark.
- If a path exceeds tolerance, fallback must be automatic and observable.
- Do not merge an optimization whose only proof is throughput.

## 8. How to add an adapter

1. Add the family directory under `runtime/adapters/`.
2. Implement the shared adapter contract.
3. Declare capability flags honestly.
4. Provide quant strategy and memory plan per runtime mode.
5. Add unit tests for capability reporting and plan generation.
6. Add correctness coverage for at least one baseline prompt.

## 9. How to add a backend op

1. Start with the safe reference path.
2. Add the accelerated implementation in its backend folder.
3. Wire backend selection behind capability checks.
4. Add benchmark coverage.
5. Add correctness coverage against the safer path.
6. Ensure the path can be disabled.

## 10. How to add a benchmark

- Name it for the thing being measured, not the technology alone.
- Record hardware profile, model profile, runtime mode, and backend.
- Emit enough metadata to compare runs later.
- Benchmark docs must not turn target numbers into marketing claims.

## 11. Logging and telemetry

- Prefer structured logs and stable metric names.
- Always include hardware profile, adapter, backend, and runtime mode where relevant.
- Never hide fallback events.
- Do not log whole prompts or model blobs by default.

## 12. Tests

### Unit

- deterministic and fast;
- adapter capability flags;
- mode selection;
- memory-plan derivation;
- backend selection logic.

### Integration

- probe + mode + adapter selection;
- backend dispatch and fallback;
- KV tier migration.

### E2E

- CLI version and probe flows;
- first run path for planned runtime commands;
- evidence in `playwright-report/` and `test-results/`.

## 13. Things to avoid

- generic abstractions before the first dense vertical slice exists;
- silent performance knobs;
- copying large tensors implicitly;
- backend-specific hacks in `core/`;
- web-stack terminology in runtime docs.
