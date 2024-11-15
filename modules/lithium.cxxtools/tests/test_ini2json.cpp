// test_ini2json.cpp
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include "ini2json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::detail;

class Ini2JsonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary INI file for testing
        iniFilePath = fs::temp_directory_path() / "test.ini";
        std::ofstream iniFile(iniFilePath);
        iniFile << "[section1]\n";
        iniFile << "key1=value1\n";
        iniFile << "key2=value2\n";
        iniFile << "\n";
        iniFile << "[section2]\n";
        iniFile << "keyA=valueA\n";
        iniFile << "keyB=valueB\n";
        iniFile.close();
    }

    void TearDown() override {
        // Remove the temporary INI file
        fs::remove(iniFilePath);
    }

    fs::path iniFilePath;
};

TEST_F(Ini2JsonTest, ConvertImpl_ValidIniFile_ReturnsJson) {
    Ini2Json converter;
    json result = converter.convertImpl(iniFilePath.string());

    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result["section1"]["key1"], "value1");
    EXPECT_EQ(result["section1"]["key2"], "value2");
    EXPECT_EQ(result["section2"]["keyA"], "valueA");
    EXPECT_EQ(result["section2"]["keyB"], "valueB");
}

TEST_F(Ini2JsonTest, ConvertImpl_InvalidIniFile_ThrowsException) {
    Ini2Json converter;
    fs::path invalidIniFilePath = fs::temp_directory_path() / "invalid.ini";

    EXPECT_THROW(converter.convertImpl(invalidIniFilePath.string()), std::runtime_error);
}

TEST_F(Ini2JsonTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    Ini2Json converter;
    json jsonData = {
        {"section1", {{"key1", "value1"}, {"key2", "value2"}}},
        {"section2", {{"keyA", "valueA"}, {"keyB", "valueB"}}}
    };

    fs::path jsonFilePath = fs::temp_directory_path() / "test.json";
    bool result = converter.saveToFileImpl(jsonData, jsonFilePath.string());

    ASSERT_TRUE(result);
    std::ifstream jsonFile(jsonFilePath);
    ASSERT_TRUE(jsonFile.is_open());

    json savedData;
    jsonFile >> savedData;
    jsonFile.close();

    EXPECT_EQ(savedData, jsonData);

    // Clean up
    fs::remove(jsonFilePath);
}

TEST_F(Ini2JsonTest, SaveToFileImpl_InvalidFilePath_ThrowsException) {
    Ini2Json converter;
    json jsonData = {
        {"section1", {{"key1", "value1"}, {"key2", "value2"}}},
        {"section2", {{"keyA", "valueA"}, {"keyB", "valueB"}}}
    };

    fs::path invalidJsonFilePath = "/invalid/path/test.json";

    EXPECT_THROW(converter.saveToFileImpl(jsonData, invalidJsonFilePath.string()), std::runtime_error);
}
