# Starter to Runtime Transition

This note explains how the current scaffold should evolve into the real Apple
runtime without rewriting history or pretending missing pieces already exist.

## Current baseline

Today the repository has both layers at once:

- the Node-based starter/bootstrap layer still exists at repo root;
- the C++ runtime scaffold now exists under `runtime/`, `apps/`, and `tests/`.

That means the transition is additive for now, not a big-bang replacement.

## Safe evolution path

1. Keep the root build healthy.
   `CMakeLists.txt`, `runtime/CMakeLists.txt`, and `apps/CMakeLists.txt` are
   already the canonical build entrypoints for runtime work.

2. Replace placeholders by vertical slices.
   New code should land as thin, testable slices inside the existing folders:
   `core/`, then `adapters/`, then backend directories such as `mlx/` and
   `metal/`.

3. Grow the CLI from smoke to product.
   `us4-cli` should expand from `--version`, `--probe`, and `--mode` into
   runtime flows like `run`, `serve`, `bench`, and `tune` only when backed by
   real contracts and tests.

4. Tighten evidence as capabilities appear.
   Smoke tests are enough for the current scaffold. Correctness fixtures,
   backend regression, and benchmark gating become mandatory only when the
   runtime can actually execute inference paths.

## What should not happen

- Do not remove the starter layer before runtime workflows replace its role.
- Do not collapse backend-specific code into `core/`.
- Do not present placeholder benchmarks as performance claims.
- Do not document adapter/backend support ahead of implementation.

## Practical reading order for runtime contributors

1. `runtime/README.md`
2. `.specs/architecture/PATTERNS.md`
3. relevant Sprint 01 task files
4. `apps/cli/main.cpp`
5. `tests/unit/` smoke coverage

## Exit condition for this transition note

This file can disappear once the repository no longer needs "starter vs runtime"
explanations because the runtime is the undisputed primary layer and the old
bootstrap coexistence is gone.
