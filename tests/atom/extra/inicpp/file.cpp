#include <gtest/gtest.h>

#include "atom/extra/inicpp/file.hpp"

#include <sstream>
#include <string>
#include <fstream>

using namespace inicpp;

// Test default constructor
TEST(IniFileBaseTest, DefaultConstructor) {
    IniFile iniFile;
    EXPECT_TRUE(iniFile.empty());
}

// Test constructor with filename
TEST(IniFileBaseTest, ConstructorWithFilename) {
    std::ofstream testFile("test.ini");
    testFile << "[section]\nkey=value\n";
    testFile.close();

    IniFile iniFile("test.ini");
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");

    std::remove("test.ini");
}

// Test constructor with input stream
TEST(IniFileBaseTest, ConstructorWithInputStream) {
    std::istringstream iss("[section]\nkey=value\n");
    IniFile iniFile(iss);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");
}

// Test setFieldSep method
TEST(IniFileBaseTest, SetFieldSep) {
    IniFile iniFile;
    iniFile.setFieldSep(':');
    std::istringstream iss("[section]\nkey:value\n");
    iniFile.decode(iss);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");
}

// Test setCommentPrefixes method
TEST(IniFileBaseTest, SetCommentPrefixes) {
    IniFile iniFile;
    iniFile.setCommentPrefixes({"//"});
    std::istringstream iss("[section]\nkey=value\n//comment\n");
    iniFile.decode(iss);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");
}

// Test setEscapeChar method
TEST(IniFileBaseTest, SetEscapeChar) {
    IniFile iniFile;
    iniFile.setEscapeChar('!');
    std::istringstream iss("[section]\nkey=value\n!#escaped comment\n");
    iniFile.decode(iss);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");
}

// Test setMultiLineValues method
TEST(IniFileBaseTest, SetMultiLineValues) {
    IniFile iniFile;
    iniFile.setMultiLineValues(true);
    std::istringstream iss("[section]\nkey=value\n\tcontinued\n");
    iniFile.decode(iss);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value\ncontinued");
}

// Test allowOverwriteDuplicateFields method
TEST(IniFileBaseTest, AllowOverwriteDuplicateFields) {
    IniFile iniFile;
    iniFile.allowOverwriteDuplicateFields(false);
    std::istringstream iss("[section]\nkey=value\nkey=another_value\n");
    EXPECT_THROW(iniFile.decode(iss), std::logic_error);
}

// Test decode method with input stream
TEST(IniFileBaseTest, DecodeWithInputStream) {
    IniFile iniFile;
    std::istringstream iss("[section]\nkey=value\n");
    iniFile.decode(iss);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");
}

// Test decode method with string
TEST(IniFileBaseTest, DecodeWithString) {
    IniFile iniFile;
    std::string content = "[section]\nkey=value\n";
    iniFile.decode(content);
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");
}

// Test load method
TEST(IniFileBaseTest, Load) {
    std::ofstream testFile("test.ini");
    testFile << "[section]\nkey=value\n";
    testFile.close();

    IniFile iniFile;
    iniFile.load("test.ini");
    EXPECT_EQ(iniFile["section"]["key"].as<std::string>(), "value");

    std::remove("test.ini");
}

// Test encode method with output stream
TEST(IniFileBaseTest, EncodeWithOutputStream) {
    IniFile iniFile;
    std::istringstream iss("[section]\nkey=value\n");
    iniFile.decode(iss);

    std::ostringstream oss;
    iniFile.encode(oss);
    EXPECT_EQ(oss.str(), "[section]\nkey=value\n");
}

// Test encode method with string
TEST(IniFileBaseTest, EncodeWithString) {
    IniFile iniFile;
    std::istringstream iss("[section]\nkey=value\n");
    iniFile.decode(iss);

    std::string encoded = iniFile.encode();
    EXPECT_EQ(encoded, "[section]\nkey=value\n");
}

// Test save method
TEST(IniFileBaseTest, Save) {
    IniFile iniFile;
    std::istringstream iss("[section]\nkey=value\n");
    iniFile.decode(iss);

    iniFile.save("test.ini");

    std::ifstream testFile("test.ini");
    std::string content((std::istreambuf_iterator<char>(testFile)),
                        std::istreambuf_iterator<char>());
    EXPECT_EQ(content, "[section]\nkey=value\n");

    std::remove("test.ini");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}