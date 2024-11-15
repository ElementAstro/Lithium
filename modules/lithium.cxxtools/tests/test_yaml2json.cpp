// test_yaml2json.cpp
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>
#include "yaml2json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::detail;

class Yaml2JsonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary YAML file for testing
        yamlFilePath = fs::temp_directory_path() / "test.yaml";
        std::ofstream yamlFile(yamlFilePath);
        yamlFile << R"(
            name: Alice
            age: 30
            city: New York
            details:
              hobbies:
                - reading
                - swimming
              married: false
        )";
        yamlFile.close();
    }

    void TearDown() override {
        // Remove the temporary YAML file
        fs::remove(yamlFilePath);
    }

    fs::path yamlFilePath;
};

TEST_F(Yaml2JsonTest, ConvertImpl_ValidYamlFile_ReturnsJson) {
    Yaml2Json converter;
    json result = converter.convertImpl(yamlFilePath.string());

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result["name"], "Alice");
    EXPECT_EQ(result["age"], "30");
    EXPECT_EQ(result["city"], "New York");
    EXPECT_EQ(result["details"]["hobbies"][0], "reading");
    EXPECT_EQ(result["details"]["hobbies"][1], "swimming");
    EXPECT_EQ(result["details"]["married"], "false");
}

TEST_F(Yaml2JsonTest, ConvertImpl_InvalidYamlFile_ThrowsException) {
    Yaml2Json converter;
    fs::path invalidYamlFilePath = fs::temp_directory_path() / "invalid.yaml";

    EXPECT_THROW(converter.convertImpl(invalidYamlFilePath.string()), std::runtime_error);
}

TEST_F(Yaml2JsonTest, ConvertImpl_EmptyYamlFile_ReturnsEmptyJson) {
    Yaml2Json converter;
    fs::path emptyYamlFilePath = fs::temp_directory_path() / "empty.yaml";
    std::ofstream emptyYamlFile(emptyYamlFilePath);
    emptyYamlFile << R"()";
    emptyYamlFile.close();

    json result = converter.convertImpl(emptyYamlFilePath.string());

    ASSERT_TRUE(result.is_null());

    // Clean up
    fs::remove(emptyYamlFilePath);
}

TEST_F(Yaml2JsonTest, ConvertImpl_NestedYamlFile_ReturnsNestedJson) {
    Yaml2Json converter;
    fs::path nestedYamlFilePath = fs::temp_directory_path() / "nested.yaml";
    std::ofstream nestedYamlFile(nestedYamlFilePath);
    nestedYamlFile << R"(
        person:
          name: Alice
          age: 30
          address:
            city: New York
            zip: 10001
    )";
    nestedYamlFile.close();

    json result = converter.convertImpl(nestedYamlFilePath.string());

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result["person"]["name"], "Alice");
    EXPECT_EQ(result["person"]["age"], "30");
    EXPECT_EQ(result["person"]["address"]["city"], "New York");
    EXPECT_EQ(result["person"]["address"]["zip"], "10001");

    // Clean up
    fs::remove(nestedYamlFilePath);
}

TEST_F(Yaml2JsonTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    Yaml2Json converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details", {{"hobbies", {"reading", "swimming"}}, {"married", false}}}
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

TEST_F(Yaml2JsonTest, SaveToFileImpl_InvalidFilePath_ThrowsException) {
    Yaml2Json converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details", {{"hobbies", {"reading", "swimming"}}, {"married", false}}}
    };

    fs::path invalidJsonFilePath = "/invalid/path/test.json";

    EXPECT_THROW(converter.saveToFileImpl(jsonData, invalidJsonFilePath.string()), std::runtime_error);
}
