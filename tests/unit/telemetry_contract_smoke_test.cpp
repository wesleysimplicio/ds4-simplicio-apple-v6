#include "sprint_01_contract_placeholders.h"

#include <set>
#include <string_view>

#include <gtest/gtest.h>

namespace {

using sprint_01_contract::TelemetryCategory;
using sprint_01_contract::TelemetryGate;

TEST(TelemetryContractSmokeTest, NamesSprint01TelemetryCategories) {
  constexpr auto& categories = sprint_01_contract::kTelemetryCategories;

  EXPECT_EQ(categories.size(), 6U);
  EXPECT_EQ(categories[0], TelemetryCategory::Session);
  EXPECT_EQ(categories[1], TelemetryCategory::HardwareProbe);
  EXPECT_EQ(categories[2], TelemetryCategory::RuntimeModeSelection);
  EXPECT_EQ(categories[3], TelemetryCategory::BackendDispatch);
  EXPECT_EQ(categories[4], TelemetryCategory::Fallback);
  EXPECT_EQ(categories[5], TelemetryCategory::Correctness);
}

TEST(TelemetryContractSmokeTest, PlanningAndBootstrapGatesDoNotPretendRuntimeCorrectness) {
  constexpr auto& gates = sprint_01_contract::kTelemetryGates;

  ASSERT_EQ(gates.size(), 3U);
  EXPECT_EQ(gates[0].phase, "planning");
  EXPECT_TRUE(gates[0].requiresStructuredEvents);
  EXPECT_FALSE(gates[0].requiresLogitDiffEnforcement);
  EXPECT_FALSE(gates[0].requiresRegressionEvidence);

  EXPECT_EQ(gates[1].phase, "bootstrap");
  EXPECT_TRUE(gates[1].requiresStructuredEvents);
  EXPECT_FALSE(gates[1].requiresLogitDiffEnforcement);
  EXPECT_FALSE(gates[1].requiresRegressionEvidence);

  EXPECT_EQ(gates[2].phase, "runtime-correctness");
  EXPECT_TRUE(gates[2].requiresStructuredEvents);
  EXPECT_TRUE(gates[2].requiresLogitDiffEnforcement);
  EXPECT_TRUE(gates[2].requiresRegressionEvidence);
}

TEST(TelemetryContractSmokeTest, GatePhasesStayUnique) {
  std::set<std::string_view> phases;
  std::size_t runtimeCorrectnessGateCount = 0;

  for (const TelemetryGate& gate : sprint_01_contract::kTelemetryGates) {
    EXPECT_TRUE(phases.insert(gate.phase).second);

    if (gate.requiresLogitDiffEnforcement || gate.requiresRegressionEvidence) {
      ++runtimeCorrectnessGateCount;
      EXPECT_EQ(gate.phase, "runtime-correctness");
    }
  }

  EXPECT_EQ(runtimeCorrectnessGateCount, 1U);
}

TEST(TelemetryContractSmokeTest, CategoriesStayUnique) {
  std::set<int> seenValues;

  for (const TelemetryCategory category : sprint_01_contract::kTelemetryCategories) {
    EXPECT_TRUE(seenValues.insert(static_cast<int>(category)).second);
  }
}

}  // namespace
