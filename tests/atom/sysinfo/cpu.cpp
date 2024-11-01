#include "atom/sysinfo/cpu.hpp"
#include <gtest/gtest.h>

using namespace atom::system;

class CpuTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if necessary
    }

    void TearDown() override {
        // Cleanup code if necessary
    }
};

TEST_F(CpuTest, GetCurrentCpuUsage) {
    float cpuUsage = getCurrentCpuUsage();
    ASSERT_GE(cpuUsage, 0.0);
    ASSERT_LE(cpuUsage, 100.0);
}

TEST_F(CpuTest, GetCurrentCpuTemperature) {
    float temperature = getCurrentCpuTemperature();
    ASSERT_GE(temperature, 0.0);
    // No upper bound check as it can vary widely
}

TEST_F(CpuTest, GetCPUModel) {
    std::string cpuModel = getCPUModel();
    ASSERT_FALSE(cpuModel.empty());
}

TEST_F(CpuTest, GetProcessorIdentifier) {
    std::string identifier = getProcessorIdentifier();
    ASSERT_FALSE(identifier.empty());
}

TEST_F(CpuTest, GetProcessorFrequency) {
    double frequency = getProcessorFrequency();
    ASSERT_GT(frequency, 0.0);
}

TEST_F(CpuTest, GetNumberOfPhysicalPackages) {
    int numberOfPackages = getNumberOfPhysicalPackages();
    ASSERT_GT(numberOfPackages, 0);
}

TEST_F(CpuTest, GetNumberOfPhysicalCPUs) {
    int numberOfCPUs = getNumberOfPhysicalCPUs();
    ASSERT_GT(numberOfCPUs, 0);
}

TEST_F(CpuTest, GetCacheSizes) {
    CacheSizes cacheSizes = getCacheSizes();
    ASSERT_GE(cacheSizes.l1i, 0);
    ASSERT_GE(cacheSizes.l1d, 0);
    ASSERT_GE(cacheSizes.l2, 0);
    ASSERT_GE(cacheSizes.l3, 0);
}
