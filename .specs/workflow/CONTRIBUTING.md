# CONTRIBUTING — `US4 V6 Apple Edition`

Guia step-by-step para adicionar feature, fix ou refactor em **US4 V6 Apple Edition** (`us4-v6-simplicio-apple`). Funciona pra humano e pra agent. Stack: C++17/20 + CMake + MLX + Metal + NEON (Accelerate) + ANE (M5+) + GoogleTest + Playwright + Ralph Loop. Time: `us4-core`.

---

## Pré-requisitos

- macOS 14+ em Apple Silicon (M1..M5+). Xcode 16 Command Line Tools instaladas.
- Repo clonado, build verde local:
  ```bash
  cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build build -j$(sysctl -n hw.ncpu)
  ctest --test-dir build --output-on-failure
  ```
- Node 20+ pra Playwright (`npx playwright install` já rodado uma vez).
- Leu `AGENTS.md` (raiz), `.specs/architecture/PATTERNS.md` (preenchido incrementalmente) e a `task.md` corrente.
- Acesso pra abrir PR no `wesleysimplicio/us4-v6-simplicio-apple` + rodar CI.

---

## Fluxo padrão (8 passos)

### 1. Criar `task.md` em sprint atual

Toda mudança não-trivial nasce em `task.md`. Caminho:

```
.specs/sprints/sprint-XX/<id>-<short-desc>.task.md
```

Use `task-template.md` como base. Preencha:
- Contexto e problema (qual gap de runtime, adapter, kernel ou DX).
- Acceptance Criteria testáveis (checkboxes — comportamento + métrica).
- Out of scope explícito (especialmente backends e adapters).
- Test plan: unit (GoogleTest), regression (sprints anteriores), correctness diff (logit), e2e Playwright.
- Definition of Done (espelha `AGENTS.md` seção DoD).
- Pegadinhas conhecidas + ADRs relevantes.

Exemplo: `.specs/sprints/sprint-02/T02.2-scalar-gemm.task.md`.

> Mudança trivial (typo, bump patch sem risco, `.clang-format` cosmetic) pode pular task. Qualquer coisa que toque kernel, runtime, correctness, adapter ou CLI exige task.

### 2. Criar branch

A partir de `main` atualizada:

```bash
git checkout main
git pull --rebase origin main
git checkout -b feat/sprint-03-t03.4-metal-gemm
```

Convenção de nome em `WORKFLOW.md` seção 1. Sprint branches usam `feat/sprint-XX-<task-id>-<slug>`.

### 3. Implementar seguindo PATTERNS

- **Não invente padrão novo.** Se `.specs/architecture/PATTERNS.md` define como criar kernel Metal, header com `#pragma once`, smart pointer, tensor ownership, adapter loop — segue.
- Mudou padrão? Abre ADR **antes** (`.specs/architecture/ADR-template.md`) e linka na task.
- Não adiciona dependência (CMake `FetchContent`, vcpkg port, Homebrew formula, MLX subdir) sem confirmar com `us4-core`.
- Edita só o pedido. Sem refactor oportunista — refactor é PR separado em branch `refactor/<slug>`.
- MLX é primary path. Metal kernel só onde MLX não cobre. NEON/Accelerate é fallback. ANE é opt-in (`--ane`).

### 4. Testes (unit + regression + correctness + e2e)

Antes do PR, todo verde local:

```bash
# format + lint
clang-format --dry-run --Werror $(git ls-files '*.cpp' '*.h' '*.mm')
clang-tidy -p build $(git ls-files 'runtime/**/*.cpp')

# unit (GoogleTest via CTest) — coverage do diff >= 80%
ctest --test-dir build --output-on-failure

# regression (suíte de sprints anteriores)
ctest --test-dir build -L regression --output-on-failure

# correctness diff (logit vs referência, tolerância da task)
./build/runtime/benchmarks/correctness_logit_diff --model qwen-0.5b --tokens 32

# bench (se task pede número)
./build/runtime/benchmarks/dense_baseline

# e2e Playwright (OBRIGATÓRIO em toda task que toca CLI/UX)
npx playwright test --reporter=list,html
```

- Bug fix sem teste regressivo é **inaceitável**. Escreve GoogleTest que falha sem o fix.
- E2E tem evidência: `playwright-report/index.html` + `test-results/<spec>/trace.zip` + screenshot + video em retry.
- Coverage caiu? Justifica no PR ou adiciona teste.
- Logit diff fora da tolerância = bloqueia. Sem exceção sem ADR.

### 5. Abrir PR usando template

Push e abre PR via gh:

```bash
git push -u origin feat/sprint-03-t03.4-metal-gemm
gh pr create --fill --web
```

Template em `.github/PULL_REQUEST_TEMPLATE.md` é preenchido automático. Complete:
- Link para `task.md`.
- Resumo (3-5 bullets — *o que* + *por que*).
- Bench numbers (tokens/s cold/warm, RAM peak) se aplicável.
- Correctness diff vs referência (link pro relatório em `runtime/benchmarks/correctness/`).
- Evidência E2E (link pro report Playwright + `trace.zip`).
- Checklist DoD marcada.
- Riscos e plano de rollback (especialmente backend novo atrás de `-DUS4_ENABLE_<X>=OFF`).

Title segue Conventional Commits: `feat(runtime): add Metal GEMM kernel for FP16`.

### 6. Review

