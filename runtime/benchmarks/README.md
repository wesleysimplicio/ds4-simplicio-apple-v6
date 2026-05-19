# Benchmarks

Este diretorio guarda harnesses de benchmark e os contratos de observabilidade
que sustentam comparacoes honestas entre backends.

## Estado atual

- `dense_baseline.cpp` mede o scaffold denso atual e imprime metadados de
  backend, fallback, token count, tempo, `asset_format`, `asset_path`,
  `mode`, fingerprint do texto gerado, telemetria de mixed dispatch ANE/Metal e
  estado termico observado pelo runtime.
- Os casos `dense-qwen/ane-requested` e `llama-fixture/ane-requested`
  registram explicitamente o caminho ANE. Em hosts sem chip M5+ eles devem
  aparecer como fallback observavel, com `observed_backend` diferente de `ane`,
  `fell_back=true` e `mixed_dispatch_strategy=disabled`.
- Os numeros atuais ainda sao evidencia de contrato, nao prova de throughput
  real de Llama 3/4.

## Evidencia minima para o vertical Llama

Quando o benchmark especifico de Llama chegar em `T07.6`, cada execucao deve
registrar pelo menos:

- `benchmark`
- `model`
- `asset_format`
- `requested_backend`
- `observed_backend`
- `backend_reason`
- `generated_tokens`
- `elapsed_ms`
- `text_fingerprint`
- `recommended_mode` ou `mode`
- `mixed_dispatch_strategy`
- `mixed_dispatch_metal_stages`
- `mixed_dispatch_ane_stages`
- `ane_compiled_layers`
- `ane_prediction_calls`
- `thermal_pressure_level`
- `thermal_reason`
- `thermal_downgraded`
- perfil de hardware relevante para comparar runs

Enquanto a trilha de corretude contra referencia HF nao existir, o benchmark
deve emitir um placeholder explicito (`correctness_delta=pending_external_reference`).
Quando a referencia existir, adicionar o delta dos primeiros 64 tokens no
mesmo output ou em um artefato vizinho.

## Regras

- Nao transformar numero de fixture em claim de performance.
- Nao esconder fallback.
- Nao comparar runs sem hardware profile.
- Nao marcar um backend como suportado so porque o adapter apareceu no registry.
- Em hosts nao-M5, evidencia ANE valida eh a evidencia de fallback claro; nao
  substituir por numeros sinteticos.

## Comandos uteis

```bash
./build/runtime/benchmarks/dense_baseline
./build/apps/us4-cli run --model-path tests/fixtures/models/llama-3.1-8b --prompt "hello" --json
./build/apps/us4-cli run --model-path tests/fixtures/models/llama-3.1-8b/toy-llama.gguf --prompt "hello" --json
```
