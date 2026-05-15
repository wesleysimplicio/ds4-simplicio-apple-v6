#pragma once

#include <array>
#include <string_view>

#include "metal/command_queue.h"

namespace us4 {

struct MetalKernelDescriptor {
  MetalKernelKind kind = MetalKernelKind::kMatmul;
  std::string_view entryPoint;
  std::string_view relativePath;
  std::string_view source;
  std::size_t preferredThreadsPerGroup = 0;
};

using MetalKernelCatalog = std::array<MetalKernelDescriptor, 3>;

const MetalKernelCatalog& GetMetalKernelCatalog();
const MetalKernelDescriptor* FindMetalKernel(MetalKernelKind kind);

}  // namespace us4
