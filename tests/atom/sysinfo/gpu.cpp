#include "atom/sysinfo/gpu.hpp"
#include <gtest/gtest.h>

using namespace atom::system;

TEST(GPUInfoTest, GetGPUInfo) {
    std::string gpuInfo = getGPUInfo();
    ASSERT_FALSE(gpuInfo.empty());
}

#ifdef _WIN32
TEST(GPUInfoTest, GetAllMonitorsInfo_Windows) {
    std::vector<MonitorInfo> monitors = getAllMonitorsInfo();
    ASSERT_FALSE(monitors.empty());
    for (const auto& monitor : monitors) {
        ASSERT_FALSE(monitor.model.empty());
        ASSERT_FALSE(monitor.identifier.empty());
        ASSERT_GT(monitor.width, 0);
        ASSERT_GT(monitor.height, 0);
        ASSERT_GT(monitor.refreshRate, 0);
    }
}
#elif defined(__linux__)
TEST(GPUInfoTest, GetAllMonitorsInfo_Linux) {
    std::vector<MonitorInfo> monitors = getAllMonitorsInfo();
    ASSERT_FALSE(monitors.empty());
    for (const auto& monitor : monitors) {
        ASSERT_FALSE(monitor.model.empty());
        ASSERT_FALSE(monitor.identifier.empty());
        ASSERT_GT(monitor.width, 0);
        ASSERT_GT(monitor.height, 0);
        ASSERT_GT(monitor.refreshRate, 0);
    }
}
#elif defined(__APPLE__)
TEST(GPUInfoTest, GetAllMonitorsInfo_Mac) {
    std::vector<MonitorInfo> monitors = getAllMonitorsInfo();
    ASSERT_FALSE(monitors.empty());
    for (const auto& monitor : monitors) {
        ASSERT_FALSE(monitor.identifier.empty());
        ASSERT_GT(monitor.width, 0);
        ASSERT_GT(monitor.height, 0);
        ASSERT_GT(monitor.refreshRate, 0);
    }
}
#endif