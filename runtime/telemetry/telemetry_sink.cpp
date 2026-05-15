#include "telemetry/telemetry_sink.h"

namespace us4 {

void TelemetrySink::Record(const TelemetrySnapshot& snapshot) {
  snapshots_.push_back(snapshot);
}

const std::vector<TelemetrySnapshot>& TelemetrySink::Snapshots() const {
  return snapshots_;
}

bool TelemetrySink::Empty() const {
  return snapshots_.empty();
}

}  // namespace us4
