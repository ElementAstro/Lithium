#include "addon/compiler.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace lithium;

class CompilerTest : public ::testing::Test {
protected:
    Compiler compiler;
    std::string testCode = R"(
        #include <iostream>
        extern "C" void testFunction() {
            std::cout << "Hello, world!" << std::endl;
        }
    )";

    fs::path tempDir;
    std::string path;

    void SetUp() override {
        // 创建临时目录
        tempDir = fs::temp_directory_path() / fs::path("temp_dir");
        fs::create_directories(tempDir);
        path = tempDir / "compile_options.json";
        // 创建一个 compile_options.json 文件
        std::ofstream optionsFile(path);
        optionsFile << R"({
            "optimization_level": "-O2",
            "cplus_version": "-std=c++20",
            "warnings": "-Wall"
        })";
        optionsFile.close();
    }

    void TearDown() override {
        // 删除临时目录及其内容
        fs::remove_all(tempDir);
    }
};

TEST_F(CompilerTest, FindAvailableCompilers) {
    auto compilers = compiler.getAvailableCompilers();
    ASSERT_FALSE(compilers.empty());
    for (const auto& compilerPath : compilers) {
        std::cout << "Found compiler: " << compilerPath << std::endl;
    }
}

TEST_F(CompilerTest, CompileToSharedLibrary) {
    bool result = compiler.compileToSharedLibrary(testCode, "testModule",
                                                  "testFunction", path);
    ASSERT_TRUE(result);

    // 检查输出文件是否存在
    fs::path outputPath = "atom/global/libtestModule.so";
    ASSERT_TRUE(fs::exists(outputPath));
}

TEST_F(CompilerTest, CompileWithCustomOptions) {
    compiler.addCompileOptions("-DENABLE_DEBUG -g");
    bool result = compiler.compileToSharedLibrary(testCode, "testModuleDebug",
                                                  "testFunction", path);
    ASSERT_TRUE(result);

    // 检查输出文件是否存在
    fs::path outputPath = "atom/global/libtestModuleDebug.so";
    ASSERT_TRUE(fs::exists(outputPath));
}

TEST_F(CompilerTest, CompileSyntaxError) {
    std::string erroneousCode = R"(
        #include <iostream>
        extern "C" void testFunction() {
            std::cout << "Hello, world!"
        }
    )";

    bool result = compiler.compileToSharedLibrary(erroneousCode, "errorModule",
                                                  "testFunction", path);
    ASSERT_FALSE(result);
}

TEST_F(CompilerTest, CompileEmptyCode) {
    bool result = compiler.compileToSharedLibrary("", "emptyModule",
                                                  "testFunction", path);
    ASSERT_FALSE(result);
}
