# WORKFLOW - US4 V6 Apple Edition

## 1. Current phase

This repository is currently in the **planning/bootstrap phase**.

That means:

- starter tooling in Node is real and executable now;
- runtime Apple docs are normative for the product direction;
- the C++ runtime scaffolding begins in Sprint 01.

Do not write workflow text that pretends `runtime/`, `CMakeLists.txt`, or release automation already exist unless the same section clearly marks them as planned.

## 2. Branch strategy

- Long-lived branch: `main`
- Working branches: `feat/<task-id>-<slug>`, `fix/<task-id>-<slug>`, `docs/<slug>`, `chore/<slug>`
- If the sprint prefix matters, include it in the task id itself, for example `feat/t01.3-hardware-probe`

One branch, one purpose.

## 3. Task-first execution

Non-trivial work starts from:

- current sprint objective in `SPRINT.md`
- one concrete `*.task.md`
- relevant product and architecture docs

Rule:

- sprint docs define the train;
- task docs define executable work;
- ADRs define irreversible decisions.

## 4. Validation by maturity

The project has different quality gates depending on what exists.

### Planning/bootstrap phase

- doc consistency
- starter self-tests
- placeholder cleanup
- sprint/task completeness

### Early runtime phase

- build
- format/lint
- unit tests
- CLI smoke E2E

### Inference-capable phase

- correctness fixtures
- regression suites
- benchmark evidence
- release gating

Do not require real logit-diff benchmarking before a runnable inference baseline exists.

## 5. CI reality today

Today the repo CI validates the **starter/bootstrap layer**.

- `.github/workflows/ci.yml`
- `.github/workflows/dod.yml`
- `.github/workflows/scaffold-self-check.yml`

Those workflows do not yet validate a C++ runtime. They validate the starter package and planning hygiene.

## 6. Planned CI transition

Sprint 01 and Sprint 02 are expected to introduce the runtime-facing pipeline:

- CMake configure/build
- clang-format
- clang-tidy
- CTest
- Playwright CLI smoke for `us4-cli`

Production release automation remains planned until the runtime build exists.

## 7. Pull requests

Every PR should state:

- what phase it belongs to: `starter/bootstrap`, `planning`, or `runtime`;
- linked task;
- validation run;
- risks and follow-ups if any.

If architecture docs change, link the relevant ADR in the PR body.
