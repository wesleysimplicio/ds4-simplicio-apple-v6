#include <cstdlib>
#include <iostream>

#include "core/hardware_probe.h"
#include "core/runtime_context.h"
#include "core/runtime_mode.h"
#include "telemetry/telemetry_sink.h"

namespace {

bool Expect(bool condition, const char* message) {
  if (!condition) {
    std::cerr << message << "\n";
    return false;
  }
  return true;
}

}  // namespace

int main() {
  bool ok = true;

  const us4::HardwareProbeResult probe = us4::HardwareProbe::Detect();
  ok &= Expect(!probe.platform.empty(), "platform should not be empty");
  ok &= Expect(!probe.architecture.empty(), "architecture should not be empty");

  const us4::RuntimeMode expected_mode =
      us4::SelectRuntimeModeFromMemoryGiB(probe.unifiedMemoryGiB);
  ok &= Expect(expected_mode == probe.recommendedMode, "recommended mode should match memory tier");

  us4::RuntimeContext context(probe);
  ok &= Expect(context.mode() == probe.recommendedMode, "runtime context should start with recommended mode");

  context.SetMode(us4::RuntimeMode::kMicro);
  ok &= Expect(context.mode() == us4::RuntimeMode::kMicro, "runtime context should allow mode override");

  us4::TelemetrySink sink;
  ok &= Expect(sink.Empty(), "telemetry sink should start empty");
  sink.Record({12.5, 34.0, probe.unifiedMemoryGiB, 8, 4, context.mode()});
  ok &= Expect(!sink.Empty(), "telemetry sink should store snapshots");
  ok &= Expect(sink.Snapshots().size() == 1, "telemetry sink should keep one snapshot");

  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
