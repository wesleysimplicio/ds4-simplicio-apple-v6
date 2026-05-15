#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "core/hardware_probe.h"
#include "core/runtime_mode.h"
#include "us4/version.h"

namespace {

void PrintHelp() {
  std::cout
      << "US4 V6 Apple Edition CLI\n"
      << "Usage:\n"
      << "  us4-cli --version\n"
      << "  us4-cli --probe [--json]\n"
      << "  us4-cli --mode auto [--json]\n";
}

void PrintProbeText(const us4::HardwareProbeResult& probe) {
  std::cout
      << "US4 V6 Apple Edition\n"
      << "version: " << us4::kUs4Version << "\n"
      << "platform: " << probe.platform << "\n"
      << "architecture: " << probe.architecture << "\n"
      << "chip: " << probe.chip << "\n"
      << "memory_gib: " << probe.unifiedMemoryGiB << "\n"
      << "is_apple_silicon: " << (probe.isAppleSilicon ? "true" : "false") << "\n"
      << "has_mlx: " << (probe.hasMlx ? "true" : "false") << "\n"
      << "has_metal: " << (probe.hasMetal ? "true" : "false") << "\n"
      << "has_neon: " << (probe.hasNeon ? "true" : "false") << "\n"
      << "has_ane: " << (probe.hasAne ? "true" : "false") << "\n"
      << "recommended_mode: " << us4::ToString(probe.recommendedMode) << "\n";
}

void PrintProbeJson(const us4::HardwareProbeResult& probe) {
  std::cout
      << "{"
      << "\"version\":\"" << us4::kUs4Version << "\","
      << "\"platform\":\"" << probe.platform << "\","
      << "\"architecture\":\"" << probe.architecture << "\","
      << "\"chip\":\"" << probe.chip << "\","
      << "\"memory_gib\":" << probe.unifiedMemoryGiB << ","
      << "\"is_apple_silicon\":" << (probe.isAppleSilicon ? "true" : "false") << ","
      << "\"has_mlx\":" << (probe.hasMlx ? "true" : "false") << ","
      << "\"has_metal\":" << (probe.hasMetal ? "true" : "false") << ","
      << "\"has_neon\":" << (probe.hasNeon ? "true" : "false") << ","
      << "\"has_ane\":" << (probe.hasAne ? "true" : "false") << ","
      << "\"recommended_mode\":\"" << us4::ToString(probe.recommendedMode) << "\""
      << "}\n";
}

}  // namespace

int main(int argc, char** argv) {
  bool output_json = false;
  bool show_probe = false;
  bool show_version = false;
  bool show_help = false;
  std::optional<std::string> mode_value;

  for (int index = 1; index < argc; ++index) {
    const std::string_view arg = argv[index];
    if (arg == "--json") {
      output_json = true;
    } else if (arg == "--probe") {
      show_probe = true;
    } else if (arg == "--version") {
      show_version = true;
    } else if (arg == "--help" || arg == "-h") {
      show_help = true;
    } else if (arg == "--mode" && index + 1 < argc) {
      mode_value = argv[++index];
    } else {
      std::cerr << "Unknown argument: " << arg << "\n";
      PrintHelp();
      return 1;
    }
  }

  if (show_help || argc == 1) {
    PrintHelp();
    return 0;
  }

  if (show_version) {
    std::cout << us4::kUs4Version << "\n";
    return 0;
  }

  const us4::HardwareProbeResult probe = us4::HardwareProbe::Detect();

  if (show_probe) {
    if (output_json) {
      PrintProbeJson(probe);
    } else {
      PrintProbeText(probe);
    }
    return 0;
  }

  if (mode_value.has_value()) {
    const auto parsed_mode = us4::ParseRuntimeMode(*mode_value);
    const us4::RuntimeMode mode =
        (mode_value == "auto" || !parsed_mode.has_value())
            ? probe.recommendedMode
            : *parsed_mode;

    if (output_json) {
      std::cout << "{\"mode\":\"" << us4::ToString(mode) << "\"}\n";
    } else {
      std::cout << us4::ToString(mode) << "\n";
    }
    return 0;
  }

  PrintHelp();
  return 0;
}
