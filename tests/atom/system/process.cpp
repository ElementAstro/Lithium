#include <gtest/gtest.h>
#include "atom/system/process.hpp"
#include <fstream>
#include <thread>

using namespace atom::system;
namespace fs = std::filesystem;

// Helper function to clean up test files
void cleanupTestFiles() {
    fs::remove("test_script.sh");
    fs::remove("test_script.ps1");
}

// Helper function to create a sample shell script
void createTestScript() {
#ifdef _WIN32
    std::ofstream script("test_script.ps1");
    script << "Write-Host 'Test Script Running'";
    script.close();
#else
    std::ofstream script("test_script.sh");
    script << "#!/bin/bash\n";
    script << "echo 'Test Script Running'";
    script.close();
    chmod("test_script.sh", 0755);
#endif
}

// Test fixture for setting up common test environment
class ProcessManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Cleanup before each test
        cleanupTestFiles();
        createTestScript();
    }

    void TearDown() override {
        // Cleanup after each test
        cleanupTestFiles();
    }

    ProcessManager manager{10}; // Initialize ProcessManager with max 10 processes
};

// Test createProcess and hasProcess functions
TEST_F(ProcessManagerTest, CreateAndCheckProcess) {
    std::string command;
#ifdef _WIN32
    command = "powershell.exe -File test_script.ps1";
#else
    command = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.createProcess(command, "TestProcess"));
    EXPECT_TRUE(manager.hasProcess("TestProcess"));
}

// Test runScript function
TEST_F(ProcessManagerTest, RunScript) {
    std::string script;
#ifdef _WIN32
    script = "Write-Host 'Test Script Running'";
#else
    script = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.runScript(script, "TestScript"));
    EXPECT_TRUE(manager.hasProcess("TestScript"));
}

// Test terminateProcess function
TEST_F(ProcessManagerTest, TerminateProcess) {
    std::string command;
#ifdef _WIN32
    command = "powershell.exe -File test_script.ps1";
#else
    command = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.createProcess(command, "TestProcess"));

    auto processes = manager.getRunningProcesses();
    ASSERT_FALSE(processes.empty());

    int pid = processes[0].pid;
    EXPECT_TRUE(manager.terminateProcess(pid));
}

// Test terminateProcessByName function
TEST_F(ProcessManagerTest, TerminateProcessByName) {
    std::string command;
#ifdef _WIN32
    command = "powershell.exe -File test_script.ps1";
#else
    command = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.createProcess(command, "TestProcess"));

    EXPECT_TRUE(manager.terminateProcessByName("TestProcess"));
}

// Test getRunningProcesses function
TEST_F(ProcessManagerTest, GetRunningProcesses) {
    std::string command;
#ifdef _WIN32
    command = "powershell.exe -File test_script.ps1";
#else
    command = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.createProcess(command, "TestProcess"));

    auto processes = manager.getRunningProcesses();
    EXPECT_FALSE(processes.empty());
    EXPECT_EQ(processes[0].name, "TestProcess");
}

// Test getProcessOutput function
TEST_F(ProcessManagerTest, GetProcessOutput) {
    std::string script;
#ifdef _WIN32
    script = "Write-Host 'Test Script Running'";
#else
    script = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.runScript(script, "TestScript"));

    // Allow some time for the script to execute
    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto output = manager.getProcessOutput("TestScript");
    ASSERT_FALSE(output.empty());
    EXPECT_EQ(output[0], "Test Script Running");
}

// Test waitForCompletion function
TEST_F(ProcessManagerTest, WaitForCompletion) {
    std::string command;
#ifdef _WIN32
    command = "powershell.exe -File test_script.ps1";
#else
    command = "./test_script.sh";
#endif

    EXPECT_TRUE(manager.createProcess(command, "TestProcess"));

    manager.waitForCompletion();

    auto processes = manager.getRunningProcesses();
    EXPECT_TRUE(processes.empty());
}

// Test getSelfProcessInfo function
TEST_F(ProcessManagerTest, GetSelfProcessInfo) {
    auto info = getSelfProcessInfo();
    EXPECT_GT(info.pid, 0);
    EXPECT_FALSE(info.path.empty());
    EXPECT_FALSE(info.name.empty());
    EXPECT_EQ(info.status, "Running");
}

// Test getParentProcessId function
TEST_F(ProcessManagerTest, GetParentProcessId) {
    int pid = getpid();
    int ppid = getParentProcessId(pid);
    EXPECT_GT(ppid, 0);
}
