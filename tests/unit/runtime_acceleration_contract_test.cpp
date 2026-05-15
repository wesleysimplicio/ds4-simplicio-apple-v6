#include <gtest/gtest.h>

#include "adapters/llama/llama_adapter.h"
#include "core/runtime_context.h"
#include "memory/unified_allocator.h"
#include "metal/command_queue.h"
#include "metal/kernel_library.h"
#include "mlx/mlx_bridge.h"

namespace {

us4::HardwareProbeResult MakeAppleProbe() {
  us4::HardwareProbeResult probe;
  probe.platform = "macos";
  probe.architecture = "arm64";
  probe.chip = "apple-m";
  probe.unifiedMemoryGiB = 36;
  probe.hasMetal = true;
  probe.hasMlx = true;
  probe.hasNeon = true;
  probe.recommendedMode = us4::RuntimeMode::kBalancedPlus;
  return probe;
}

}  // namespace

TEST(RuntimeAccelerationContractTest, UnifiedAllocatorTracksSharedAllocations) {
  us4::UnifiedAllocator allocator;
  allocator.Allocate(64, false);
  allocator.Allocate(256, true);

  EXPECT_EQ(allocator.AllocationCount(), 2U);
  EXPECT_EQ(allocator.SharedAllocationCount(), 1U);
  EXPECT_EQ(allocator.ResidentBytes(), 320U);
}

TEST(RuntimeAccelerationContractTest, RuntimeContextExposesMetalQueueAndMlxBridge) {
  us4::RuntimeContext context(MakeAppleProbe());

  EXPECT_TRUE(context.metalQueue().Available());
  EXPECT_EQ(context.metalQueue().Reason(), "metal-queue-ready");
  EXPECT_TRUE(context.mlxBridge().Available());
  EXPECT_EQ(context.mlxBridge().Reason(), "mlx-bridge-ready");
}

TEST(RuntimeAccelerationContractTest, MetalQueueRecordsSharedDispatches) {
  us4::RuntimeContext context(MakeAppleProbe());
  const auto shared = context.allocator().Allocate(512, true);

  EXPECT_TRUE(context.metalQueue().Dispatch(us4::MetalKernelKind::kMatmul, 4, 32, shared));
  ASSERT_EQ(context.metalQueue().DispatchCount(), 1U);
  EXPECT_EQ(context.metalQueue().Dispatches().front().entryPoint, "us4_matmul_fp16");
  EXPECT_EQ(context.metalQueue().Dispatches().front().relativePath, "runtime/metal/kernels/matmul.metal");
  EXPECT_TRUE(context.metalQueue().Dispatches().front().usesSharedAllocation);
  EXPECT_EQ(context.metalQueue().Reason(), "metal-dispatch-recorded");
}

TEST(RuntimeAccelerationContractTest, MetalKernelCatalogExposesKernelMetadata) {
  const auto& catalog = us4::GetMetalKernelCatalog();

  ASSERT_EQ(catalog.size(), 3U);
  EXPECT_NE(us4::FindMetalKernel(us4::MetalKernelKind::kMatmul), nullptr);
  EXPECT_NE(us4::FindMetalKernel(us4::MetalKernelKind::kSoftmax), nullptr);
  EXPECT_NE(us4::FindMetalKernel(us4::MetalKernelKind::kRmsNorm), nullptr);
  EXPECT_EQ(catalog[1].entryPoint, "us4_softmax_rows");
  EXPECT_EQ(catalog[2].relativePath, "runtime/metal/kernels/rmsnorm.metal");
  EXPECT_FALSE(catalog[0].source.empty());
}

TEST(RuntimeAccelerationContractTest, MlxBridgeBuildsAndEvaluatesDensePlan) {
  us4::RuntimeContext context(MakeAppleProbe());
  const auto shared = context.allocator().Allocate(1024, true);

  EXPECT_TRUE(context.mlxBridge().BuildDensePlan("llama", 64, shared));
  ASSERT_TRUE(context.mlxBridge().LastPlan().has_value());
  EXPECT_EQ(context.mlxBridge().LastPlan()->family, "llama");
  EXPECT_TRUE(context.mlxBridge().LastPlan()->usesUnifiedAllocation);
  EXPECT_TRUE(context.mlxBridge().EvaluateLastPlan());
  EXPECT_TRUE(context.mlxBridge().LastEvaluationSucceeded());
  EXPECT_EQ(context.mlxBridge().Reason(), "mlx-plan-evaluated");
}

TEST(RuntimeAccelerationContractTest, LlamaGenerationTouchesMetalScaffoldWhenSelected) {
  us4::RuntimeContext context(MakeAppleProbe());
  const us4::LlamaAdapter adapter;

  const us4::GenerationResult result = adapter.Generate(
      {.prompt = "hello", .maxTokens = 4, .asset = nullptr, .requestedBackend = us4::BackendType::kMetal},
      context);

  EXPECT_EQ(result.backend, "metal");
  EXPECT_EQ(result.sharedAllocations, 1U);
  EXPECT_EQ(result.metalDispatches, 1U);
  EXPECT_FALSE(result.mlxPlanBuilt);
  EXPECT_EQ(context.metalQueue().DispatchCount(), 1U);
  EXPECT_EQ(context.allocator().SharedAllocationCount(), 1U);
}
