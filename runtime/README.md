# Runtime Scaffold

This directory hosts the early C++ runtime scaffold for US4 V6 Apple Edition.

It is no longer just a planned placeholder: the repo already contains a
buildable skeleton for Sprint 01, while the real inference runtime is still
ahead of us.

## What exists now

- root `CMakeLists.txt` configures the runtime build
- `runtime/CMakeLists.txt` builds `us4_runtime_core`
- `apps/CMakeLists.txt` builds the `us4-cli` smoke executable
- `core/` contains compileable contracts for hardware probe, runtime mode, and
  runtime context
- `telemetry/` contains minimal sink/types placeholders used by smoke tests
- `benchmarks/dense_baseline.cpp` is a benchmark placeholder, not an inference
  measurement
- `tests/unit/` contains Sprint 01 contract smoke coverage

## What the scaffold already proves

- the runtime tree layout is real
- build entrypoints are explicit
- `us4-cli` already exposes `--version`, `--probe`, and `--mode <value>`
- hardware probe and mode selection contracts compile and run

## What is still missing

- tensor types and execution graph
- real family adapters under `runtime/adapters/`
- MLX bridge and Metal kernels
- NEON / Accelerate fallback hot paths
- inference-capable `run`, `serve`, `bench`, and `tune` CLI flows
- correctness fixtures and backend regression coverage

## Directory intent

| Path | Intent today | Evolves into |
|---|---|---|
| `core/` | stable contracts and orchestration skeleton | runtime orchestration, selection, and shared primitives |
| `adapters/` | placeholders and per-family notes | dense, MoE, and low-memory adapters |
| `mlx/` | reserved primary backend surface | MLX graph/build/eval integration |
| `metal/` | reserved accelerated backend surface | measured hot kernels only |
| `neon/` | reserved CPU fallback surface | scalar/NEON low-memory and safety paths |
| `ane/` | reserved opt-in backend surface | validated M5+ offload paths |
| `memory/`, `kv/`, `cache/`, `moe/`, `speculative/`, `tuning/` | roadmap-aligned placeholders | runtime subsystems landed by later sprints |
| `telemetry/` | smoke-level instrumentation contract | structured runtime metrics and fallback observability |
| `benchmarks/` | placeholder harness | correctness and throughput evidence |

## Build entrypoints

From repo root:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/us4-cli --version
./build/us4-cli --probe
./build/us4-cli --mode auto
```

These commands validate the scaffold and CLI contract. They do not validate
real inference yet.

## Transition rule

During the starter-to-runtime transition, this tree must stay honest about the
current repo state:

- document what already builds and runs;
- mark placeholders as placeholders;
- land real runtime behavior inside the existing ownership boundaries from
  `PATTERNS.md`;
- avoid claiming MLX, Metal, NEON, or adapter support before the code and tests
  exist.

See [STARTER-TO-RUNTIME.md](STARTER-TO-RUNTIME.md) for the short migration map.
