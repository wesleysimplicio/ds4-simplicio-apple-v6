#include <metal_stdlib>
using namespace metal;

kernel void us4_matmul_fp16(
    device const half* lhs [[buffer(0)]],
    device const half* rhs [[buffer(1)]],
    device half* out [[buffer(2)]],
    constant uint& inner [[buffer(3)]],
    constant uint& width [[buffer(4)]],
    uint gid [[thread_position_in_grid]]) {
  const uint row = gid / width;
  const uint col = gid % width;
  half value = half(0.0h);
  for (uint k = 0; k < inner; ++k) {
    value += lhs[row * inner + k] * rhs[k * width + col];
  }
  out[gid] = value;
}
