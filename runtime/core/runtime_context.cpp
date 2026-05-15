#include "core/runtime_context.h"

namespace us4 {

RuntimeContext::RuntimeContext(HardwareProbeResult probe_result)
    : hardware_(std::move(probe_result)), mode_(hardware_.recommendedMode) {}

const HardwareProbeResult& RuntimeContext::hardware() const {
  return hardware_;
}

RuntimeMode RuntimeContext::mode() const {
  return mode_;
}

void RuntimeContext::SetMode(RuntimeMode mode) {
  mode_ = mode;
}

}  // namespace us4
