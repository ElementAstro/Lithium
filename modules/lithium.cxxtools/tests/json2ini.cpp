#include "json2ini.hpp"

#include <gtest/gtest.h>
#include <fstream>
#include "atom/macro.hpp"

using namespace lithium::cxxtools;

class JSON2INITest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample JSON file
        std::ofstream jsonFile("test.json");
        jsonFile << R"({
            "section1": {
                "key1": "value1",
                "key2": "value2"
            },
            "section2": {
                "key3": "value3",
                "key4": "value4"
            }
        })";
        jsonFile.close();
    }

    void TearDown() override {
        // Clean up the created files
        ATOM_UNUSED_RESULT(std::remove("test.json"));
        ATOM_UNUSED_RESULT(std::remove("test.ini"));
    }
};

TEST_F(JSON2INITest, BasicConversion) {
    EXPECT_NO_THROW(jsonToIni("test.json", "test.ini"));

    std::ifstream iniFile("test.ini");
    ASSERT_TRUE(iniFile.is_open());

    std::string iniContent((std::istreambuf_iterator<char>(iniFile)),
                           std::istreambuf_iterator<char>());
    std::string expectedIni = R"([section1]
key1=value1
key2=value2

[section2]
key3=value3
key4=value4

)";
    EXPECT_EQ(iniContent, expectedIni);
}

TEST_F(JSON2INITest, MissingJSONFile) {
    EXPECT_THROW(jsonToIni("nonexistent.json", "test.ini"), std::runtime_error);
}

TEST_F(JSON2INITest, InvalidJSONContent) {
    std::ofstream jsonFile("invalid.json");
    jsonFile << R"({ "section1": { "key1": "value1", "key2": "value2", )";
    jsonFile.close();

    EXPECT_THROW(jsonToIni("invalid.json", "test.ini"), std::runtime_error);

    ATOM_UNUSED_RESULT(std::remove("invalid.json"));
}
