# E2E smoke

Esta pasta agora cobre o estado real do repo: o starter CLI em JavaScript.

## Escopo atual

- alvo executado: `node bin/cli.js`
- contrato smoke: `--version`, `--probe` e `--mode auto --json`
- runner: Playwright Test como orquestrador e produtor de evidencias

## O que o smoke valida

- `--version` responde em texto puro e em JSON
- `--probe` responde em texto com resumo de modo e hardware
- `--probe --json` e `--mode auto --json` expoem o contrato estruturado esperado
- cada execucao anexa stdout/stderr no report do Playwright

## Evidencia esperada

- `playwright-report/index.html`
- `test-results/**`
- attachments com stdout/stderr por caso
- trace habilitado pela configuracao

## Nota de transicao

O nome de contrato continua `us4-cli`, mas o binario real hoje ainda e o starter JS.
Quando o CLI nativo existir, a migracao esperada aqui e trocar o comando executado
no helper do spec, preservando as mesmas assercoes de contrato onde fizer sentido.
