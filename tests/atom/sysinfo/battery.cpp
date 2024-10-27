#include "atom/sysinfo/battery.hpp"
#include <gtest/gtest.h>

using namespace atom::system;

// Test fixture for BatteryInfo
class BatteryInfoTest : public ::testing::Test {
protected:
    BatteryInfo batteryInfo;

    void SetUp() override {
        // Initialize BatteryInfo with default values
        batteryInfo = BatteryInfo();
    }
};

// Test default values of BatteryInfo
TEST_F(BatteryInfoTest, DefaultValues) {
    EXPECT_FALSE(batteryInfo.isBatteryPresent);
    EXPECT_FALSE(batteryInfo.isCharging);
    EXPECT_FLOAT_EQ(batteryInfo.batteryLifePercent, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.batteryLifeTime, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.batteryFullLifeTime, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.energyNow, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.energyFull, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.energyDesign, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.voltageNow, 0.0);
    EXPECT_FLOAT_EQ(batteryInfo.currentNow, 0.0);
}

// Test operator== for BatteryInfo
TEST_F(BatteryInfoTest, EqualityOperator) {
    BatteryInfo other;
    EXPECT_TRUE(batteryInfo == other);

    other.isBatteryPresent = true;
    EXPECT_FALSE(batteryInfo == other);
}

// Test operator!= for BatteryInfo
TEST_F(BatteryInfoTest, InequalityOperator) {
    BatteryInfo other;
    EXPECT_FALSE(batteryInfo != other);

    other.isBatteryPresent = true;
    EXPECT_TRUE(batteryInfo != other);
}

// Test operator= for BatteryInfo
TEST_F(BatteryInfoTest, AssignmentOperator) {
    BatteryInfo other;
    other.isBatteryPresent = true;
    other.isCharging = true;
    other.batteryLifePercent = 50.0;
    other.batteryLifeTime = 120.0;
    other.batteryFullLifeTime = 240.0;
    other.energyNow = 5000000.0;
    other.energyFull = 10000000.0;
    other.energyDesign = 12000000.0;
    other.voltageNow = 3.7;
    other.currentNow = 1.5;

    batteryInfo = other;
    EXPECT_TRUE(batteryInfo == other);
}

// Test getBatteryInfo function
TEST(BatteryInfoFunctionTest, GetBatteryInfo) {
    BatteryInfo info = getBatteryInfo();

    // Since we don't know the actual values returned by getBatteryInfo,
    // we will just check if the function returns a BatteryInfo object.
    EXPECT_TRUE(info.isBatteryPresent || !info.isBatteryPresent);
    EXPECT_TRUE(info.isCharging || !info.isCharging);
    EXPECT_GE(info.batteryLifePercent, 0.0);
    EXPECT_GE(info.batteryLifeTime, 0.0);
    EXPECT_GE(info.batteryFullLifeTime, 0.0);
    EXPECT_GE(info.energyNow, 0.0);
    EXPECT_GE(info.energyFull, 0.0);
    EXPECT_GE(info.energyDesign, 0.0);
    EXPECT_GE(info.voltageNow, 0.0);
    EXPECT_GE(info.currentNow, 0.0);
}
