#include "atom/utils/argsview.hpp"
#include <gtest/gtest.h>

using namespace atom::utils;

class ArgsViewTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 设置测试用的命令行参数
        argc = 7;
        argv = new char*[argc];
        argv[0] = const_cast<char*>("program");
        argv[1] = const_cast<char*>("--name");
        argv[2] = const_cast<char*>("test");
        argv[3] = const_cast<char*>("--age");
        argv[4] = const_cast<char*>("30");
        argv[5] = const_cast<char*>("--verbose");
        argv[6] = const_cast<char*>("positional_arg");

        argsView = new ArgsView(argc, argv);
        argsView->addArgument("--name", "Name of the user", true);
        argsView->addArgument("--age", "Age of the user");
        argsView->addFlag("--verbose", "Enable verbose mode");
        argsView->addPositionalArgument("positional_arg",
                                        "A positional argument");
    }

    void TearDown() override {
        delete[] argv;
        delete argsView;
    }

    int argc;
    char** argv;
    ArgsView* argsView;
};

// 测试获取参数
TEST_F(ArgsViewTest, GetArgument) {
    auto name = argsView->get("--name");
    ASSERT_TRUE(name.has_value());
    EXPECT_EQ(name.value(), "test");

    auto age = argsView->get("--age");
    ASSERT_TRUE(age.has_value());
    EXPECT_EQ(age.value(), "30");
}

// 测试获取缺失的参数
TEST_F(ArgsViewTest, GetMissingArgument) {
    auto missing = argsView->get("--missing");
    EXPECT_FALSE(missing.has_value());
}

// 测试获取标志
TEST_F(ArgsViewTest, GetFlag) { EXPECT_TRUE(argsView->hasFlag("--verbose")); }

// 测试缺失标志
TEST_F(ArgsViewTest, MissingFlag) {
    EXPECT_FALSE(argsView->hasFlag("--missing_flag"));
}

// 测试获取位置参数
TEST_F(ArgsViewTest, GetPositionalArgument) {
    auto positionalArgs = argsView->getArgs();
    EXPECT_EQ(
        positionalArgs.size(),
        3);  // program, --name, test, --age, 30, --verbose, positional_arg
}

// 测试获取帮助信息
TEST_F(ArgsViewTest, HelpMessage) {
    std::string helpMessage = argsView->help();
    EXPECT_NE(helpMessage.find("--name"), std::string::npos);
    EXPECT_NE(helpMessage.find("Name of the user"), std::string::npos);
    EXPECT_NE(helpMessage.find("--age"), std::string::npos);
    EXPECT_NE(helpMessage.find("Age of the user"), std::string::npos);
    EXPECT_NE(helpMessage.find("--verbose"), std::string::npos);
    EXPECT_NE(helpMessage.find("Enable verbose mode"), std::string::npos);
}

// 测试规则
TEST_F(ArgsViewTest, Rules) {
    bool ruleTriggered = false;
    argsView->addRule("--name", [&](std::string_view value) {
        ruleTriggered = true;
        EXPECT_EQ(value, "test");
    });

    EXPECT_TRUE(ruleTriggered);
}