#include "atom/sysinfo/os.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>


#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <fstream>
#elif __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <sys/utsname.h>

#endif

using namespace atom::system;

#ifdef _WIN32

class MockWindowsApi {
public:
    MOCK_METHOD(BOOL, GetVersionExA, (LPOSVERSIONINFO), ());
    MOCK_METHOD(BOOL, GetComputerNameA, (LPSTR, LPDWORD), ());
};

MockWindowsApi* mockWindowsApi;

BOOL WINAPI MockGetVersionExA(LPOSVERSIONINFO lpVersionInfo) {
    return mockWindowsApi->GetVersionExA(lpVersionInfo);
}

BOOL WINAPI MockGetComputerNameA(LPSTR lpBuffer, LPDWORD lpnSize) {
    return mockWindowsApi->GetComputerNameA(lpBuffer, lpnSize);
}

void setupMockWindowsApi() {
    mockWindowsApi = new MockWindowsApi();
    GetVersionExA = MockGetVersionExA;
    GetComputerNameA = MockGetComputerNameA;
}

void cleanupMockWindowsApi() { delete mockWindowsApi; }

#elif __linux__

class MockLinuxApi {
public:
    MOCK_METHOD(int, gethostname, (char*, size_t), ());
};

MockLinuxApi* mockLinuxApi;

int MockGethostname(char* name, size_t len) {
    return mockLinuxApi->gethostname(name, len);
}

void setupMockLinuxApi() {
    mockLinuxApi = new MockLinuxApi();
    gethostname = MockGethostname;
}

void cleanupMockLinuxApi() { delete mockLinuxApi; }

#elif __APPLE__

class MockAppleApi {
public:
    MOCK_METHOD(int, uname, (struct utsname*), ());
    MOCK_METHOD(CFStringRef, SCDynamicStoreCopyComputerName,
                (SCDynamicStoreRef, CFStringEncoding*), ());
};

MockAppleApi* mockAppleApi;

int MockUname(struct utsname* buf) { return mockAppleApi->uname(buf); }

CFStringRef MockSCDynamicStoreCopyComputerName(SCDynamicStoreRef store,
                                               CFStringEncoding* nameEncoding) {
    return mockAppleApi->SCDynamicStoreCopyComputerName(store, nameEncoding);
}

void setupMockAppleApi() {
    mockAppleApi = new MockAppleApi();
    uname = MockUname;
    SCDynamicStoreCopyComputerName = MockSCDynamicStoreCopyComputerName;
}

void cleanupMockAppleApi() { delete mockAppleApi; }

#endif

class OSTest : public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        setupMockWindowsApi();
#elif __linux__
        setupMockLinuxApi();
#elif __APPLE__
        setupMockAppleApi();
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        cleanupMockWindowsApi();
#elif __linux__
        cleanupMockLinuxApi();
#elif __APPLE__
        cleanupMockAppleApi();
#endif
    }
};

#ifdef _WIN32

TEST_F(OSTest, GetOperatingSystemInfo_Windows) {
    OSVERSIONINFOEX osvi;
    osvi.dwMajorVersion = 10;
    osvi.dwMinorVersion = 0;
    osvi.dwBuildNumber = 19041;

    EXPECT_CALL(*mockWindowsApi, GetVersionExA(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(osvi),
                                 testing::Return(TRUE)));

    std::string computerName = "TestComputer";
    DWORD size = computerName.size();

    EXPECT_CALL(*mockWindowsApi, GetComputerNameA(testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SetArrayArgument<0>(
                                     computerName.begin(), computerName.end()),
                                 testing::SetArgPointee<1>(size),
                                 testing::Return(TRUE)));

    OperatingSystemInfo osInfo = getOperatingSystemInfo();

    EXPECT_EQ(osInfo.osName, "Windows");
    EXPECT_EQ(osInfo.osVersion, "10.0 (Build 19041)");
    EXPECT_EQ(osInfo.computerName, "TestComputer");
}

#elif __linux__

TEST_F(OSTest, GetOperatingSystemInfo_Linux) {
    std::string osReleaseContent = "PRETTY_NAME=\"Ubuntu 20.04 LTS\"";
    std::string kernelVersionContent =
        "Linux version 5.4.0-42-generic (buildd@lcy01-amd64-021)";

    EXPECT_CALL(*mockLinuxApi, gethostname(testing::_, testing::_))
        .WillOnce(testing::DoAll(
            testing::SetArrayArgument<0>("TestComputer", "TestComputer" + 12),
            testing::Return(0)));

    std::ofstream osReleaseFile("/etc/os-release");
    osReleaseFile << osReleaseContent;
    osReleaseFile.close();

    std::ofstream kernelVersionFile("/proc/version");
    kernelVersionFile << kernelVersionContent;
    kernelVersionFile.close();

    OperatingSystemInfo osInfo = getOperatingSystemInfo();

    EXPECT_EQ(osInfo.osName, "\"Ubuntu 20.04 LTS\"");
    EXPECT_EQ(osInfo.kernelVersion, "Linux");
    EXPECT_EQ(osInfo.computerName, "TestComputer");
}

#elif __APPLE__

TEST_F(OSTest, GetOperatingSystemInfo_MacOS) {
    struct utsname info;
    strcpy(info.sysname, "Darwin");
    strcpy(info.release, "20.3.0");
    strcpy(info.version,
           "Darwin Kernel Version 20.3.0: Thu Jan 21 22:06:51 PST 2021; "
           "root:xnu-7195.81.3~1/RELEASE_X86_64");

    EXPECT_CALL(*mockAppleApi, uname(testing::_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(info),
                                 testing::Return(0)));

    CFStringRef name = CFSTR("TestComputer");

    EXPECT_CALL(*mockAppleApi,
                SCDynamicStoreCopyComputerName(testing::_, testing::_))
        .WillOnce(testing::Return(name));

    OperatingSystemInfo osInfo = getOperatingSystemInfo();

    EXPECT_EQ(osInfo.osName, "Darwin");
    EXPECT_EQ(osInfo.osVersion, "20.3.0");
    EXPECT_EQ(osInfo.kernelVersion,
              "Darwin Kernel Version 20.3.0: Thu Jan 21 22:06:51 PST 2021; "
              "root:xnu-7195.81.3~1/RELEASE_X86_64");
    EXPECT_EQ(osInfo.computerName, "TestComputer");

    CFRelease(name);
}

#endif
