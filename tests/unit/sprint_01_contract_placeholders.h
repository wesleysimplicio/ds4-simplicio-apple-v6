#pragma once

#include <array>
#include <string_view>

namespace sprint_01_contract {

enum class RuntimeMode {
  FULL,
  BALANCED_PLUS,
  DEGRADED,
  ULTRA_LOW,
  MICRO,
  MICRO_PLUS,
  NANO,
};

struct MemoryTierMapping {
  RuntimeMode mode;
  std::string_view memoryTier;
  std::string_view rationale;
};

struct ProbeResponsibility {
  std::string_view name;
  std::string_view description;
};

enum class TelemetryCategory {
  Session,
  HardwareProbe,
  RuntimeModeSelection,
  BackendDispatch,
  Fallback,
  Correctness,
};

struct TelemetryGate {
  std::string_view phase;
  bool requiresStructuredEvents;
  bool requiresLogitDiffEnforcement;
  bool requiresRegressionEvidence;
};

inline constexpr std::array<RuntimeMode, 7> kCanonicalRuntimeModes = {
    RuntimeMode::FULL,
    RuntimeMode::BALANCED_PLUS,
    RuntimeMode::DEGRADED,
    RuntimeMode::ULTRA_LOW,
    RuntimeMode::MICRO,
    RuntimeMode::MICRO_PLUS,
    RuntimeMode::NANO,
};

inline constexpr std::array<MemoryTierMapping, 7> kMemoryTierMappings = {{
    {RuntimeMode::FULL, "high", "Fits the broadest Apple Silicon memory envelope."},
    {RuntimeMode::BALANCED_PLUS, "medium-high", "Keeps headroom for balanced interactive use."},
    {RuntimeMode::DEGRADED, "medium", "Accepts reduced concurrency before hard fallback."},
    {RuntimeMode::ULTRA_LOW, "low", "Prefers aggressive memory savings over throughput."},
    {RuntimeMode::MICRO, "very-low", "Targets minimal viable local execution."},
    {RuntimeMode::MICRO_PLUS, "very-low-plus", "Extends MICRO with slightly better interactivity."},
    {RuntimeMode::NANO, "minimum", "Last-resort mode for the smallest supported footprint."},
}};

inline constexpr std::array<ProbeResponsibility, 5> kProbeResponsibilities = {{
    {"soc_family", "Identify the Apple Silicon generation and performance tier."},
    {"memory_capacity", "Report unified memory capacity for mode selection."},
    {"backend_capabilities", "Surface MLX, Metal, NEON/Accelerate, and optional ANE readiness."},
    {"runtime_constraints", "Describe hard limits that block a requested mode or backend."},
    {"fallback_context", "Emit enough context to explain automatic downgrade decisions."},
}};

inline constexpr std::array<TelemetryCategory, 6> kTelemetryCategories = {
    TelemetryCategory::Session,
    TelemetryCategory::HardwareProbe,
    TelemetryCategory::RuntimeModeSelection,
    TelemetryCategory::BackendDispatch,
    TelemetryCategory::Fallback,
    TelemetryCategory::Correctness,
};

inline constexpr std::array<TelemetryGate, 3> kTelemetryGates = {{
    {"planning", true, false, false},
    {"bootstrap", true, false, false},
    {"runtime-correctness", true, true, true},
}};

}  // namespace sprint_01_contract
