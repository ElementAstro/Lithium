#include "atom/system/env.hpp"
#include <gtest/gtest.h>
#include <cstdlib>  // for setenv, getenv, unsetenv
#include <filesystem>
#include <memory>
#include <string>

using namespace atom::utils;
namespace fs = std::filesystem;

// Helper function to initialize Env with mock arguments
std::shared_ptr<Env> createEnv(int argc, const char* argv[]) {
    return Env::createShared(argc, const_cast<char**>(argv));
}

// Test fixture for setting up common test environment
class EnvTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }

    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test add and get functions
TEST_F(EnvTest, AddAndGet) {
    const char* argv[] = {"program", "-key", "value"};
    auto env = createEnv(3, argv);
    EXPECT_EQ(env->get("key"), "value");
}

// Test has function
TEST_F(EnvTest, Has) {
    const char* argv[] = {"program", "-key", "value"};
    auto env = createEnv(3, argv);
    EXPECT_TRUE(env->has("key"));
    EXPECT_FALSE(env->has("nonexistent_key"));
}

// Test del function
TEST_F(EnvTest, Del) {
    const char* argv[] = {"program", "-key", "value"};
    auto env = createEnv(3, argv);
    env->del("key");
    EXPECT_FALSE(env->has("key"));
}

// Test addHelp and printHelp functions
TEST_F(EnvTest, AddHelp) {
    const char* argv[] = {"program"};
    auto env = createEnv(1, argv);
    env->addHelp("key", "description");
    // This will not actually check output but should be run to ensure no errors
    env->printHelp();
}

// Test setEnv and getEnv functions
TEST_F(EnvTest, SetAndGetEnv) {
    const char* argv[] = {"program"};
    auto env = createEnv(1, argv);

    env->setEnv("TEST_ENV_VAR", "test_value");
    EXPECT_EQ(env->getEnv("TEST_ENV_VAR"), "test_value");

    // Clean up
    env->del("TEST_ENV_VAR");
}

// Test getAbsolutePath function
TEST_F(EnvTest, GetAbsolutePath) {
    const char* argv[] = {"program"};
    auto env = createEnv(1, argv);
    std::string relativePath = "relative/path";
    std::string absolutePath = env->getAbsolutePath(relativePath);
    EXPECT_TRUE(fs::path(absolutePath).is_absolute());
}

// Test getAbsoluteWorkPath function
TEST_F(EnvTest, GetAbsoluteWorkPath) {
    const char* argv[] = {"program"};
    auto env = createEnv(1, argv);
    std::string relativePath = "work/path";
    std::string absoluteWorkPath = env->getAbsoluteWorkPath(relativePath);
    EXPECT_EQ(absoluteWorkPath, "/");
}

// Test getConfigPath function
TEST_F(EnvTest, GetConfigPath) {
    const char* argv[] = {"program", "-c", "config_path"};
    auto env = createEnv(3, argv);
    std::string configPath = env->getConfigPath();
    EXPECT_TRUE(fs::path(configPath).is_absolute());
    EXPECT_EQ(configPath, env->getAbsolutePath("config_path"));
}

// Test Environ function
TEST_F(EnvTest, Environ) {
    const char* argv[] = {"program"};
    auto env = createEnv(1, argv);
    auto environmentVariables = env->Environ();
    EXPECT_FALSE(environmentVariables.empty());
    EXPECT_NE(environmentVariables.find("PATH"), environmentVariables.end());
}