#include "addon/sandbox.hpp"
#include "gtest/gtest.h"

class SandboxTest : public ::testing::Test {
protected:
    lithium::Sandbox sandbox;

    void SetUp() override {
        // Common setup can go here
    }

    void TearDown() override {
        // Common cleanup can go here
    }
};

TEST_F(SandboxTest, SetTimeLimit) {
    EXPECT_TRUE(sandbox.setTimeLimit(1000));
    EXPECT_EQ(sandbox.getTimeUsed(), 0);  // Initially, time used should be 0
}

TEST_F(SandboxTest, SetMemoryLimit) {
    EXPECT_TRUE(sandbox.setMemoryLimit(10240));  // 10 MB
    EXPECT_EQ(sandbox.getMemoryUsed(),
              0);  // Initially, memory used should be 0
}

TEST_F(SandboxTest, SetRootDirectory) {
    EXPECT_TRUE(sandbox.setRootDirectory("/path/to/sandbox/root"));
}

TEST_F(SandboxTest, SetUserId) {
    EXPECT_TRUE(sandbox.setUserId(
        1000));  // Assuming 1000 is a valid user ID on the system
}

TEST_F(SandboxTest, SetProgramPath) {
    EXPECT_TRUE(sandbox.setProgramPath("/path/to/executable"));
}

TEST_F(SandboxTest, SetProgramArgs) {
    EXPECT_TRUE(sandbox.setProgramArgs({"arg1", "arg2"}));
}

TEST_F(SandboxTest, RunSuccess) {
    sandbox.setTimeLimit(1000);     // 1 second
    sandbox.setMemoryLimit(10240);  // 10 MB
    sandbox.setRootDirectory("/path/to/sandbox/root");
    sandbox.setUserId(1000);  // Assuming 1000 is a valid user ID on the system
    sandbox.setProgramPath("/path/to/successful/executable");
    sandbox.setProgramArgs({"arg1", "arg2"});

    EXPECT_TRUE(sandbox.run());
    EXPECT_GT(sandbox.getTimeUsed(), 0);  // Time used should be greater than 0
    EXPECT_GT(sandbox.getMemoryUsed(),
              0);  // Memory used should be greater than 0
}

TEST_F(SandboxTest, RunFailure) {
    sandbox.setTimeLimit(1000);     // 1 second
    sandbox.setMemoryLimit(10240);  // 10 MB
    sandbox.setRootDirectory("/path/to/sandbox/root");
    sandbox.setUserId(1000);  // Assuming 1000 is a valid user ID on the system
    sandbox.setProgramPath("/path/to/failing/executable");
    sandbox.setProgramArgs({"arg1", "arg2"});

    EXPECT_FALSE(sandbox.run());
}
