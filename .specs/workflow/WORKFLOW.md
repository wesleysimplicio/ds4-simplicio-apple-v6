# WORKFLOW — `US4 V6 Apple Edition`

Como **us4-core** move código de ideia até produção em **local LLM inference runtime**. Fonte de verdade pra branch strategy, PR rules, code review, deploy pipeline e hotfix. Stack: C++17/20 + CMake + MLX + Metal + NEON (Accelerate) + ANE (M5+) + GoogleTest + Playwright + Ralph Loop.

---

## 1. Branch Strategy

### Padrão: Trunk-Based Development

Branch única de longa vida: `main`. Tudo merge em `main` via PR pequeno e rápido. Releases saem de `main` via tag.

- `main` é sempre deployável. Build verde, testes passando, sem feature half-done exposta.
- Feature branches são **curtas** (vida média < 2 dias). Branch que dura semana = sinal de task mal quebrada.
- Feature flags isolam código incompleto que já foi merged. Backends novos ficam atrás de `-DUS4_ENABLE_<BACKEND>=OFF` por default até GA.
- Rebase em cima de `main` antes do merge. Histórico linear, sem merge commits ruidosos.

### Naming convention

```
feat/<short-desc>          # nova feature      ex: feat/sprint-03-cuda-gemm
fix/<short-desc>           # bug fix           ex: fix/attention-mask-overflow
chore/<short-desc>         # manutencao        ex: chore/bump-mlx
refactor/<short-desc>      # refactor sem mudar comportamento
docs/<short-desc>          # so docs
perf/<short-desc>          # otimizacao de performance
hotfix/<short-desc>        # patch urgente em producao
release/<vX.Y.Z>           # branch de release
```

`<short-desc>` em kebab-case, máximo 4 palavras, sem ticket id no nome (ticket vai no commit/PR).

### Sprint branches

Tasks de sprint usam padrão `feat/sprint-XX-<task-id>-<slug>`. Exemplo: `feat/sprint-02-t02.2-scalar-gemm`.

---

## 2. Pull Request Rules

### Tamanho

- **Alvo:** PR com até 400 linhas modificadas (sem contar lock files e gerados).
- PR > 600 linhas precisa justificativa explícita no corpo. Reviewer pode pedir split.
- **1 PR = 1 propósito.** Nada de "feat: scalar matmul + refactor adapter base + bump deps" no mesmo PR.

### Title

Conventional Commits no título. Exemplos:

```
feat(runtime): add scalar attention with causal mask
fix(adapters/qwen): handle empty KV concat in first token
chore(ci): cache vcpkg between runs
perf(cuda): fuse epilogue into GEMM kernel
docs(adr): add ADR-007 about expert pager eviction policy
```

### Body (template em `.github/PULL_REQUEST_TEMPLATE.md`)

Toda PR contém:

- Link pra task em `.specs/sprints/sprint-XX/<id>.task.md` (ou issue).
- Resumo do que muda em 3-5 bullets.
- Bench numbers (tokens/s cold/warm, RAM/VRAM peak) se aplicável.
- Correctness diff vs referência (link pro relatório de `runtime/benchmarks/correctness/`).
- Evidência E2E (link pro report Playwright + trace.zip).
- Checklist DoD (build, format, lint, unit, e2e, regression, correctness, docs, changelog).
- Riscos e plano de rollback.

### Review

- **Mínimo 1 reviewer humano.** Nunca self-merge em `main` sem aprovação.
- PRs tocando kernel/runtime/segurança exigem 2 reviewers, sendo 1 com tag `runtime` ou `security`.
- Bot review (CodeRabbit, Greptile, agents ECC) é complementar, não substitui humano.
- Reviewer responde em até 4h úteis. SLA quebrado = feature freeze do reviewer.

---

## 3. Code Review Protocol

### Reviewer

- Lê task antes do diff. Sem contexto, comentário fica raso.
- Comenta no diff usando convenção:
  - `nit:` opinião estética, autor pode ignorar.
  - `q:` pergunta, espera resposta.
  - `req:` mudança bloqueante antes do merge.
  - `praise:` reforço positivo de padrão bom.
- Aprova com `LGTM` apenas quando todos `req:` resolvidos.
- Se ler ADR violado, abre `req:` linkando ADR.
- Se correctness diff regredir > tolerância da task, abre `req:` bloqueando.

