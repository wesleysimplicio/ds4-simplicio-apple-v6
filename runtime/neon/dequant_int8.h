#pragma once

#include <string>
#include <vector>

#include "core/tensor.h"

namespace us4 {

bool DequantizeInt8Groups(const Tensor &quantized, std::size_t groupSize,
                          const std::vector<float> &scales, Tensor &output,
                          std::string *error = nullptr);

} // namespace us4
