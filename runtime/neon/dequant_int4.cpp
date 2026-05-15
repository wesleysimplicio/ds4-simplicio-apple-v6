#include "neon/dequant_int4.h"

#include <cstdint>

namespace us4 {

namespace {

std::int8_t DecodeSignedNibble(const std::uint8_t nibble) {
  const std::uint8_t normalized = nibble & 0x0FU;
  return normalized >= 8U ? static_cast<std::int8_t>(normalized) - 16
                          : static_cast<std::int8_t>(normalized);
}

} // namespace

bool DequantizeInt4Groups(const Tensor &quantized,
                          const std::size_t logicalElementCount,
                          const std::size_t groupSize,
                          const std::vector<float> &scales, Tensor &output,
                          std::string *error) {
  if (quantized.dtype() != DType::kInt4) {
    if (error != nullptr) {
      *error = "dequant-int4 expects int4 input";
    }
    return false;
  }
  if (groupSize == 0 || logicalElementCount == 0) {
    if (error != nullptr) {
      *error =
          "dequant-int4 requires non-zero logical element count and group size";
    }
    return false;
  }
  if (output.dtype() != DType::kFloat32) {
    if (error != nullptr) {
      *error = "dequant-int4 expects float32 output";
    }
    return false;
  }
  if (output.ElementCount() != logicalElementCount) {
    if (error != nullptr) {
      *error = "dequant-int4 output element count mismatch";
    }
    return false;
  }

  const std::size_t requiredBytes = (logicalElementCount + 1U) / 2U;
  if (quantized.ByteSize() < requiredBytes) {
    if (error != nullptr) {
      *error = "dequant-int4 input buffer too small";
    }
    return false;
  }

  const std::size_t groupCount =
      (logicalElementCount + groupSize - 1U) / groupSize;
  if (scales.size() != groupCount) {
    if (error != nullptr) {
      *error = "dequant-int4 scale count mismatch";
    }
    return false;
  }

  const auto *source = reinterpret_cast<const std::uint8_t *>(quantized.Data());
  float *destination = output.MutableDataAsFloat32();
  if (destination == nullptr) {
    if (error != nullptr) {
      *error = "dequant-int4 output buffer unavailable";
    }
    return false;
  }

  for (std::size_t index = 0; index < logicalElementCount; ++index) {
    const std::uint8_t packed = source[index / 2U];
    const std::uint8_t nibble =
        (index % 2U == 0U) ? (packed & 0x0FU) : (packed >> 4U);
    const std::size_t groupIndex = index / groupSize;
    destination[index] =
        static_cast<float>(DecodeSignedNibble(nibble)) * scales[groupIndex];
  }

  return true;
}

} // namespace us4
