// test_json2ini.cpp
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "json2ini.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::converters;

class JsonToIniConverterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary JSON file for testing
        jsonFilePath = fs::temp_directory_path() / "test.json";
        json jsonData = {
            {"section1", {{"key1", "value1"}, {"key2", "value2"}}},
            {"section2", {{"keyA", "valueA"}, {"keyB", "valueB"}}}};
        std::ofstream jsonFile(jsonFilePath);
        jsonFile << std::setw(4) << jsonData;
        jsonFile.close();
    }

    void TearDown() override {
        // Remove the temporary JSON file
        fs::remove(jsonFilePath);
    }

    fs::path jsonFilePath;
};

TEST_F(JsonToIniConverterTest, ConvertImpl_ValidJsonData_ReturnsIni) {
    JsonToIniConverter converter;
    json jsonData = {{"section1", {{"key1", "value1"}, {"key2", "value2"}}},
                     {"section2", {{"keyA", "valueA"}, {"keyB", "valueB"}}}};

    fs::path iniFilePath = fs::temp_directory_path() / "test.ini";
    bool result = converter.convertImpl(jsonData, iniFilePath);

    ASSERT_TRUE(result);
    std::ifstream iniFile(iniFilePath);
    ASSERT_TRUE(iniFile.is_open());

    std::string line;
    std::getline(iniFile, line);
    EXPECT_EQ(line, "[section1]");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "key1=value1");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "key2=value2");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "[section2]");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "keyA=valueA");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "keyB=valueB");

    iniFile.close();

    // Clean up
    fs::remove(iniFilePath);
}

TEST_F(JsonToIniConverterTest, ConvertImpl_InvalidJsonData_ThrowsException) {
    JsonToIniConverter converter;
    json invalidJsonData = {
        {"name", "Alice"}, {"age", 30}, {"city", "New York"}};

    fs::path iniFilePath = fs::temp_directory_path() / "test.ini";

    EXPECT_THROW(converter.convertImpl(invalidJsonData, iniFilePath),
                 std::runtime_error);
}

TEST_F(JsonToIniConverterTest, ConvertImpl_EmptyJsonData_ReturnsEmptyIni) {
    JsonToIniConverter converter;
    json emptyJsonData = json::object();

    fs::path iniFilePath = fs::temp_directory_path() / "test.ini";
    bool result = converter.convertImpl(emptyJsonData, iniFilePath);

    ASSERT_TRUE(result);
    std::ifstream iniFile(iniFilePath);
    ASSERT_TRUE(iniFile.is_open());

    std::string line;
    EXPECT_FALSE(std::getline(iniFile, line));

    iniFile.close();

    // Clean up
    fs::remove(iniFilePath);
}

TEST_F(JsonToIniConverterTest, ConvertImpl_NestedJsonData_ThrowsException) {
    JsonToIniConverter converter;
    json nestedJsonData = {
        {"section1", {{"key1", "value1"}, {"nested", {{"key2", "value2"}}}}}};

    fs::path iniFilePath = fs::temp_directory_path() / "test.ini";

    EXPECT_THROW(converter.convertImpl(nestedJsonData, iniFilePath),
                 std::runtime_error);
}

TEST_F(JsonToIniConverterTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    JsonToIniConverter converter;
    json jsonData = {{"section1", {{"key1", "value1"}, {"key2", "value2"}}},
                     {"section2", {{"keyA", "valueA"}, {"keyB", "valueB"}}}};

    fs::path iniFilePath = fs::temp_directory_path() / "test.ini";
    bool result = converter.convertImpl(jsonData, iniFilePath);

    ASSERT_TRUE(result);
    std::ifstream iniFile(iniFilePath);
    ASSERT_TRUE(iniFile.is_open());

    std::string line;
    std::getline(iniFile, line);
    EXPECT_EQ(line, "[section1]");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "key1=value1");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "key2=value2");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "[section2]");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "keyA=valueA");

    std::getline(iniFile, line);
    EXPECT_EQ(line, "keyB=valueB");

    iniFile.close();

    // Clean up
    fs::remove(iniFilePath);
}

TEST_F(JsonToIniConverterTest, SaveToFileImpl_InvalidFilePath_ThrowsException) {
    JsonToIniConverter converter;
    json jsonData = {{"section1", {{"key1", "value1"}, {"key2", "value2"}}},
                     {"section2", {{"keyA", "valueA"}, {"keyB", "valueB"}}}};

    fs::path invalidIniFilePath = "/invalid/path/test.ini";

    EXPECT_THROW(converter.convertImpl(jsonData, invalidIniFilePath),
                 std::runtime_error);
}
