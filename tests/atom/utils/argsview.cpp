#include <gtest/gtest.h>

#include "atom/utils/argsview.hpp"

using namespace atom::utils;

TEST(ArgumentParserTest, Constructor) {
    ArgumentParser parser("test_program");
    ASSERT_EQ(parser.getFlag("nonexistent_flag"), false);
}

TEST(ArgumentParserTest, AddArgument) {
    ArgumentParser parser("test_program");
    parser.addArgument("arg1", ArgumentParser::ArgType::STRING, true, "default",
                       "help message", {"a"});
    auto arg = parser.get<std::string>("arg1");
    ASSERT_TRUE(arg.has_value());
    ASSERT_EQ(arg.value(), "default");
}

TEST(ArgumentParserTest, AddFlag) {
    ArgumentParser parser("test_program");
    parser.addFlag("flag1", "help message", {"f"});
    ASSERT_EQ(parser.getFlag("flag1"), false);
}

TEST(ArgumentParserTest, AddSubcommand) {
    ArgumentParser parser("test_program");
    parser.addSubcommand("subcommand1", "help message");
    // Subcommand parsing is tested in the parse method
}

TEST(ArgumentParserTest, ParseArguments) {
    ArgumentParser parser("test_program");
    parser.addArgument("arg1", ArgumentParser::ArgType::STRING, true);
    parser.addFlag("flag1");

    std::vector<std::string> argv = {"test_program", "--arg1", "value1",
                                     "--flag1"};
    parser.parse(static_cast<int>(argv.size()), argv);

    auto arg1 = parser.get<std::string>("arg1");
    ASSERT_TRUE(arg1.has_value());
    ASSERT_EQ(arg1.value(), "value1");

    ASSERT_EQ(parser.getFlag("flag1"), true);
}

TEST(ArgumentParserTest, ParseSubcommand) {
    ArgumentParser parser("test_program");
    parser.addSubcommand("subcommand1", "help message");
    parser.addArgument("arg1", ArgumentParser::ArgType::STRING, true);

    std::vector<std::string> argv = {"test_program", "subcommand1", "--arg1",
                                     "value1"};
    parser.parse(static_cast<int>(argv.size()), argv);

    // TODO: Implement getSubcommandParser
    // auto subcommandParser = parser.getSubcommandParser("subcommand1");
    // auto arg1 = subcommandParser->get<std::string>("arg1");
    // ASSERT_TRUE(arg1.has_value());
    // ASSERT_EQ(arg1.value(), "value1");
}

TEST(ArgumentParserTest, GetArgument) {
    ArgumentParser parser("test_program");
    parser.addArgument("arg1", ArgumentParser::ArgType::STRING, true,
                       "default");

    auto arg1 = parser.get<std::string>("arg1");
    ASSERT_TRUE(arg1.has_value());
    ASSERT_EQ(arg1.value(), "default");
}

TEST(ArgumentParserTest, GetFlag) {
    ArgumentParser parser("test_program");
    parser.addFlag("flag1");

    ASSERT_EQ(parser.getFlag("flag1"), false);
    std::vector<std::string> argv = {"test_program", "--flag1"};
    parser.parse(static_cast<int>(argv.size()), argv);
    ASSERT_EQ(parser.getFlag("flag1"), true);
}

TEST(ArgumentParserTest, PrintHelp) {
    ArgumentParser parser("test_program");
    parser.addArgument("arg1", ArgumentParser::ArgType::STRING, true, "default",
                       "help message", {"a"});
    parser.addFlag("flag1", "help message", {"f"});
    parser.addSubcommand("subcommand1", "help message");

    testing::internal::CaptureStdout();
    parser.printHelp();
    std::string output = testing::internal::GetCapturedStdout();

    ASSERT_NE(output.find("Usage:"), std::string::npos);
    ASSERT_NE(output.find("--arg1"), std::string::npos);
    ASSERT_NE(output.find("--flag1"), std::string::npos);
    ASSERT_NE(output.find("subcommand1"), std::string::npos);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
