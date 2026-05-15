#include <iostream>

#include "core/hardware_probe.h"
#include "core/runtime_mode.h"

int main() {
  const us4::HardwareProbeResult probe = us4::HardwareProbe::Detect();
  std::cout << "dense_baseline_placeholder\n";
  std::cout << "recommended_mode=" << us4::ToString(probe.recommendedMode) << "\n";
  return 0;
}
