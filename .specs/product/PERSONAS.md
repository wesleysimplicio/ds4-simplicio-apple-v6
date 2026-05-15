# Personas - US4 V6 Apple Edition

## P1 - ML Engineer

- Runs Qwen, Llama, Gemma, DeepSeek, or Kimi locally on Apple Silicon.
- Needs repeatable benchmarks across adapters and hardware tiers.
- Cares about tokens/s, time-to-first-token, KV behavior, and correctness drift.
- Wins when one runtime replaces a pile of one-off scripts and tools.

## P2 - Researcher

- Compares dense, MoE, and ternary architectures on the same Mac.
- Needs the same prompt, same seed, and clear drift reporting across backends.
- Wants visibility into expert hit-rate, prefetch hit-rate, KV tier movement, and fallback decisions.
- Wins when experimentation becomes comparable instead of anecdotal.

## P3 - App Developer

- Embeds local inference into a privacy-first Apple app or internal tool.
- Needs stable CLI and library contracts, predictable memory behavior, and small-footprint modes.
- Especially values `MICRO`, `MICRO_PLUS`, and `NANO` for constrained devices.
- Wins when local inference is shippable without turning the app into a tuning lab.

## P4 - Fleet / Platform Engineer

- Operates local inference on managed Macs in enterprise or lab environments.
- Needs consistent install, observable health, version discipline, and rollback safety.
- Cares about hardware probe output, supported memory tiers, structured telemetry, and release confidence.
- Wins when the runtime behaves like an operational product instead of an experiment.

## Persona notes

- `16 GB` maps to `NANO`, not `MICRO` or `MICRO_PLUS`.
- Swift/macOS embedding is important, but the first public surface is still CLI + library.
- Multimodal and ANE-heavy workflows are secondary until dense and MoE core paths are correct.
