#include <gtest/gtest.h>
#include "atom/system/storage.hpp"
#include <filesystem>
#include <thread>

using namespace atom::system;
namespace fs = std::filesystem;

// Test fixture for setting up common test environment
class StorageMonitorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Mock callback function to use in tests
void mockCallback(const std::string& path) {
    std::cout << "Callback triggered for path: " << path << std::endl;
}

// Test startMonitoring and stopMonitoring functions
TEST_F(StorageMonitorTest, StartAndStopMonitoring) {
    StorageMonitor monitor;
    EXPECT_TRUE(monitor.startMonitoring());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    monitor.stopMonitoring();
    EXPECT_FALSE(monitor.isRunning());
}

// Test registerCallback and triggerCallbacks functions
TEST_F(StorageMonitorTest, RegisterAndTriggerCallbacks) {
    StorageMonitor monitor;
    monitor.registerCallback(mockCallback);

    // Simulate triggering callbacks
    monitor.triggerCallbacks("/mock/path");

    // Normally, we would have a mechanism to verify that the callback was triggered.
    // For simplicity, this example only prints the callback message.
}

// Test isNewMediaInserted function
TEST_F(StorageMonitorTest, IsNewMediaInserted) {
    StorageMonitor monitor;

    fs::path testPath = "/mock/path";
    monitor.listAllStorage();

    // Manually update storage stats for testing
    monitor.m_storageStats[testPath] = {1000, 500};

    // Simulate a change in storage space
    fs::space_info currentSpace;
    currentSpace.capacity = 2000;
    currentSpace.free = 1000;

    EXPECT_TRUE(monitor.isNewMediaInserted(testPath));
}

// Test listAllStorage function
TEST_F(StorageMonitorTest, ListAllStorage) {
    StorageMonitor monitor;
    monitor.listAllStorage();

    // Verify that storage paths have been listed
    EXPECT_FALSE(monitor.m_storagePaths.empty());
}

// Test listFiles function
TEST_F(StorageMonitorTest, ListFiles) {
    StorageMonitor monitor;

    // Simulate listing files in a mock path
    monitor.listFiles("/mock/path");

    // Normally, we would have a mechanism to verify the file listing.
    // For simplicity, this example only prints the file list.
}
