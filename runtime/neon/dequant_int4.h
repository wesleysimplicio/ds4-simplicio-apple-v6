#pragma once

#include <string>
#include <vector>

#include "core/tensor.h"

namespace us4 {

bool DequantizeInt4Groups(const Tensor &quantized,
                          std::size_t logicalElementCount,
                          std::size_t groupSize,
                          const std::vector<float> &scales, Tensor &output,
                          std::string *error = nullptr);

} // namespace us4
