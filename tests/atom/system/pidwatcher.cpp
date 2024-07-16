#include "atom/system/pidwatcher.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <tlhelp32.h>
// clang-format on
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

using namespace atom::system;
using namespace std::chrono_literals;

// Mock class to simulate process creation and termination
class ProcessSimulator {
public:
    static pid_t createProcess() {
#ifdef _WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        if (!CreateProcess(nullptr, "notepad.exe", nullptr, nullptr, FALSE, 0,
                           nullptr, nullptr, &si, &pi)) {
            return -1;
        }

        return pi.dwProcessId;
#else
        pid_t pid = fork();
        if (pid == 0) {
            // Child process, sleep to simulate running process
            std::this_thread::sleep_for(5s);
            exit(0);
        }
        return pid;
#endif
    }

    static void killProcess(pid_t pid) {
#ifdef _WIN32
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess != nullptr) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
#else
        kill(pid, SIGKILL);
#endif
    }
};

// Test fixture for setting up common test environment
class PidWatcherTest : public ::testing::Test {
protected:
    void SetUp() override { pidWatcher = std::make_unique<PidWatcher>(); }

    void TearDown() override { pidWatcher->stop(); }

    std::unique_ptr<PidWatcher> pidWatcher;
};

// Test setExitCallback and process termination detection
TEST_F(PidWatcherTest, SetExitCallback) {
    bool exitCallbackCalled = false;
    pidWatcher->setExitCallback(
        [&exitCallbackCalled]() { exitCallbackCalled = true; });

    pid_t pid = ProcessSimulator::createProcess();
    ASSERT_NE(pid, -1);

    // Start monitoring the created process
    EXPECT_TRUE(pidWatcher->start(std::to_string(pid)));

    // Simulate process termination
    ProcessSimulator::killProcess(pid);

    // Wait for a short period to ensure the callback gets called
    std::this_thread::sleep_for(2s);
    EXPECT_TRUE(exitCallbackCalled);
}

// Test setMonitorFunction and monitor function execution
TEST_F(PidWatcherTest, SetMonitorFunction) {
    bool monitorCallbackCalled = false;
    pidWatcher->setMonitorFunction(
        [&monitorCallbackCalled]() { monitorCallbackCalled = true; }, 1s);

    pid_t pid = ProcessSimulator::createProcess();
    ASSERT_NE(pid, -1);

    // Start monitoring the created process
    EXPECT_TRUE(pidWatcher->start(std::to_string(pid)));

    // Wait for a short period to ensure the monitor function gets called
    std::this_thread::sleep_for(2s);
    EXPECT_TRUE(monitorCallbackCalled);

    // Clean up
    ProcessSimulator::killProcess(pid);
}

// Test getPidByName
TEST_F(PidWatcherTest, GetPidByName) {
    pid_t pid = ProcessSimulator::createProcess();
    ASSERT_NE(pid, -1);

#ifdef _WIN32
    std::string processName = "notepad.exe";
#else
    std::string processName = std::to_string(pid);
#endif

    pid_t foundPid = pidWatcher->getPidByName(processName);
    EXPECT_EQ(pid, foundPid);

    // Clean up
    ProcessSimulator::killProcess(pid);
}

// Test start function
TEST_F(PidWatcherTest, Start) {
    pid_t pid = ProcessSimulator::createProcess();
    ASSERT_NE(pid, -1);

    EXPECT_TRUE(pidWatcher->start(std::to_string(pid)));

    // Clean up
    ProcessSimulator::killProcess(pid);
}

// Test stop function
TEST_F(PidWatcherTest, Stop) {
    pid_t pid = ProcessSimulator::createProcess();
    ASSERT_NE(pid, -1);

    EXPECT_TRUE(pidWatcher->start(std::to_string(pid)));

    pidWatcher->stop();

    // Clean up
    ProcessSimulator::killProcess(pid);
}

// Test Switch function
TEST_F(PidWatcherTest, Switch) {
    pid_t pid1 = ProcessSimulator::createProcess();
    ASSERT_NE(pid1, -1);
    EXPECT_TRUE(pidWatcher->start(std::to_string(pid1)));

    pid_t pid2 = ProcessSimulator::createProcess();
    ASSERT_NE(pid2, -1);
    EXPECT_TRUE(pidWatcher->Switch(std::to_string(pid2)));

    // Clean up
    ProcessSimulator::killProcess(pid1);
    ProcessSimulator::killProcess(pid2);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
