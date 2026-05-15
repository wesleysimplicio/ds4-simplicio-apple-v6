#pragma once

#include <string_view>

#include "core/runtime_context.h"

namespace us4 {

enum class ArchitectureType {
  kDense,
  kMoe,
  kTernary,
  kUnknown,
};

class IUS4V6Adapter {
 public:
  virtual ~IUS4V6Adapter() = default;

  virtual std::string_view Family() const = 0;
  virtual std::string_view ModelName() const = 0;
  virtual ArchitectureType Architecture() const = 0;

  virtual bool SupportsMoe() const = 0;
  virtual bool SupportsMlxBackend() const = 0;
  virtual bool SupportsSpeculativeDecoding() const = 0;

  virtual RuntimeMode MinimumMode() const = 0;
  virtual RuntimeMode RecommendedMode(const HardwareProbeResult& hardware) const = 0;
  virtual void ConfigureRuntime(RuntimeContext& context) const = 0;
};

}  // namespace us4
