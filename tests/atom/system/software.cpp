#include "atom/system/software.hpp"
#include <gtest/gtest.h>
#include <fstream>


using namespace atom::system;

// Test fixture for setting up common test environment
class SoftwareTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test getAppVersion function
TEST_F(SoftwareTest, GetAppVersion) {
#ifdef _WIN32
    std::string version = getAppVersion(R"(C:\Windows\System32\notepad.exe)");
    EXPECT_FALSE(version.empty());
#elif defined(__APPLE__)
    std::string version = getAppVersion("/Applications/Safari.app");
    EXPECT_FALSE(version.empty());
#elif defined(__ANDROID__)
    std::string version = getAppVersion("/data/app/com.example.myapp");
    EXPECT_FALSE(version.empty());
#else
    std::ofstream file("test_app");
    file << "@(#) 1.2.3 ";
    file.close();

    std::string version = getAppVersion("./test_app");
    EXPECT_EQ(version, "1.2.3");

    std::remove("test_app");
#endif
}

// Test getAppPermissions function
TEST_F(SoftwareTest, GetAppPermissions) {
#ifdef _WIN32
    std::string path = R"(C:\Windows\System32\notepad.exe)";
#elif defined(__APPLE__) || defined(__linux__)
    std::string path = "/bin/ls";
#elif defined(__ANDROID__)
    std::string path = "/data/app/com.example.myapp";
#endif

    auto permissions = getAppPermissions(path);
    EXPECT_FALSE(permissions.empty());
}

// Test getAppPath function
TEST_F(SoftwareTest, GetAppPath) {
#ifdef _WIN32
    std::string software_name = "notepad.exe";
    auto path = getAppPath(software_name);
    EXPECT_FALSE(path.empty());
#elif defined(__APPLE__)
    std::string software_name = "Safari.app";
    auto path = getAppPath(software_name);
    EXPECT_FALSE(path.empty());
#elif defined(__linux__)
    std::string software_name = "ls";
    auto path = getAppPath(software_name);
    EXPECT_FALSE(path.empty());
#endif
}

// Test checkSoftwareInstalled function
TEST_F(SoftwareTest, CheckSoftwareInstalled) {
#ifdef _WIN32
    std::string software_name = "Notepad++";
    EXPECT_TRUE(checkSoftwareInstalled(software_name));
#elif defined(__APPLE__)
    std::string software_name = "Safari";
    EXPECT_TRUE(checkSoftwareInstalled(software_name));
#elif defined(__linux__)
    std::string software_name = "ls";
    EXPECT_TRUE(checkSoftwareInstalled(software_name));
#endif
}
