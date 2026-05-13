# RELEASE — `US4 V6 Apple Edition`

Processo para cortar release de **US4 V6 Apple Edition** (`us4-v6-simplicio-apple`). Releases são tagueadas, automatizadas via GitHub Actions, assinadas (Apple Developer ID) e reversíveis. Dono do processo: `us4-core`. Stack: C++17/20 + CMake + MLX + Metal + NEON + ANE + GoogleTest + Playwright + Ralph Loop.

---

## 1. Princípios

- **SemVer estrito.** `MAJOR.MINOR.PATCH`. Quebra contrato = MAJOR, feature compatível = MINOR, fix compatível = PATCH.
- **Tag é fonte de verdade.** Sem tag, sem deploy de produção.
- **CHANGELOG é contrato com o usuário.** Toda release tem entrada lida e revisada.
- **Rollback em minutos.** Toda release tem caminho documentado de volta (Homebrew tap reverte fórmula, GitHub Release marca anterior como `latest`).
- **Correctness é gate de release.** Logit diff regressivo bloqueia release até resolver ou aprovar via ADR.

---

## 2. Bump de versão (SemVer)

Critério rápido:

| Mudança | Bump |
|---------|------|
| Bug fix interno, sem mudar API/CLI/correctness | PATCH (`1.4.2` -> `1.4.3`) |
| Backend novo atrás de flag (GA), adapter novo, CLI flag retrocompatível | MINOR (`1.4.2` -> `1.5.0`) |
| Quebra de API C++ exportada, mudança em `.us4` format, CLI breaking | MAJOR (`1.4.2` -> `2.0.0`) |
| Pre-release, RC | sufixo (`1.5.0-rc.1`) |

Local do número de versão:
- `CMakeLists.txt`: `project(us4 VERSION 1.5.0 LANGUAGES CXX OBJCXX)`.
- `runtime/version.h`: `constexpr auto kUs4Version = "1.5.0";` (gerado por CMake).
- Homebrew formula em tap separado (`wesleysimplicio/homebrew-us4`): `version "1.5.0"` + novo SHA256.

Bump:

```bash
git add CMakeLists.txt CHANGELOG.md
git commit -m "chore(release): bump to 1.5.0"
```

---

## 3. Atualizar `CHANGELOG.md`

Formato Keep a Changelog. Toda release tem bloco com seções abaixo (omita as vazias):

```markdown
## [1.5.0] - 2026-05-07

### Added
- Metal GEMM kernel with FP16 support (sprint-03 / T03.4).
- ANE opt-in path via `--ane` for M5+ devices.

### Changed
- KV cache eviction policy now uses LRU per-expert (ADR-007).
- Default thread pool size = `hw.physicalcpu` (was `hw.ncpu`).

### Fixed
- Logit diff regression in causal mask for prompt > 2048 tokens (#142).
- MLX tensor leak on `RuntimeContext` shutdown (#138).

### Removed
- Legacy `--gpu` flag (deprecated em v1.3, use `--backend metal`).

### Security
- Bump `safetensors-cpp` from 0.2.1 to 0.2.4 (CVE-2026-0142, OOB read no GGUF loader).
```

