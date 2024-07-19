#include "atom/utils/switch.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace atom::utils;

// Test fixture for StringSwitch
class StringSwitchTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize the switch with some cases
        switch_.registerCase("case1", [](int x) { EXPECT_EQ(x, 1); });
        switch_.registerCase("case2", [](int x) { EXPECT_EQ(x, 2); });
        switch_.setDefault(std::nullopt);
    }

    void TearDown() override { switch_.clearCases(); }

    StringSwitch<int> switch_;
};

// Test case registration
TEST_F(StringSwitchTest, RegisterCase) {
    switch_.registerCase("case3", [](int x) { EXPECT_EQ(x, 3); });
    EXPECT_TRUE(switch_.match("case3", 3));
}

// Test case unregistration
TEST_F(StringSwitchTest, UnregisterCase) {
    switch_.unregisterCase("case1");
    EXPECT_FALSE(switch_.match("case1", 1));
}

// Test case matching
TEST_F(StringSwitchTest, MatchCase) {
    EXPECT_TRUE(switch_.match("case1", 1));
    EXPECT_TRUE(switch_.match("case2", 2));
    EXPECT_FALSE(switch_.match("case3", 3));
}

// Test default function
TEST_F(StringSwitchTest, DefaultFunction) {
    switch_.setDefault([](int x) { EXPECT_EQ(x, 4); });
    EXPECT_TRUE(switch_.match("unknown", 4));
}

// Test clear cases
TEST_F(StringSwitchTest, ClearCases) {
    switch_.clearCases();
    EXPECT_FALSE(switch_.match("case1", 1));
    EXPECT_FALSE(switch_.match("case2", 2));
}

// Test get cases
TEST_F(StringSwitchTest, GetCases) {
    auto cases = switch_.getCases();
    EXPECT_EQ(cases.size(), 2);
    EXPECT_TRUE(std::find(cases.begin(), cases.end(), "case1") != cases.end());
    EXPECT_TRUE(std::find(cases.begin(), cases.end(), "case2") != cases.end());
}

// Test duplicate case registration
TEST_F(StringSwitchTest, DuplicateCase) {
    EXPECT_THROW(switch_.registerCase("case1", [](ATOM_UNUSED int x) {}),
                 atom::error::ObjectAlreadyExist);
}

// Test C++20 designated initializers
TEST(StringSwitchCpp20Test, DesignatedInitializers) {
    StringSwitch<int> switch_{{"case1", [](int x) { EXPECT_EQ(x, 1); }},
                              {"case2", [](int x) { EXPECT_EQ(x, 2); }}};

    EXPECT_TRUE(switch_.match("case1", 1));
    EXPECT_TRUE(switch_.match("case2", 2));
}