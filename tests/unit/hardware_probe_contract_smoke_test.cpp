#include "sprint_01_contract_placeholders.h"

#include <set>
#include <string_view>

#include <gtest/gtest.h>

namespace {

using sprint_01_contract::ProbeResponsibility;

TEST(HardwareProbeContractSmokeTest, ListsExplicitSprint01Responsibilities) {
  constexpr auto& responsibilities = sprint_01_contract::kProbeResponsibilities;

  ASSERT_EQ(responsibilities.size(), 5U);
  EXPECT_EQ(responsibilities[0].name, "soc_family");
  EXPECT_EQ(responsibilities[1].name, "memory_capacity");
  EXPECT_EQ(responsibilities[2].name, "backend_capabilities");
  EXPECT_EQ(responsibilities[3].name, "runtime_constraints");
  EXPECT_EQ(responsibilities[4].name, "fallback_context");
}

TEST(HardwareProbeContractSmokeTest, ResponsibilityNamesAreUniqueAndDescriptive) {
  std::set<std::string_view> seenNames;

  for (const ProbeResponsibility& responsibility :
       sprint_01_contract::kProbeResponsibilities) {
    EXPECT_TRUE(seenNames.insert(responsibility.name).second);
    EXPECT_FALSE(responsibility.description.empty());
  }
}

}  // namespace