Regras:
- PT-BR no chat, **CHANGELOG sempre em inglês** (face pública do repo).
- Sem entrada genérica tipo "various improvements". Específico ou nada.
- `Security` ganha destaque, com CVE/advisory linkado.
- Entrada referencia task (sprint-XX/TXX.Y) ou PR (#numero).
- Logit-diff regressions resolvidas vão em `Fixed` com link pro ADR/issue.

---

## 4. Criar tag

Após bump e CHANGELOG mergeados em `main`:

```bash
git checkout main
git pull --rebase origin main

grep 'VERSION ' CMakeLists.txt
head -20 CHANGELOG.md

git tag -a v1.5.0 -m "Release 1.5.0"
git push origin v1.5.0
```

Tag deve apontar pro commit em que CHANGELOG e version foram atualizados. Não tag em commit antigo.

> Tag é imutável. Errou? Cria nova patch (`v1.5.1`) com correção. Nunca delete e re-cria tag publicada.

---

## 5. Deploy automático via GitHub Actions

Push da tag dispara `.github/workflows/release.yml`:

```yaml
on:
  push:
    tags:
      - 'v*.*.*'

jobs:
  release:
    runs-on: macos-14
    steps:
      - uses: actions/checkout@v4
      - name: Setup toolchain
        run: brew install ninja cmake
      - name: Build Release (arm64)
        run: |
          cmake -S . -B build -G Ninja \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_OSX_ARCHITECTURES=arm64 \
            -DUS4_ENABLE_MLX=ON \
            -DUS4_ENABLE_METAL=ON \
            -DUS4_ENABLE_ANE=ON
          cmake --build build -j $(sysctl -n hw.ncpu)
      - name: Run DoD gate (unit + regression + correctness + e2e)
        run: |
          ctest --test-dir build --output-on-failure
          npx playwright install --with-deps
          npx playwright test
      - name: Sign binary
        env:
          APPLE_DEVELOPER_ID: ${{ secrets.APPLE_DEVELOPER_ID }}
        run: |
          codesign --force --options=runtime --sign "$APPLE_DEVELOPER_ID" \
            --timestamp build/us4-cli
      - name: Package tarball
        run: |
          tar -czf us4-v6-apple-${{ github.ref_name }}.tar.gz \
            -C build us4-cli runtime/libus4-runtime.dylib
          shasum -a 256 us4-v6-apple-${{ github.ref_name }}.tar.gz > checksum.txt
      - name: Notarize
        env:
          APPLE_NOTARY_USER: ${{ secrets.APPLE_NOTARY_USER }}
          APPLE_NOTARY_PASSWORD: ${{ secrets.APPLE_NOTARY_PASSWORD }}
          APPLE_TEAM_ID: ${{ secrets.APPLE_TEAM_ID }}
        run: |
          xcrun notarytool submit us4-v6-apple-${{ github.ref_name }}.tar.gz \
            --apple-id "$APPLE_NOTARY_USER" \
            --password "$APPLE_NOTARY_PASSWORD" \
            --team-id "$APPLE_TEAM_ID" \
            --wait
      - name: Publish GitHub Release
        uses: softprops/action-gh-release@v2
        with:
          files: |
            us4-v6-apple-${{ github.ref_name }}.tar.gz
            checksum.txt
          body_path: CHANGELOG.md
          draft: false
          prerelease: ${{ contains(github.ref_name, '-rc.') }}
      - name: Bump Homebrew tap
        env:
          TAP_TOKEN: ${{ secrets.HOMEBREW_TAP_TOKEN }}
        run: |
          ./.github/scripts/update-homebrew-tap.sh \
            ${{ github.ref_name }} \
            $(cat checksum.txt | cut -d' ' -f1)
      - name: Smoke test pos-publish
        run: |
          brew tap wesleysimplicio/us4
          brew install us4
          us4-cli --version | grep ${{ github.ref_name }}
          us4-cli --probe
      - name: Notify
        run: ./.github/scripts/notify-release.sh ${{ github.ref_name }}
```

Acompanhar o run:

```bash
gh run watch
gh run list --workflow=release.yml --limit 5
```

Falhou? Workflow é idempotente, pode re-rodar. Se assinatura/notarização falhou, é segredo errado — checa `gh secret list` antes de re-rodar.

---

## 6. Smoke tests pós-deploy

Cenários críticos rodando contra a release publicada. Detectar regressão grande em < 5min.

Cobertura mínima:
- `us4-cli --version` retorna a versão nova.
- `us4-cli --probe` detecta hardware (M-series, MLX, Metal, ANE quando M5+).
- `us4-cli run --model qwen-0.5b --prompt "hi"` gera >= 5 tokens em <= 60s.
- Correctness diff em modelo de smoke (`qwen-0.5b`, 16 tokens) dentro de 1e-3.
- Homebrew install limpo em macOS 14 + macOS 15 funciona.

Smoke roda dentro do workflow `release.yml`. Falha = não publica como `latest`, mantém release anterior.

---

## 7. Rollback

Quando: smoke falhou, usuário reportou crash/correctness regressivo, métrica spikou.

### Estratégia A — Reverter Homebrew tap (mais rápido)

```bash
cd ~/dev/homebrew-us4
git revert HEAD
git push origin main
```

Usuários que ainda não atualizaram caem no checksum velho.

### Estratégia B — Marcar release anterior como `latest`

```bash
gh release edit v1.5.0 --prerelease
gh release edit v1.4.2 --latest
```

### Marca a release ruim

```bash
gh release edit v1.5.0 --notes "ROLLED BACK - see incident #INC-2026-05-07"
```

CHANGELOG ganha nota:

```markdown
## [1.5.0] - 2026-05-07 [ROLLED BACK]
> Rolled back at 14:32 UTC. See incident report INC-2026-05-07.
```

### Pós-rollback

- Postmortem em `.specs/incidents/INC-YYYY-MM-DD.md` em até 48h.
- Fix vai em PR normal (com teste regressivo + correctness diff) e tagueia próxima patch (`v1.5.1`).
- Adiciona regression test ou correctness fixture que teria pego o bug.
- Se causa veio de gap arquitetural, abre ADR.
- Atualiza skill/playbook se causa-raiz era processo, não código.

---

## 8. Pre-releases e RCs

Para mudanças grandes (MAJOR) ou backend novo atingindo GA, considere RC:

```bash
git tag v2.0.0-rc.1
git push origin v2.0.0-rc.1
```

- Workflow `release.yml` detecta sufixo `-rc.` e publica como `prerelease: true`.
- Não bumpa Homebrew tap `latest`. Tap separado `wesleysimplicio/us4-rc` opcional.
- Beta testers usam por 3-7 dias antes do tag final `v2.0.0`.
- Bugs em RC viram patch no RC (`v2.0.0-rc.2`), não em PATCH SemVer ainda.

---

## 9. Checklist do release manager

- [ ] `main` verde (build, format, lint, unit, regression, correctness, e2e).
- [ ] Versão bumpada em `CMakeLists.txt` conforme SemVer.
- [ ] `CHANGELOG.md` atualizado, revisado, em inglês, com tasks/PRs linkados.
- [ ] Tag criada apontando pro commit certo.
- [ ] Workflow `release.yml` completou verde.
- [ ] Binário assinado (codesign) + notarizado (Apple).
- [ ] Smoke tests passaram (probe + run + correctness).
- [ ] Homebrew tap atualizado com novo checksum.
- [ ] Métricas estáveis nos primeiros 30min (issues abertos, brew install reports).
- [ ] Notificação pra `us4-core` enviada.
- [ ] Release notes publicadas (`gh release create v1.5.0 -F CHANGELOG.md`).

Em incidente, congelar releases até postmortem fechar com ação concreta no roadmap.
