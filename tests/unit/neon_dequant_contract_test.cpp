#include <gtest/gtest.h>

#include <cstdint>

#include "neon/dequant_int4.h"
#include "neon/dequant_int8.h"

TEST(NeonDequantContractTest, Int8GroupwiseDequantScalesEachGroup) {
  us4::Tensor quantized({4}, us4::DType::kInt8, us4::DeviceType::kCpu);
  us4::Tensor output({4}, us4::DType::kFloat32, us4::DeviceType::kCpu);

  auto *bytes = reinterpret_cast<std::int8_t *>(quantized.MutableData());
  bytes[0] = 4;
  bytes[1] = -2;
  bytes[2] = 3;
  bytes[3] = -1;

  std::string error;
  ASSERT_TRUE(
      us4::DequantizeInt8Groups(quantized, 2, {0.5F, 0.25F}, output, &error))
      << error;

  const float *values = output.DataAsFloat32();
  ASSERT_NE(values, nullptr);
  EXPECT_FLOAT_EQ(values[0], 2.0F);
  EXPECT_FLOAT_EQ(values[1], -1.0F);
  EXPECT_FLOAT_EQ(values[2], 0.75F);
  EXPECT_FLOAT_EQ(values[3], -0.25F);
}

TEST(NeonDequantContractTest, Int4GroupwiseDequantUnpacksSignedNibbles) {
  us4::Tensor quantized({8}, us4::DType::kInt4, us4::DeviceType::kCpu);
  us4::Tensor output({8}, us4::DType::kFloat32, us4::DeviceType::kCpu);

  auto *bytes = reinterpret_cast<std::uint8_t *>(quantized.MutableData());
  bytes[0] = 0x2F; // -1, 2
  bytes[1] = 0x91; // 1, -7
  bytes[2] = 0x47; // 7, 4
  bytes[3] = 0x8C; // -4, -8

  std::string error;
  ASSERT_TRUE(
      us4::DequantizeInt4Groups(quantized, 8, 4, {0.5F, 0.25F}, output, &error))
      << error;

  const float *values = output.DataAsFloat32();
  ASSERT_NE(values, nullptr);
  EXPECT_FLOAT_EQ(values[0], -0.5F);
  EXPECT_FLOAT_EQ(values[1], 1.0F);
  EXPECT_FLOAT_EQ(values[2], 0.5F);
  EXPECT_FLOAT_EQ(values[3], -3.5F);
  EXPECT_FLOAT_EQ(values[4], 1.75F);
  EXPECT_FLOAT_EQ(values[5], 1.0F);
  EXPECT_FLOAT_EQ(values[6], -1.0F);
  EXPECT_FLOAT_EQ(values[7], -2.0F);
}

TEST(NeonDequantContractTest, DequantRejectsScaleShapeMismatch) {
  us4::Tensor quantized({4}, us4::DType::kInt8, us4::DeviceType::kCpu);
  us4::Tensor output({4}, us4::DType::kFloat32, us4::DeviceType::kCpu);

  std::string error;
  EXPECT_FALSE(us4::DequantizeInt8Groups(quantized, 2, {0.5F}, output, &error));
  EXPECT_EQ(error, "dequant-int8 scale count mismatch");
}
