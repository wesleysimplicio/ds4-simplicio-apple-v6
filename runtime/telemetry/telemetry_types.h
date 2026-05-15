#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace us4::telemetry {

enum class TelemetryCategory {
  kLatency,
  kTokenThroughput,
  kMemory,
  kModeTransition,
};

enum class MetricUnit {
  kMicroseconds,
  kTokensPerSecond,
  kBytes,
  kCount,
};

enum class RuntimeMode {
  kFull,
  kBalancedPlus,
  kDegraded,
  kUltraLow,
  kMicro,
  kMicroPlus,
  kNano,
};

enum class ModeTransitionReason {
  kHardwareProbe,
  kMemoryPressure,
  kCorrectnessFallback,
  kThermalPressure,
  kManualOverride,
  kUnknown,
};

struct MetricDescriptor {
  std::string_view name;
  TelemetryCategory category;
  MetricUnit unit;
};

struct LatencyMetric {
  MetricDescriptor descriptor;
  std::uint64_t valueMicros;
};

struct TokenThroughputMetric {
  MetricDescriptor descriptor;
  double tokensPerSecond;
  std::uint64_t tokenCount;
};

struct MemoryMetric {
  MetricDescriptor descriptor;
  std::uint64_t bytes;
  std::uint64_t peakBytes;
};

struct ModeTransitionMetric {
  MetricDescriptor descriptor;
  RuntimeMode fromMode;
  RuntimeMode toMode;
  ModeTransitionReason reason;
  std::uint64_t transitionCount;
};

inline constexpr MetricDescriptor kPrefillLatencyMetric{
    "latency.prefill.us",
    TelemetryCategory::kLatency,
    MetricUnit::kMicroseconds,
};

inline constexpr MetricDescriptor kDecodeStepLatencyMetric{
    "latency.decode_step.us",
    TelemetryCategory::kLatency,
    MetricUnit::kMicroseconds,
};

inline constexpr MetricDescriptor kTokensPerSecondMetric{
    "throughput.tokens_per_second",
    TelemetryCategory::kTokenThroughput,
    MetricUnit::kTokensPerSecond,
};

inline constexpr MetricDescriptor kResidentMemoryMetric{
    "memory.resident.bytes",
    TelemetryCategory::kMemory,
    MetricUnit::kBytes,
};

inline constexpr MetricDescriptor kUnifiedMemoryPeakMetric{
    "memory.unified_peak.bytes",
    TelemetryCategory::kMemory,
    MetricUnit::kBytes,
};

inline constexpr MetricDescriptor kModeTransitionCountMetric{
    "mode.transition.count",
    TelemetryCategory::kModeTransition,
    MetricUnit::kCount,
};

inline constexpr std::array<MetricDescriptor, 6> kSprint01MetricCatalog{
    kPrefillLatencyMetric,
    kDecodeStepLatencyMetric,
    kTokensPerSecondMetric,
    kResidentMemoryMetric,
    kUnifiedMemoryPeakMetric,
    kModeTransitionCountMetric,
};

constexpr std::size_t sprint01MetricCount() {
  return kSprint01MetricCatalog.size();
}

std::string_view toString(TelemetryCategory category);
std::string_view toString(MetricUnit unit);
std::string_view toString(RuntimeMode mode);
std::string_view toString(ModeTransitionReason reason);

}  // namespace us4::telemetry
