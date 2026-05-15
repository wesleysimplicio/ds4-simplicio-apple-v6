#include "neon/dequant_int8.h"

#include <cstdint>

namespace us4 {

bool DequantizeInt8Groups(const Tensor &quantized, const std::size_t groupSize,
                          const std::vector<float> &scales, Tensor &output,
                          std::string *error) {
  if (quantized.dtype() != DType::kInt8) {
    if (error != nullptr) {
      *error = "dequant-int8 expects int8 input";
    }
    return false;
  }
  if (groupSize == 0) {
    if (error != nullptr) {
      *error = "dequant-int8 requires non-zero group size";
    }
    return false;
  }
  if (output.dtype() != DType::kFloat32) {
    if (error != nullptr) {
      *error = "dequant-int8 expects float32 output";
    }
    return false;
  }
  if (output.Shape() != quantized.Shape()) {
    if (error != nullptr) {
      *error = "dequant-int8 output shape mismatch";
    }
    return false;
  }

  const std::size_t elementCount = quantized.ElementCount();
  const std::size_t groupCount = (elementCount + groupSize - 1U) / groupSize;
  if (scales.size() != groupCount) {
    if (error != nullptr) {
      *error = "dequant-int8 scale count mismatch";
    }
    return false;
  }

  const auto *source = reinterpret_cast<const std::int8_t *>(quantized.Data());
  float *destination = output.MutableDataAsFloat32();
  if (destination == nullptr) {
    if (error != nullptr) {
      *error = "dequant-int8 output buffer unavailable";
    }
    return false;
  }

  for (std::size_t index = 0; index < elementCount; ++index) {
    const std::size_t groupIndex = index / groupSize;
    destination[index] = static_cast<float>(source[index]) * scales[groupIndex];
  }

  return true;
}

} // namespace us4
