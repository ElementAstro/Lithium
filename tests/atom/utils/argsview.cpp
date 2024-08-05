#include <gtest/gtest.h>

#include "atom/utils/argsview.hpp"

using namespace atom::utils;

TEST(ArgumentParserTest, RequiredArguments) {
    ArgumentParser parser;
    parser.addArgument("input", ArgumentParser::ArgType::STRING, true, {},
                       "Input file", {"i"});
    parser.addArgument("output", ArgumentParser::ArgType::STRING, true, {},
                       "Output file", {"o"});

    const char* argv[] = {"program", "--input", "input.txt", "--output",
                          "output.txt"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_NO_THROW(parser.parse(argc, const_cast<char**>(argv)));

    EXPECT_EQ(parser.get<std::string>("input").value(), "input.txt");
    EXPECT_EQ(parser.get<std::string>("output").value(), "output.txt");
}

TEST(ArgumentParserTest, OptionalArguments) {
    ArgumentParser parser;
    parser.addArgument("threads", ArgumentParser::ArgType::INTEGER, false, 4,
                       "Number of threads", {"t"});

    const char* argv[] = {"program", "--threads", "8"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_NO_THROW(parser.parse(argc, const_cast<char**>(argv)));

    EXPECT_EQ(parser.get<int>("threads").value(), 8);
}

TEST(ArgumentParserTest, DefaultValue) {
    ArgumentParser parser;
    parser.addArgument("threads", ArgumentParser::ArgType::INTEGER, false, 4,
                       "Number of threads", {"t"});

    const char* argv[] = {"program"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_NO_THROW(parser.parse(argc, const_cast<char**>(argv)));

    EXPECT_EQ(parser.get<int>("threads").value(), 4);
}

TEST(ArgumentParserTest, BooleanFlag) {
    ArgumentParser parser;
    parser.addFlag("verbose", "Enable verbose output", {"v"});

    const char* argv[] = {"program", "--verbose"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_NO_THROW(parser.parse(argc, const_cast<char**>(argv)));

    EXPECT_TRUE(parser.getFlag("verbose"));
}

TEST(ArgumentParserTest, MissingRequiredArgument) {
    ArgumentParser parser;
    parser.addArgument("input", ArgumentParser::ArgType::STRING, true, {},
                       "Input file", {"i"});

    const char* argv[] = {"program"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_THROW(parser.parse(argc, const_cast<char**>(argv)),
                 std::invalid_argument);
}

TEST(ArgumentParserTest, Aliases) {
    ArgumentParser parser;
    parser.addArgument("input", ArgumentParser::ArgType::STRING, true, {},
                       "Input file", {"i"});

    const char* argv[] = {"program", "-i", "input.txt"};
    int argc = sizeof(argv) / sizeof(argv[0]);

    EXPECT_NO_THROW(parser.parse(argc, const_cast<char**>(argv)));

    EXPECT_EQ(parser.get<std::string>("input").value(), "input.txt");
}

TEST(ArgumentParserTest, MultipleValues) {
    ArgumentParser parser;
    parser.addMultivalueArgument("files", ArgumentParser::ArgType::STRING,
                                 false, "List of files", {"f"});

    const char* argv[] = {"program", "--files", "file1.txt", "file2.txt"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    std::cout << "argc: " << argc << std::endl;
    EXPECT_NO_THROW(parser.parse(argc, const_cast<char**>(argv)));
    std::cout << "argc: " << argc << std::endl;
    auto files = parser.getMultivalue<std::string>("files");
    std::cout << "files: " << files.value()[0] << std::endl;
    ASSERT_TRUE(files.has_value());
    EXPECT_EQ(files.value().size(), 2);
    EXPECT_EQ(files.value()[0], "file1.txt");
    EXPECT_EQ(files.value()[1], "file2.txt");
}