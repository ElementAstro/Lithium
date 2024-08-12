#include "atom/sysinfo/memory.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>


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
    MOCK_METHOD(BOOL, GlobalMemoryStatusEx, (LPMEMORYSTATUSEX), ());
};

MockWindowsApi* mockWindowsApi;

BOOL WINAPI MockGlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer) {
    return mockWindowsApi->GlobalMemoryStatusEx(lpBuffer);
}

void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    GlobalMemoryStatusEx = MockGlobalMemoryStatusEx;
}

void cleanupMockWindowsApi() { delete mockWindowsApi; }

#else
class MockFileReader {
public:
    MOCK_METHOD(std::string, ReadFile, (const std::string&), ());
};

MockFileReader* mockFileReader;

std::string MockReadFile(const std::string& path) {
    return mockFileReader->ReadFile(path);
}

void setupMockFileReader() { mockFileReader = new MockFileReader(); }

void cleanupMockFileReader() { delete mockFileReader; }
#endif

class MemoryTest : public ::testing::Test {
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
TEST_F(MemoryTest, GetMemoryUsage_Windows) {
    MEMORYSTATUSEX mockStatus;
    mockStatus.ullTotalPhys = 8 * 1024 * 1024 * 1024ULL;  // 8GB
    mockStatus.ullAvailPhys = 4 * 1024 * 1024 * 1024ULL;  // 4GB

    EXPECT_CALL(*mockWindowsApi, GlobalMemoryStatusEx(_))
        .WillOnce(DoAll(SetArgPointee<0>(mockStatus), Return(TRUE)));

    float memoryUsage = getMemoryUsage();
    EXPECT_NEAR(memoryUsage, 50.0, 1e-5);
}

TEST_F(MemoryTest, GetTotalMemorySize_Windows) {
    MEMORYSTATUSEX mockStatus;
    mockStatus.ullTotalPhys = 16 * 1024 * 1024 * 1024ULL;  // 16GB

    EXPECT_CALL(*mockWindowsApi, GlobalMemoryStatusEx(_))
        .WillOnce(DoAll(SetArgPointee<0>(mockStatus), Return(TRUE)));

    unsigned long long totalMemorySize = getTotalMemorySize();
    EXPECT_EQ(totalMemorySize, 16 * 1024 * 1024 * 1024ULL);
}

#else
TEST_F(MemoryTest, GetMemoryUsage_Linux) {
    std::string mockMeminfo =
        "MemTotal:       16384 kB\n"
        "MemFree:         8192 kB\n"
        "Buffers:         1024 kB\n"
        "Cached:          2048 kB\n";

    EXPECT_CALL(*mockFileReader, ReadFile("/proc/meminfo"))
        .WillOnce(Return(mockMeminfo));

    float memoryUsage = getMemoryUsage();
    EXPECT_NEAR(memoryUsage, 31.25,
                1e-5);  // (16384 - 8192 - 1024 - 2048) / 16384 * 100
}

TEST_F(MemoryTest, GetTotalMemorySize_Linux) {
    long pages = 4096;     // Number of pages
    long pageSize = 4096;  // Size of a page in bytes

    EXPECT_CALL(*mockFileReader, ReadFile("/proc/meminfo"))
        .WillOnce(Return(""));

    unsigned long long totalMemorySize = getTotalMemorySize();
    EXPECT_EQ(totalMemorySize,
              static_cast<unsigned long long>(pages) * pageSize);
}

#endif
