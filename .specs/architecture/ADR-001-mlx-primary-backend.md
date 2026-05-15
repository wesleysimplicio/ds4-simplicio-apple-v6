# ADR-001 - MLX as Primary Backend on Apple Silicon

## Status

Accepted

## Context

US4 V6 Apple Edition targets Apple Silicon first. The project needs one default execution path that matches unified memory well and minimizes bespoke kernel work in the first implementation phases.

## Decision

MLX is the primary backend for Apple Silicon.

Metal remains available for measured hot kernels not well covered by MLX. NEON / Accelerate remains the CPU fallback. ANE is opt-in and secondary.

## Consequences

- Runtime docs and planning assume MLX-first.
- Sprint ordering prefers dense correctness on MLX before broad Metal specialization.
- Any Metal-first exception should be justified by benchmark evidence and correctness parity.
