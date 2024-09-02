#include "addon/toolchain.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

// Mock filesystem for controlled testing
namespace fs = std::filesystem;

class ToolchainTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建一个临时目录以存放测试文件
        temp_dir = fs::temp_directory_path() / "toolchain_test";
        fs::create_directory(temp_dir);
    }

    void TearDown() override {
        // 清理测试创建的临时目录
        fs::remove_all(temp_dir);
    }

    fs::path temp_dir;
};

// 测试 Toolchain 类的基本功能
TEST_F(ToolchainTest, ToolchainInitialization) {
    Toolchain tc("GCC", "gcc", "make", "9.3.0", "/usr/bin/gcc");

    EXPECT_EQ(tc.getName(), "GCC");
}

// 测试 ToolchainManager 类的基本功能
TEST_F(ToolchainTest, ScanForToolchains) {
    ToolchainManager manager;

    // 模拟文件系统的二进制文件
    auto gccPath = temp_dir / "gcc";
    std::ofstream(gccPath) << "#!/bin/bash\necho 'gcc (Ubuntu 9.3.0)'";

    auto clangPath = temp_dir / "clang";
    std::ofstream(clangPath) << "#!/bin/bash\necho 'clang version 10.0.0'";

    // 运行扫描工具链
    manager.scanForToolchains();

    auto toolchains = manager.getToolchains();
    ASSERT_FALSE(toolchains.empty());

    bool foundGcc = false;
    bool foundClang = false;

    for (const auto& tc : toolchains) {
        if (tc.getName().find("gcc") != std::string::npos) {
            foundGcc = true;
        }
        if (tc.getName().find("clang") != std::string::npos) {
            foundClang = true;
        }
    }

    EXPECT_TRUE(foundGcc);
    EXPECT_TRUE(foundClang);
}

// 测试 ToolchainManager 选择工具链的功能
TEST_F(ToolchainTest, SelectToolchain) {
    ToolchainManager manager;
    manager.scanForToolchains();

    EXPECT_TRUE(manager.selectToolchain("gcc"));
    EXPECT_FALSE(manager.selectToolchain("non_existent_toolchain"));
}

// 测试 ToolchainManager 保存和加载配置的功能
TEST_F(ToolchainTest, SaveAndLoadConfig) {
    ToolchainManager manager;

    // 添加工具链并保存配置
    manager.scanForToolchains();
    manager.saveConfig(temp_dir / "config.txt");

    // 清除并重新加载配置
    manager.loadConfig(temp_dir / "config.txt");

    // 验证配置是否正确加载
    EXPECT_TRUE(manager.selectToolchain("gcc"));
}

// 测试 getCompilerVersion 的边缘情况
/*
TEST_F(ToolchainTest, GetCompilerVersion) {
    ToolchainManager manager;

    // 模拟一个无效的编译器路径
    auto version = manager.getCompilerVersion("/invalid/path/to/compiler");

    EXPECT_EQ(version, "Unknown version");

    // 模拟一个返回版本信息的编译器
    auto valid_path = temp_dir / "valid_compiler";
    std::ofstream(valid_path) << "#!/bin/bash\necho 'Compiler version 1.0.0'";
    fs::permissions(valid_path, fs::perms::owner_exec);

    version = manager.getCompilerVersion(valid_path.string());
    EXPECT_NE(version.find("1.0.0"), std::string::npos);
}
*/

// 测试 ToolchainManager 中的 getAvailableCompilers
TEST_F(ToolchainTest, GetAvailableCompilers) {
    ToolchainManager manager;

    manager.scanForToolchains();

    auto compilers = manager.getAvailableCompilers();
    EXPECT_FALSE(compilers.empty());

    bool foundGcc = false;
    bool foundClang = false;

    for (const auto& compiler : compilers) {
        if (compiler.find("gcc") != std::string::npos) {
            foundGcc = true;
        }
        if (compiler.find("clang") != std::string::npos) {
            foundClang = true;
        }
    }

    EXPECT_TRUE(foundGcc);
    EXPECT_TRUE(foundClang);
}