- CI verde é pré-requisito. PR com vermelho não vai pra review humana.
- Reviewer humano em até 4h úteis. PR de hotfix tem SLA 30min.
- PRs tocando kernel/runtime/segurança exigem 2 reviewers (1 com tag `runtime` ou `security`).
- Endereça todos `req:` antes de pedir re-review.
- Discussão de design vai no diff. Discussão arquitetural ampla vira ADR.

### 7. Merge squash

Após aprovação:

```bash
gh pr merge --squash --delete-branch
```

Mensagem squash = title do PR + corpo enxuto. Histórico de `main` fica linear.

### 8. Distribuição

- Merge em `main` dispara `ci.yml` + `dod.yml`. Verde => artifact (binário + Playwright report) em GitHub Actions.
- Pra cortar release de produção: bump versão, atualiza `CHANGELOG.md`, tag SemVer. Detalhes em `RELEASE.md`.
- Distribuição: tarball assinado + Homebrew tap `wesleysimplicio/us4`.

---

## Skills/Agents que você pode usar

Skills e agents reduzem trabalho repetitivo e enforçam padrão. Ver `.skills/` e `.agents/` no repo.

### Skills disponíveis (em `.skills/`)

| Skill | Quando trigar | Caminho |
|-------|---------------|---------|
| `caveman` | Default da sessão. Output terse. Não afeta código/PR/commit. | `.skills/caveman/SKILL.md` |
| `ralph-loop` | **Obrigatório** em toda task técnica. Loop `read → plan → execute → format → lint → unit → e2e → regression → fix → repeat` até DoD verde. | `.skills/ralph-loop/SKILL.md` |
| `everything-claude-code` | Bundle ~60 agents + ~221 skills. Padrão: máximo de agents ECC em paralelo. | `.skills/everything-claude-code/SKILL.md` |
| `playwright-e2e` | Escrever ou ajustar teste E2E pro CLI `us4-cli`. Fixtures, process wrapper, evidências. | `.skills/playwright-e2e/SKILL.md` |
| `conventional-commits` | Compor mensagem de commit ou title de PR. Cobre `feat`, `fix`, `perf`, breaking change. | `.skills/conventional-commits/SKILL.md` |
| `_template` | Base pra criar skill nova. | `.skills/_template/SKILL.md` |

### Agents customizados (em `.agents/`)

| Agent | Uso |
|-------|-----|
| `ralph-loop.agent.md` | **Executor autônomo padrão.** Loop até DoD verde. Aciona em toda task técnica. |
| `tdd.agent.md` | TDD specialist. GoogleTest falhando antes do código. Loop red-green-refactor. |
| `reviewer.agent.md` | Code review C++ sem editar. Memory safety, modern C++, concurrency, MLX/Metal idioms. |
| `architect.agent.md` | Desenha arquitetura, cria ADRs, popula `PATTERNS.md`. Não escreve código de produção. |

> `.agents/` é a fonte canônica (padrão AGENTS.md ecosystem). Espelhado em `.github/copilot/agents/` pro GitHub Copilot Workspace. Detalhes em [`.agents/README.md`](../../.agents/README.md).

### Como invocar

- Em Claude Code: `Skill(playwright-e2e)` ou referência em prompt.
- Em Copilot Agent Mode: seleciona agent custom no chat.
- Skills com trigger explícito têm prefixo `$skill-name` em comentário ou prompt.

---

## Checklist rápido antes do PR

- [ ] `task.md` criado e linkado.
- [ ] Branch nome segue convenção (`feat/sprint-XX-<task-id>-<slug>`).
- [ ] Build verde (`cmake --build build` sem warning novo).
- [ ] `clang-format --dry-run --Werror` + `clang-tidy -p build` verdes.
- [ ] Unit (`ctest`) verde, coverage do diff >= 80%.
- [ ] Regression suite verde.
- [ ] Correctness diff dentro da tolerância da task.
- [ ] E2E Playwright verde com **evidência anexada** (`playwright-report/` + `trace.zip` + screenshots + video em retry).
- [ ] PATTERNS.md respeitado, ou ADR aberta.
- [ ] Sem dependência nova não-aprovada.
- [ ] PR title em Conventional Commits.
- [ ] Template de PR preenchido (task link, bench, correctness, E2E).
- [ ] Sem `std::cout` / `printf` / `NSLog` de debug deixado pra trás.
- [ ] Sem secrets em diff (`git diff | grep -i 'secret\|token\|key'`).
- [ ] CHANGELOG atualizado se mudança visível ao usuário.

---

## Erros comuns (não faça)

- Branch que vive 2 semanas: quebra task em pedaços menores (alvo < 2 dias).
- PR de 2000 linhas com "vários ajustes": split em PRs de até 400 linhas.
- GoogleTest que mocka kernel/runtime pra esconder falha: falso verde, vai voltar como incidente.
- Mudar padrão sem ADR: dívida invisível.
- Force-push em branch de PR aberto: reviewer perde contexto.
- Merge com CI vermelho: bloqueia o time inteiro depois.
- Esquecer de remover flag `-DUS4_ENABLE_<X>=OFF` quando backend atinge GA: lixo composto.
- Quebrar correctness sem ADR + sign-off do owner: rollback imediato em `main`.
- Adicionar `#include` sem verificar build em todas as configs (Release/Debug, ARM64).

Em dúvida, pergunte em `us4-core` antes de abrir PR. Custo de pergunta < custo de revert.
