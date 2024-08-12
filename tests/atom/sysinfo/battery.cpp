#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "atom/sysinfo/battery.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <fstream>
#endif

using namespace atom::system;
using namespace testing;

#ifdef _WIN32
class MockWindowsApi {
public:
    MOCK_METHOD(BOOL, GetSystemPowerStatus, (LPSYSTEM_POWER_STATUS), ());
};

MockWindowsApi* mockWindowsApi;

BOOL WINAPI MockGetSystemPowerStatus(LPSYSTEM_POWER_STATUS lpSystemPowerStatus) {
    return mockWindowsApi->GetSystemPowerStatus(lpSystemPowerStatus);
}

void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    // GetSystemPowerStatus = MockGetSystemPowerStatus;
    typedef WINBOOL (WINAPI *GetSystemPowerStatusPtr)(LPSYSTEM_POWER_STATUS);
    GetSystemPowerStatusPtr realGetSystemPowerStatus = GetSystemPowerStatus;
    GetSystemPowerStatusPtr GetSystemPowerStatus = MockGetSystemPowerStatus;
}

void cleanupMockWindowsApi() {
    delete mockWindowsApi;
}

#else
class MockFileReader {
public:
    MOCK_METHOD(std::string, ReadFile, (const std::string&), ());
};

MockFileReader* mockFileReader;

std::string MockReadFile(const std::string& path) {
    return mockFileReader->ReadFile(path);
}

void setupMockFileReader() {
    mockFileReader = new MockFileReader();
}

void cleanupMockFileReader() {
    delete mockFileReader;
}
#endif

class BatteryTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        setupMockWindowsApi();
#else
        setupMockFileReader();
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        cleanupMockWindowsApi();
#else
        cleanupMockFileReader();
#endif
    }
};

#ifdef _WIN32
TEST_F(BatteryTest, GetBatteryInfo_Windows) {
    SYSTEM_POWER_STATUS powerStatus = {};
    powerStatus.BatteryFlag = 1;
    powerStatus.BatteryLifePercent = 50;
    powerStatus.BatteryLifeTime = 7200;
    powerStatus.BatteryFullLifeTime = 14400;
    powerStatus.ACLineStatus = 1;

    EXPECT_CALL(*mockWindowsApi, GetSystemPowerStatus(_)).WillOnce(DoAll(SetArgPointee<0>(powerStatus), Return(TRUE)));

    BatteryInfo info = getBatteryInfo();

    EXPECT_TRUE(info.isBatteryPresent);
    EXPECT_TRUE(info.isCharging);
    EXPECT_EQ(info.batteryLifePercent, 50.0F);
    EXPECT_EQ(info.batteryLifeTime, 7200.0F);
    EXPECT_EQ(info.batteryFullLifeTime, 14400.0F);
}

#else

TEST_F(BatteryTest, GetBatteryInfo_Linux) {
    std::string mockBatteryData =
        "POWER_SUPPLY_PRESENT=1\n"
        "POWER_SUPPLY_STATUS=Charging\n"
        "POWER_SUPPLY_CAPACITY=75\n"
        "POWER_SUPPLY_TIME_TO_EMPTY_MIN=120\n"
        "POWER_SUPPLY_TIME_TO_FULL_NOW=240\n"
        "POWER_SUPPLY_ENERGY_NOW=40000\n"
        "POWER_SUPPLY_ENERGY_FULL_DESIGN=50000\n"
        "POWER_SUPPLY_VOLTAGE_NOW=12000000\n"
        "POWER_SUPPLY_CURRENT_NOW=2000000\n";

    EXPECT_CALL(*mockFileReader, ReadFile(_)).WillOnce(Return(mockBatteryData));

    BatteryInfo info = getBatteryInfo();

    EXPECT_TRUE(info.isBatteryPresent);
    EXPECT_TRUE(info.isCharging);
    EXPECT_EQ(info.batteryLifePercent, 75.0f);
    EXPECT_EQ(info.batteryLifeTime, 120.0f);
    EXPECT_EQ(info.batteryFullLifeTime, 240.0f);
    EXPECT_EQ(info.energyNow, 40000.0f);
    EXPECT_EQ(info.energyDesign, 50000.0f);
    EXPECT_EQ(info.voltageNow, 12.0f);
    EXPECT_EQ(info.currentNow, 2.0f);
}

#endif
