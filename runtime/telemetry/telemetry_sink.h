#pragma once

#include <cstddef>
#include <vector>

#include "core/runtime_mode.h"

namespace us4 {

struct TelemetrySnapshot {
  double latencyMs = 0.0;
  double tokensPerSecond = 0.0;
  unsigned long long peakMemoryGiB = 0;
  std::size_t promptTokens = 0;
  std::size_t generatedTokens = 0;
  RuntimeMode mode = RuntimeMode::kNano;
};

class TelemetrySink {
 public:
  void Record(const TelemetrySnapshot& snapshot);
  const std::vector<TelemetrySnapshot>& Snapshots() const;
  bool Empty() const;

 private:
  std::vector<TelemetrySnapshot> snapshots_;
};

}  // namespace us4
