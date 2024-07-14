#include "ini2json.hpp"

#include <gtest/gtest.h>
#include <fstream>
#include "macro.hpp"

using namespace lithium::cxxtools;

class INI2JSONTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample INI file
        std::ofstream iniFile("test.ini");
        iniFile << "[section1]\nkey1=value1\nkey2=value2\n"
                << "[section2]\nkey3=value3\nkey4=value4\n";
        iniFile.close();
    }

    void TearDown() override {
        // Clean up the created files
        ATOM_UNREF_PARAM(std::remove("test.ini"));
        ATOM_UNREF_PARAM(std::remove("test.json"));
    }
};

TEST_F(INI2JSONTest, BasicConversion) {
    EXPECT_TRUE(iniToJson("test.ini", "test.json"));

    std::ifstream jsonFile("test.json");
    ASSERT_TRUE(jsonFile.is_open());

    std::string jsonContent((std::istreambuf_iterator<char>(jsonFile)),
                            std::istreambuf_iterator<char>());
    std::string expectedJson = R"({
    "section1": {
        "key1": "value1",
        "key2": "value2"
    },
    "section2": {
        "key3": "value3",
        "key4": "value4"
    }
})";
    EXPECT_EQ(jsonContent, expectedJson);
}

TEST_F(INI2JSONTest, MissingINIFile) {
    EXPECT_THROW(
        { iniToJson("nonexistent.ini", "test.json"); }, std::runtime_error);
}

TEST_F(INI2JSONTest, InvalidINIContent) {
    std::ofstream iniFile("invalid.ini");
    iniFile << "[section1\nkey1=value1\nkey2=value2\n";
    iniFile.close();

    EXPECT_THROW({ iniToJson("invalid.ini", "test.json"); }, std::exception);

    ATOM_UNREF_PARAM(std::remove("invalid.ini"));
}
