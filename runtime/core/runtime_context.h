#pragma once

#include "core/hardware_probe.h"

namespace us4 {

class RuntimeContext {
 public:
  RuntimeContext() = default;
  explicit RuntimeContext(HardwareProbeResult probe_result);

  const HardwareProbeResult& hardware() const;
  RuntimeMode mode() const;
  void SetMode(RuntimeMode mode);

 private:
  HardwareProbeResult hardware_;
  RuntimeMode mode_ = RuntimeMode::kNano;
};

}  // namespace us4
