// test_xml2json.cpp
#include <gtest/gtest.h>
#include <tinyxml2.h>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "xml2json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::detail;

class Xml2JsonTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary XML file for testing
        xmlFilePath = fs::temp_directory_path() / "test.xml";
        std::ofstream xmlFile(xmlFilePath);
        xmlFile << R"(
            <root>
                <name>Alice</name>
                <age>30</age>
                <city>New York</city>
                <details>
                    <hobbies>reading</hobbies>
                    <hobbies>swimming</hobbies>
                    <married>false</married>
                </details>
            </root>
        )";
        xmlFile.close();
    }

    void TearDown() override {
        // Remove the temporary XML file
        fs::remove(xmlFilePath);
    }

    fs::path xmlFilePath;
};

TEST_F(Xml2JsonTest, ConvertImpl_ValidXmlFile_ReturnsJson) {
    Xml2Json converter;
    json result = converter.convertImpl(xmlFilePath.string());

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result["root"]["name"], "Alice");
    EXPECT_EQ(result["root"]["age"], "30");
    EXPECT_EQ(result["root"]["city"], "New York");
    EXPECT_EQ(result["root"]["details"]["hobbies"][0], "reading");
    EXPECT_EQ(result["root"]["details"]["hobbies"][1], "swimming");
    EXPECT_EQ(result["root"]["details"]["married"], "false");
}

TEST_F(Xml2JsonTest, ConvertImpl_InvalidXmlFile_ThrowsException) {
    Xml2Json converter;
    fs::path invalidXmlFilePath = fs::temp_directory_path() / "invalid.xml";

    EXPECT_THROW(converter.convertImpl(invalidXmlFilePath.string()),
                 std::runtime_error);
}

TEST_F(Xml2JsonTest, ConvertImpl_EmptyXmlFile_ReturnsEmptyJson) {
    Xml2Json converter;
    fs::path emptyXmlFilePath = fs::temp_directory_path() / "empty.xml";
    std::ofstream emptyXmlFile(emptyXmlFilePath);
    emptyXmlFile << R"(<root></root>)";
    emptyXmlFile.close();

    json result = converter.convertImpl(emptyXmlFilePath.string());

    ASSERT_EQ(result.size(), 1);
    EXPECT_TRUE(result["root"].empty());

    // Clean up
    fs::remove(emptyXmlFilePath);
}

TEST_F(Xml2JsonTest, ConvertImpl_NestedXmlFile_ReturnsNestedJson) {
    Xml2Json converter;
    fs::path nestedXmlFilePath = fs::temp_directory_path() / "nested.xml";
    std::ofstream nestedXmlFile(nestedXmlFilePath);
    nestedXmlFile << R"(
        <root>
            <person>
                <name>Alice</name>
                <age>30</age>
                <address>
                    <city>New York</city>
                    <zip>10001</zip>
                </address>
            </person>
        </root>
    )";
    nestedXmlFile.close();

    json result = converter.convertImpl(nestedXmlFilePath.string());

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result["root"]["person"]["name"], "Alice");
    EXPECT_EQ(result["root"]["person"]["age"], "30");
    EXPECT_EQ(result["root"]["person"]["address"]["city"], "New York");
    EXPECT_EQ(result["root"]["person"]["address"]["zip"], "10001");

    // Clean up
    fs::remove(nestedXmlFilePath);
}

TEST_F(Xml2JsonTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    Xml2Json converter;
    json jsonData = {
        {"root",
         {{"name", "Alice"},
          {"age", 30},
          {"city", "New York"},
          {"details",
           {{"hobbies", {"reading", "swimming"}}, {"married", false}}}}}};

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

TEST_F(Xml2JsonTest, SaveToFileImpl_InvalidFilePath_ThrowsException) {
    Xml2Json converter;
    json jsonData = {
        {"root",
         {{"name", "Alice"},
          {"age", 30},
          {"city", "New York"},
          {"details",
           {{"hobbies", {"reading", "swimming"}}, {"married", false}}}}}};

    fs::path invalidJsonFilePath = "/invalid/path/test.json";

    EXPECT_THROW(
        converter.saveToFileImpl(jsonData, invalidJsonFilePath.string()),
        std::runtime_error);
}
