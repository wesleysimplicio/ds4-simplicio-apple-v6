#include "sprint_01_contract_placeholders.h"

#include <algorithm>
#include <set>

#include <gtest/gtest.h>

namespace {

using sprint_01_contract::MemoryTierMapping;
using sprint_01_contract::RuntimeMode;

TEST(RuntimeModeContractSmokeTest, UsesCanonicalSprint01Taxonomy) {
  constexpr auto& modes = sprint_01_contract::kCanonicalRuntimeModes;

  EXPECT_EQ(modes.size(), 7U);
  EXPECT_EQ(modes[0], RuntimeMode::FULL);
  EXPECT_EQ(modes[1], RuntimeMode::BALANCED_PLUS);
  EXPECT_EQ(modes[2], RuntimeMode::DEGRADED);
  EXPECT_EQ(modes[3], RuntimeMode::ULTRA_LOW);
  EXPECT_EQ(modes[4], RuntimeMode::MICRO);
  EXPECT_EQ(modes[5], RuntimeMode::MICRO_PLUS);
  EXPECT_EQ(modes[6], RuntimeMode::NANO);
}

TEST(RuntimeModeContractSmokeTest, KeepsModeValuesUnique) {
  std::set<int> seenValues;

  for (const RuntimeMode mode : sprint_01_contract::kCanonicalRuntimeModes) {
    EXPECT_TRUE(seenValues.insert(static_cast<int>(mode)).second);
  }
}

TEST(RuntimeModeContractSmokeTest, DocumentsMemoryTierMappingForEveryMode) {
  constexpr auto& mappings = sprint_01_contract::kMemoryTierMappings;
  std::set<int> coveredModes;

  EXPECT_EQ(mappings.size(), sprint_01_contract::kCanonicalRuntimeModes.size());

  for (const MemoryTierMapping& mapping : mappings) {
    const auto canonicalIt =
        std::find(sprint_01_contract::kCanonicalRuntimeModes.begin(),
                  sprint_01_contract::kCanonicalRuntimeModes.end(),
                  mapping.mode);

    EXPECT_NE(canonicalIt, sprint_01_contract::kCanonicalRuntimeModes.end());
    EXPECT_TRUE(coveredModes.insert(static_cast<int>(mapping.mode)).second);
    EXPECT_FALSE(mapping.memoryTier.empty());
    EXPECT_FALSE(mapping.rationale.empty());
  }

  EXPECT_EQ(coveredModes.size(), sprint_01_contract::kCanonicalRuntimeModes.size());
}

}  // namespace