### Autor

- Não merga com `req:` aberto.
- Push novo commit em vez de force-push enquanto review tá ativo.
- Após aprovação, pode `git rebase -i` pra squash local antes do merge.

### Merge

- **Squash and merge** é o padrão. Mensagem squash = title do PR + corpo enxuto.
- Branch é deletada automaticamente pós-merge (config do repo).
- Build pós-merge em `main` precisa ficar verde. Vermelho = revert imediato.

---

## 4. Deploy Pipeline

### Visão geral

```
push main -> GitHub Actions (macos-14 (M-series))
  - format check (clang-format --dry-run --Werror + clang-tidy -p build)
  - build CMake + Ninja em Release
  - unit tests via CTest (cobertura mínima 80%)
  - regression suite (adapters/modes/backends anteriores)
  - correctness diff vs referência
  - e2e Playwright (CLI flow smoke)
  - artifact upload (binário + Playwright report + trace)
  - se tudo verde: publish artifact em GitHub Release (draft)
```

### Ambientes / Canais de distribuição

| Canal | Branch/Tag | Trigger | Saída |
|----------|------------|---------|-----|
| `dev` | qualquer PR | abrir/atualizar PR | preview-<pr>.us4-apple.dev (CI artifact tarball) |
| `staging` | `main` | merge em main | GitHub Release pre-release tarball |
| `production` | tag `vX.Y.Z` | tag push manual | Homebrew tap `wesleysimplicio/us4` + signed tarball |

### Deploy de produção

- Disparado por tag SemVer: `git tag v1.4.2 && git push origin v1.4.2`.
- Workflow `release.yml` em `.github/workflows/` faz: build assinado, upload pra Release, smoke test pós-publish, notify channel.
- Janela preferida: terça e quarta, manhã. Evitar sexta e véspera de feriado.
- Rollback: marcar Release anterior como "latest" + emitir patch.

### Feature flags / Toggles de build

- Backends novos ficam atrás de flag CMake `-DUS4_ENABLE_<BACKEND>=OFF` até atingirem DoD de GA (cobertura, correctness, bench).
- Adapters novos ficam atrás de runtime flag `--enable-adapter <name>` até GA.
- Flag tem dono e data de remoção. Flag órfã > 60 dias é tech debt e vai pro BACKLOG.

---

## 5. Hotfix Process

Use quando bug crítico em produção exige patch fora do ciclo normal.

### Critério

Hotfix só pra: correctness regression em release publicado, crash em `us4-cli run`, security incident, data loss em KV cache persistido. Performance ruim não é hotfix, vai pelo fluxo normal.

### Passos

```bash
# 1. Branch a partir da tag de producao atual
git checkout v1.4.2
git checkout -b hotfix/cuda-attention-nan

# 2. Fix minimo. Sem refactor. Sem feature extra.
# 3. Teste regressivo cobrindo o bug + correctness diff anexado
# 4. PR rotulado "hotfix" com aprovacao acelerada (1 reviewer, SLA 30min)

# 5. Merge squash em main
# 6. Tag patch SemVer
git tag v1.4.3
git push origin v1.4.3

# 7. Release automatico via workflow release.yml
# 8. Pos-incidente: postmortem em 48h em .specs/incidents/
```

### Pós-hotfix

- Postmortem blameless: o que falhou, como detectou, o que melhorar.
- Adicionar teste que teria pego o bug (unit ou regression + correctness).
- Se falha veio de gap arquitetural, abrir ADR.
- Atualizar `CHANGELOG.md` com entrada `Fixed` na nova versão.

---

## 6. Branch protection settings

Configuração em GitHub do repo `us4-v6-simplicio-apple`:

- `main`: require PR, require 1 approval, require status checks (ci, dod, e2e), require linear history, dismiss stale reviews on push, no force-push, no delete.
- Tags `v*`: protected, only us4-core maintainers podem criar.
- CODEOWNERS em `.github/CODEOWNERS` aponta áreas (runtime/, adapters/, kernel/) pros maintainers responsáveis.

Esta configuração é versionada via `gh api` script em `.github/repo-settings/` (ou Terraform quando o projeto crescer).
