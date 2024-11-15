// test_json2yaml.cpp
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "json2yaml.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::converters;

class JsonToYamlConverterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary JSON file for testing
        jsonFilePath = fs::temp_directory_path() / "test.json";
        json jsonData = {
            {"name", "Alice"},
            {"age", 30},
            {"city", "New York"},
            {"details",
             {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};
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

TEST_F(JsonToYamlConverterTest, ConvertImpl_ValidJsonData_ReturnsYaml) {
    JsonToYamlConverter converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details",
         {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};

    fs::path yamlFilePath = fs::temp_directory_path() / "test.yaml";
    bool result = converter.convertImpl(jsonData, yamlFilePath);

    ASSERT_TRUE(result);
    std::ifstream yamlFile(yamlFilePath);
    ASSERT_TRUE(yamlFile.is_open());

    YAML::Node yamlNode = YAML::LoadFile(yamlFilePath.string());
    EXPECT_EQ(yamlNode["name"].as<std::string>(), "Alice");
    EXPECT_EQ(yamlNode["age"].as<int>(), 30);
    EXPECT_EQ(yamlNode["city"].as<std::string>(), "New York");
    EXPECT_EQ(yamlNode["details"]["hobbies"][0].as<std::string>(), "reading");
    EXPECT_EQ(yamlNode["details"]["hobbies"][1].as<std::string>(), "swimming");
    EXPECT_EQ(yamlNode["details"]["married"].as<bool>(), false);

    // Clean up
    fs::remove(yamlFilePath);
}

TEST_F(JsonToYamlConverterTest, ConvertImpl_InvalidJsonData_ThrowsException) {
    JsonToYamlConverter converter;
    json invalidJsonData = {
        {"name", "Alice"}, {"age", 30}, {"city", "New York"}};

    fs::path yamlFilePath = fs::temp_directory_path() / "test.yaml";

    EXPECT_THROW(converter.convertImpl(invalidJsonData, yamlFilePath),
                 std::runtime_error);
}

TEST_F(JsonToYamlConverterTest, ConvertImpl_EmptyJsonData_ReturnsEmptyYaml) {
    JsonToYamlConverter converter;
    json emptyJsonData = json::object();

    fs::path yamlFilePath = fs::temp_directory_path() / "test.yaml";
    bool result = converter.convertImpl(emptyJsonData, yamlFilePath);

    ASSERT_TRUE(result);
    std::ifstream yamlFile(yamlFilePath);
    ASSERT_TRUE(yamlFile.is_open());

    YAML::Node yamlNode = YAML::LoadFile(yamlFilePath.string());
    EXPECT_TRUE(yamlNode.IsMap());
    EXPECT_TRUE(yamlNode.size() == 0);

    // Clean up
    fs::remove(yamlFilePath);
}

TEST_F(JsonToYamlConverterTest, ConvertImpl_NestedJsonData_ReturnsNestedYaml) {
    JsonToYamlConverter converter;
    json nestedJsonData = {
        {"person",
         {{"name", "Alice"},
          {"age", 30},
          {"address", {{"city", "New York"}, {"zip", "10001"}}}}}};

    fs::path yamlFilePath = fs::temp_directory_path() / "test.yaml";
    bool result = converter.convertImpl(nestedJsonData, yamlFilePath);

    ASSERT_TRUE(result);
    std::ifstream yamlFile(yamlFilePath);
    ASSERT_TRUE(yamlFile.is_open());

    YAML::Node yamlNode = YAML::LoadFile(yamlFilePath.string());
    EXPECT_EQ(yamlNode["person"]["name"].as<std::string>(), "Alice");
    EXPECT_EQ(yamlNode["person"]["age"].as<int>(), 30);
    EXPECT_EQ(yamlNode["person"]["address"]["city"].as<std::string>(),
              "New York");
    EXPECT_EQ(yamlNode["person"]["address"]["zip"].as<std::string>(), "10001");

    // Clean up
    fs::remove(yamlFilePath);
}

TEST_F(JsonToYamlConverterTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    JsonToYamlConverter converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details",
         {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};

    fs::path yamlFilePath = fs::temp_directory_path() / "test.yaml";
    bool result = converter.convertImpl(jsonData, yamlFilePath);

    ASSERT_TRUE(result);
    std::ifstream yamlFile(yamlFilePath);
    ASSERT_TRUE(yamlFile.is_open());

    YAML::Node yamlNode = YAML::LoadFile(yamlFilePath.string());
    EXPECT_EQ(yamlNode["name"].as<std::string>(), "Alice");
    EXPECT_EQ(yamlNode["age"].as<int>(), 30);
    EXPECT_EQ(yamlNode["city"].as<std::string>(), "New York");
    EXPECT_EQ(yamlNode["details"]["hobbies"][0].as<std::string>(), "reading");
    EXPECT_EQ(yamlNode["details"]["hobbies"][1].as<std::string>(), "swimming");
    EXPECT_EQ(yamlNode["details"]["married"].as<bool>(), false);

    // Clean up
    fs::remove(yamlFilePath);
}

TEST_F(JsonToYamlConverterTest,
       SaveToFileImpl_InvalidFilePath_ThrowsException) {
    JsonToYamlConverter converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details",
         {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};

    fs::path invalidYamlFilePath = "/invalid/path/test.yaml";

    EXPECT_THROW(converter.convertImpl(jsonData, invalidYamlFilePath),
                 std::runtime_error);
}
