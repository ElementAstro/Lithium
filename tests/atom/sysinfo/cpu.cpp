#include "atom/sysinfo/cpu.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>


#ifdef _WIN32
#include <windows.h>
#include <pdh.h>
#else
#include <fstream>
#endif

using namespace atom::system;
using namespace testing;

#ifdef _WIN32
class MockWindowsApi {
public:
    MOCK_METHOD(PDH_STATUS, PdhOpenQuery, (LPCWSTR, DWORD_PTR, PDH_HQUERY*),
                ());
    MOCK_METHOD(PDH_STATUS, PdhAddCounter,
                (PDH_HQUERY, LPCWSTR, DWORD_PTR, PDH_HCOUNTER*), ());
    MOCK_METHOD(PDH_STATUS, PdhCollectQueryData, (PDH_HQUERY), ());
    MOCK_METHOD(PDH_STATUS, PdhGetFormattedCounterValue,
                (PDH_HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE), ());
    MOCK_METHOD(PDH_STATUS, PdhCloseQuery, (PDH_HQUERY), ());
};

MockWindowsApi* mockWindowsApi;

PDH_STATUS WINAPI MockPdhOpenQuery(LPCWSTR, DWORD_PTR, PDH_HQUERY* query) {
    return mockWindowsApi->PdhOpenQuery(nullptr, 0, query);
}

PDH_STATUS WINAPI MockPdhAddCounter(PDH_HQUERY query, LPCWSTR counterPath,
                                    DWORD_PTR userData, PDH_HCOUNTER* counter) {
    return mockWindowsApi->PdhAddCounter(query, counterPath, userData, counter);
}

PDH_STATUS WINAPI MockPdhCollectQueryData(PDH_HQUERY query) {
    return mockWindowsApi->PdhCollectQueryData(query);
}

PDH_STATUS WINAPI MockPdhGetFormattedCounterValue(PDH_HCOUNTER counter,
                                                  DWORD format, LPDWORD type,
                                                  PPDH_FMT_COUNTERVALUE value) {
    return mockWindowsApi->PdhGetFormattedCounterValue(counter, format, type,
                                                       value);
}

PDH_STATUS WINAPI MockPdhCloseQuery(PDH_HQUERY query) {
    return mockWindowsApi->PdhCloseQuery(query);
}

void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    PdhOpenQuery = MockPdhOpenQuery;
    PdhAddCounter = MockPdhAddCounter;
    PdhCollectQueryData = MockPdhCollectQueryData;
    PdhGetFormattedCounterValue = MockPdhGetFormattedCounterValue;
    PdhCloseQuery = MockPdhCloseQuery;
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

class CpuTest : public ::testing::Test {
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
TEST_F(CpuTest, GetCurrentCpuUsage_Windows) {
    PDH_HQUERY query;
    PDH_HCOUNTER counter;
    PDH_FMT_COUNTERVALUE counterValue;
    counterValue.doubleValue = 25.0;

    EXPECT_CALL(*mockWindowsApi, PdhOpenQuery(_, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(query), Return(ERROR_SUCCESS)));
    EXPECT_CALL(*mockWindowsApi, PdhAddCounter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(counter), Return(ERROR_SUCCESS)));
    EXPECT_CALL(*mockWindowsApi, PdhCollectQueryData(_))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*mockWindowsApi, PdhGetFormattedCounterValue(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<3>(counterValue), Return(ERROR_SUCCESS)));
    EXPECT_CALL(*mockWindowsApi, PdhCloseQuery(_))
        .WillOnce(Return(ERROR_SUCCESS));

    float cpuUsage = getCurrentCpuUsage();
    EXPECT_EQ(cpuUsage, 25.0f);
}

#else

TEST_F(CpuTest, GetCurrentCpuUsage_Linux) {
    std::string mockCpuStatData =
        "cpu  4705 150 2268 225732 1298 0 130 0 0 0\n";
    EXPECT_CALL(*mockFileReader, ReadFile("/proc/stat"))
        .WillOnce(Return(mockCpuStatData));

    float cpuUsage = getCurrentCpuUsage();
    EXPECT_NEAR(cpuUsage, 2.06,
                0.01);  // Adjust the expected value based on the mock data
}

#endif
