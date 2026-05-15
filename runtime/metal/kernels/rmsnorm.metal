#include <metal_stdlib>
using namespace metal;

kernel void us4_rmsnorm_fp32(
    device const float* input [[buffer(0)]],
    device const float* weight [[buffer(1)]],
    device float* output [[buffer(2)]],
    constant uint& width [[buffer(3)]],
    uint gid [[thread_position_in_grid]]) {
  const uint row = gid / width;
  const uint col = gid % width;
  const uint rowBase = row * width;
  float sumSquares = 0.0f;
  for (uint i = 0; i < width; ++i) {
    const float v = input[rowBase + i];
    sumSquares += v * v;
  }
  const float invRms = rsqrt(max(sumSquares / max(float(width), 1.0f), 1e-6f));
  output[gid] = input[rowBase + col] * invRms * weight[col];
}
