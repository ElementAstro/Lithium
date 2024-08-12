#include "atom/system/user.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>


using namespace atom::system;

// Test fixture for setting up common test environment
class UserTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test isRoot function
TEST_F(UserTest, IsRoot) {
#ifdef _WIN32
    EXPECT_FALSE(
        isRoot());  // This test might fail if run with elevated privileges
#else
    if (getuid() == 0) {
        EXPECT_TRUE(isRoot());
    } else {
        EXPECT_FALSE(isRoot());
    }
#endif
}

// Test getUserGroups function
TEST_F(UserTest, GetUserGroups) {
    auto groups = getUserGroups();
    EXPECT_FALSE(groups.empty());

    for (const auto& group : groups) {
        std::wcout << L"Group: " << group << std::endl;
    }
}

// Test getUsername function
TEST_F(UserTest, GetUsername) {
    std::string username = getUsername();
    EXPECT_FALSE(username.empty());
    std::cout << "Username: " << username << std::endl;
}

// Test getHostname function
TEST_F(UserTest, GetHostname) {
    std::string hostname = getHostname();
    EXPECT_FALSE(hostname.empty());
    std::cout << "Hostname: " << hostname << std::endl;
}

// Test getUserId function
TEST_F(UserTest, GetUserId) {
    int userId = getUserId();
    EXPECT_GT(userId, 0);
    std::cout << "User ID: " << userId << std::endl;
}

// Test getGroupId function
TEST_F(UserTest, GetGroupId) {
    int groupId = getGroupId();
    EXPECT_GT(groupId, 0);
    std::cout << "Group ID: " << groupId << std::endl;
}

// Test getHomeDirectory function
TEST_F(UserTest, GetHomeDirectory) {
    std::string homeDir = getHomeDirectory();
    EXPECT_FALSE(homeDir.empty());
    std::cout << "Home Directory: " << homeDir << std::endl;
}

// Test getLoginShell function
TEST_F(UserTest, GetLoginShell) {
    std::string loginShell = getLoginShell();
    EXPECT_FALSE(loginShell.empty());
    std::cout << "Login Shell: " << loginShell << std::endl;
}

// Test getLogin function
TEST_F(UserTest, GetLogin) {
    std::string login = getLogin();
    EXPECT_FALSE(login.empty());
    std::cout << "Login: " << login << std::endl;
}

#ifdef _WIN32
// Test getUserProfileDirectory function
TEST_F(UserTest, GetUserProfileDirectory) {
    std::string profileDir = getUserProfileDirectory();
    EXPECT_FALSE(profileDir.empty());
    std::cout << "User Profile Directory: " << profileDir << std::endl;
}
#endif
