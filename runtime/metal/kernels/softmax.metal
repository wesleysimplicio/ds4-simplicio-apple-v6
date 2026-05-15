#include <metal_stdlib>
using namespace metal;

kernel void us4_softmax_rows(
    device const float* input [[buffer(0)]],
    device float* output [[buffer(1)]],
    constant uint& width [[buffer(2)]],
    uint gid [[thread_position_in_grid]]) {
  const uint row = gid / width;
  const uint col = gid % width;
  const uint rowBase = row * width;
  float maxValue = input[rowBase];
  for (uint i = 1; i < width; ++i) {
    maxValue = max(maxValue, input[rowBase + i]);
  }
  float denom = 0.0f;
  for (uint i = 0; i < width; ++i) {
    denom += exp(input[rowBase + i] - maxValue);
  }
  output[gid] = exp(input[rowBase + col] - maxValue) / max(denom, 1e-6f);
}
