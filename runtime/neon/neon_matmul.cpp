#include "neon/neon_matmul.h"

#include <algorithm>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

#include "cpu/scalar_matmul.h"

namespace us4 {

namespace {

bool WriteError(std::string *error, const char *message) {
  if (error != nullptr) {
    *error = message;
  }
  return false;
}

bool ValidateMatrix(const Tensor &tensor, std::string *error) {
  if (tensor.dtype() != DType::kFloat32) {
    return WriteError(error, "tensor must be fp32");
  }
  if (tensor.Rank() != 2) {
    return WriteError(error, "tensor must be rank-2");
  }
  if (!tensor.IsContiguous()) {
    return WriteError(error, "tensor must be contiguous");
  }
  if (tensor.DataAsFloat32() == nullptr) {
    return WriteError(error, "tensor storage is unavailable");
  }
  return true;
}

void RunScalarLane4Matmul(const Tensor &lhs, const Tensor &rhs,
                          Tensor &output) {
  const std::size_t lhsRows = lhs.Shape()[0];
  const std::size_t lhsCols = lhs.Shape()[1];
  const std::size_t rhsCols = rhs.Shape()[1];

  const float *lhsData = lhs.DataAsFloat32();
  const float *rhsData = rhs.DataAsFloat32();
  float *outData = output.MutableDataAsFloat32();

  constexpr std::size_t kLaneWidth = 4U;
  for (std::size_t row = 0; row < lhsRows; ++row) {
    std::size_t col = 0;
    for (; col + kLaneWidth <= rhsCols; col += kLaneWidth) {
      float acc0 = 0.0F;
      float acc1 = 0.0F;
      float acc2 = 0.0F;
      float acc3 = 0.0F;

      for (std::size_t inner = 0; inner < lhsCols; ++inner) {
        const float lhsValue = lhsData[row * lhsCols + inner];
        const std::size_t rhsOffset = inner * rhsCols + col;
        acc0 += lhsValue * rhsData[rhsOffset + 0];
        acc1 += lhsValue * rhsData[rhsOffset + 1];
        acc2 += lhsValue * rhsData[rhsOffset + 2];
        acc3 += lhsValue * rhsData[rhsOffset + 3];
      }

      const std::size_t outOffset = row * rhsCols + col;
      outData[outOffset + 0] = acc0;
      outData[outOffset + 1] = acc1;
      outData[outOffset + 2] = acc2;
      outData[outOffset + 3] = acc3;
    }

    for (; col < rhsCols; ++col) {
      float accumulator = 0.0F;
      for (std::size_t inner = 0; inner < lhsCols; ++inner) {
        accumulator +=
            lhsData[row * lhsCols + inner] * rhsData[inner * rhsCols + col];
      }
      outData[row * rhsCols + col] = accumulator;
    }
  }
}

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
void RunNeonLane4Matmul(const Tensor &lhs, const Tensor &rhs, Tensor &output) {
  const std::size_t lhsRows = lhs.Shape()[0];
  const std::size_t lhsCols = lhs.Shape()[1];
  const std::size_t rhsCols = rhs.Shape()[1];

  const float *lhsData = lhs.DataAsFloat32();
  const float *rhsData = rhs.DataAsFloat32();
  float *outData = output.MutableDataAsFloat32();

  constexpr std::size_t kLaneWidth = 4U;
  for (std::size_t row = 0; row < lhsRows; ++row) {
    std::size_t col = 0;
    for (; col + kLaneWidth <= rhsCols; col += kLaneWidth) {
      float32x4_t accumulator = vdupq_n_f32(0.0F);

      for (std::size_t inner = 0; inner < lhsCols; ++inner) {
        const float lhsValue = lhsData[row * lhsCols + inner];
        const float32x4_t rhsVector =
            vld1q_f32(rhsData + (inner * rhsCols + col));
        accumulator = vaddq_f32(accumulator, vmulq_n_f32(rhsVector, lhsValue));
      }

      vst1q_f32(outData + (row * rhsCols + col), accumulator);
    }

    for (; col < rhsCols; ++col) {
      float accumulator = 0.0F;
      for (std::size_t inner = 0; inner < lhsCols; ++inner) {
        accumulator +=
            lhsData[row * lhsCols + inner] * rhsData[inner * rhsCols + col];
      }
      outData[row * rhsCols + col] = accumulator;
    }
  }
}
#endif

} // namespace

bool NeonMatmul(const Tensor &lhs, const Tensor &rhs, Tensor &output,
                std::string *error) {
  if (!ValidateMatrix(lhs, error) || !ValidateMatrix(rhs, error)) {
    return false;
  }

  if (output.dtype() != DType::kFloat32 || output.Rank() != 2 ||
      output.MutableDataAsFloat32() == nullptr || !output.IsContiguous()) {
    return WriteError(error, "output must be a writable fp32 rank-2 tensor");
  }

  const std::size_t lhsRows = lhs.Shape()[0];
  const std::size_t lhsCols = lhs.Shape()[1];
  const std::size_t rhsRows = rhs.Shape()[0];
  const std::size_t rhsCols = rhs.Shape()[1];

  if (lhsCols != rhsRows) {
    return WriteError(error, "lhs columns must match rhs rows");
  }
  if (output.Shape()[0] != lhsRows || output.Shape()[1] != rhsCols) {
    return WriteError(error, "output shape does not match matmul result");
  }

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
  RunNeonLane4Matmul(lhs, rhs, output);
#else
  RunScalarLane4Matmul(lhs, rhs, output);
#endif

  return true;
}

} // namespace us4
