#include "atom/utils/qprocess.hpp"
#include <gtest/gtest.h>
#include "atom/error/exception.hpp"

using namespace atom::utils;

TEST(MyProcessTest, BasicFunctionality) {
    QProcess process;
    process.start("/bin/echo", {"Hello", "World"});

    ASSERT_TRUE(process.waitForStarted(1000))
        << "Process should start within 1 second";
    std::string output = process.readAllStandardOutput();
    ASSERT_EQ(output, "Hello World\n")
        << "Standard output should match expected";

    ASSERT_TRUE(process.waitForFinished(1000))
        << "Process should finish within 1 second";
    ASSERT_FALSE(process.isRunning())
        << "Process should not be running after finishing";
}

TEST(MyProcessTest, WorkingDirectory) {
    QProcess process;
    process.setWorkingDirectory("/tmp");
    process.start("/bin/pwd", {});

    ASSERT_TRUE(process.waitForStarted(1000));
    std::string output = process.readAllStandardOutput();
    ASSERT_EQ(output, "/tmp\n") << "Process should run in the /tmp directory";

    ASSERT_TRUE(process.waitForFinished(1000));
}

TEST(MyProcessTest, EnvironmentVariables) {
    QProcess process;
    process.setEnvironment({"TEST_VAR=HelloWorld"});
    process.start("/bin/sh", {"-c", "echo $TEST_VAR"});

    ASSERT_TRUE(process.waitForStarted(1000));
    std::string output = process.readAllStandardOutput();
    ASSERT_EQ(output, "HelloWorld\n")
        << "Environment variable should be correctly set";

    ASSERT_TRUE(process.waitForFinished(1000));
}

TEST(MyProcessTest, StartAlreadyRunningProcess) {
    QProcess process;
    process.start("/bin/sleep", {"1"});
    ASSERT_TRUE(process.waitForStarted(1000));

    EXPECT_THROW(process.start("/bin/sleep", {"1"}),
                 atom::error::SystemCollapse)
        << "Starting an already running process should throw an error";

    process.terminate();
}

TEST(MyProcessTest, StartInvalidProgram) {
    QProcess process;
    EXPECT_THROW(process.start("/bin/nonexistentprogram", {}),
                 atom::error::SystemCollapse)
        << "Starting a nonexistent program should throw an error";
}

TEST(MyProcessTest, CallMethodsBeforeStart) {
    QProcess process;
    EXPECT_THROW(process.write("Test"), atom::error::SystemCollapse)
        << "Writing to a non-started process should throw an error";
    EXPECT_THROW(process.readAllStandardOutput(), atom::error::SystemCollapse)
        << "Reading from a non-started process should throw an error";
    EXPECT_THROW(process.readAllStandardError(), atom::error::SystemCollapse)
        << "Reading error from a non-started process should throw an error";
}

TEST(MyProcessTest, ReadOutputAfterFinish) {
    QProcess process;
    process.start("/bin/echo", {"Hello"});
    ASSERT_TRUE(process.waitForStarted(1000));
    ASSERT_TRUE(process.waitForFinished(1000));

    std::string output = process.readAllStandardOutput();
    ASSERT_EQ(output, "Hello\n")
        << "Output should be readable after process finishes";
}

TEST(MyProcessTest, WriteLargeDataToStandardInput) {
    QProcess process;
    process.start("/bin/cat", {});

    ASSERT_TRUE(process.waitForStarted(1000));

    std::string largeData(10 * 1024 * 1024, 'A');  // 10 MB of 'A'
    ASSERT_NO_THROW(process.write(largeData))
        << "Writing large data should not throw";

    process.terminate();
}

TEST(MyProcessTest, LongRunningProcess) {
    QProcess process;
    process.start("/bin/sleep", {"3"});

    ASSERT_TRUE(process.waitForStarted(1000))
        << "Process should start within 1 second";
    ASSERT_TRUE(process.isRunning()) << "Process should be running";

    ASSERT_TRUE(process.waitForFinished(4000))
        << "Process should finish within 4 seconds";
    ASSERT_FALSE(process.isRunning())
        << "Process should not be running after finishing";
}

TEST(MyProcessTest, SpecialCharactersInCommand) {
    QProcess process;
    process.start("/bin/echo", {"$HOME", "`ls`", "\"quoted\""});

    ASSERT_TRUE(process.waitForStarted(1000));
    std::string output = process.readAllStandardOutput();
    ASSERT_EQ(output, "$HOME `ls` \"quoted\"\n")
        << "Special characters should be handled correctly";

    ASSERT_TRUE(process.waitForFinished(1000));
}

TEST(MyProcessTest, LongEnvironmentVariable) {
    QProcess process;
    std::string longVar(1024, 'A');
    process.setEnvironment({"LONG_VAR=" + longVar});
    process.start("/bin/sh", {"-c", "echo ${LONG_VAR}"});

    ASSERT_TRUE(process.waitForStarted(1000));
    std::string output = process.readAllStandardOutput();
    ASSERT_EQ(output, longVar + "\n")
        << "Long environment variable should be handled correctly";

    ASSERT_TRUE(process.waitForFinished(1000));
}
