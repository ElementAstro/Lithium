// test_json2xml.cpp
#include <gtest/gtest.h>
#include <tinyxml2.h>
#include <filesystem>
#include <fstream>
#include "atom/type/json.hpp"
#include "json2xml.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;
using namespace lithium::cxxtools::converters;

class JsonToXmlConverterTest : public ::testing::Test {
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

TEST_F(JsonToXmlConverterTest, ConvertImpl_ValidJsonData_ReturnsXml) {
    JsonToXmlConverter converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details",
         {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};

    fs::path xmlFilePath = fs::temp_directory_path() / "test.xml";
    bool result = converter.convertImpl(jsonData, xmlFilePath);

    ASSERT_TRUE(result);
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLError eResult = xmlDoc.LoadFile(xmlFilePath.string().c_str());
    ASSERT_EQ(eResult, tinyxml2::XML_SUCCESS);

    tinyxml2::XMLElement* root = xmlDoc.FirstChildElement("root");
    ASSERT_NE(root, nullptr);

    tinyxml2::XMLElement* nameElement = root->FirstChildElement("name");
    ASSERT_NE(nameElement, nullptr);
    EXPECT_STREQ(nameElement->GetText(), "Alice");

    tinyxml2::XMLElement* ageElement = root->FirstChildElement("age");
    ASSERT_NE(ageElement, nullptr);
    EXPECT_STREQ(ageElement->GetText(), "30");

    tinyxml2::XMLElement* cityElement = root->FirstChildElement("city");
    ASSERT_NE(cityElement, nullptr);
    EXPECT_STREQ(cityElement->GetText(), "New York");

    tinyxml2::XMLElement* detailsElement = root->FirstChildElement("details");
    ASSERT_NE(detailsElement, nullptr);

    tinyxml2::XMLElement* hobbiesElement =
        detailsElement->FirstChildElement("hobbies");
    ASSERT_NE(hobbiesElement, nullptr);
    tinyxml2::XMLElement* hobby1Element =
        hobbiesElement->FirstChildElement("hobbies");
    ASSERT_NE(hobby1Element, nullptr);
    EXPECT_STREQ(hobby1Element->GetText(), "reading");
    tinyxml2::XMLElement* hobby2Element =
        hobby1Element->NextSiblingElement("hobbies");
    ASSERT_NE(hobby2Element, nullptr);
    EXPECT_STREQ(hobby2Element->GetText(), "swimming");

    tinyxml2::XMLElement* marriedElement =
        detailsElement->FirstChildElement("married");
    ASSERT_NE(marriedElement, nullptr);
    EXPECT_STREQ(marriedElement->GetText(), "false");

    // Clean up
    fs::remove(xmlFilePath);
}

TEST_F(JsonToXmlConverterTest, ConvertImpl_InvalidJsonData_ThrowsException) {
    JsonToXmlConverter converter;
    json invalidJsonData = {
        {"name", "Alice"}, {"age", 30}, {"city", "New York"}};

    fs::path xmlFilePath = fs::temp_directory_path() / "test.xml";

    EXPECT_THROW(converter.convertImpl(invalidJsonData, xmlFilePath),
                 std::runtime_error);
}

TEST_F(JsonToXmlConverterTest, ConvertImpl_EmptyJsonData_ReturnsEmptyXml) {
    JsonToXmlConverter converter;
    json emptyJsonData = json::object();

    fs::path xmlFilePath = fs::temp_directory_path() / "test.xml";
    bool result = converter.convertImpl(emptyJsonData, xmlFilePath);

    ASSERT_TRUE(result);
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLError eResult = xmlDoc.LoadFile(xmlFilePath.string().c_str());
    ASSERT_EQ(eResult, tinyxml2::XML_SUCCESS);

    tinyxml2::XMLElement* root = xmlDoc.FirstChildElement("root");
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->FirstChild(), nullptr);

    // Clean up
    fs::remove(xmlFilePath);
}

TEST_F(JsonToXmlConverterTest, ConvertImpl_NestedJsonData_ReturnsNestedXml) {
    JsonToXmlConverter converter;
    json nestedJsonData = {
        {"person",
         {{"name", "Alice"},
          {"age", 30},
          {"address", {{"city", "New York"}, {"zip", "10001"}}}}}};

    fs::path xmlFilePath = fs::temp_directory_path() / "test.xml";
    bool result = converter.convertImpl(nestedJsonData, xmlFilePath);

    ASSERT_TRUE(result);
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLError eResult = xmlDoc.LoadFile(xmlFilePath.string().c_str());
    ASSERT_EQ(eResult, tinyxml2::XML_SUCCESS);

    tinyxml2::XMLElement* root = xmlDoc.FirstChildElement("root");
    ASSERT_NE(root, nullptr);

    tinyxml2::XMLElement* personElement = root->FirstChildElement("person");
    ASSERT_NE(personElement, nullptr);

    tinyxml2::XMLElement* nameElement =
        personElement->FirstChildElement("name");
    ASSERT_NE(nameElement, nullptr);
    EXPECT_STREQ(nameElement->GetText(), "Alice");

    tinyxml2::XMLElement* ageElement = personElement->FirstChildElement("age");
    ASSERT_NE(ageElement, nullptr);
    EXPECT_STREQ(ageElement->GetText(), "30");

    tinyxml2::XMLElement* addressElement =
        personElement->FirstChildElement("address");
    ASSERT_NE(addressElement, nullptr);

    tinyxml2::XMLElement* cityElement =
        addressElement->FirstChildElement("city");
    ASSERT_NE(cityElement, nullptr);
    EXPECT_STREQ(cityElement->GetText(), "New York");

    tinyxml2::XMLElement* zipElement = addressElement->FirstChildElement("zip");
    ASSERT_NE(zipElement, nullptr);
    EXPECT_STREQ(zipElement->GetText(), "10001");

    // Clean up
    fs::remove(xmlFilePath);
}

TEST_F(JsonToXmlConverterTest, SaveToFileImpl_ValidJsonData_SavesToFile) {
    JsonToXmlConverter converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details",
         {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};

    fs::path xmlFilePath = fs::temp_directory_path() / "test.xml";
    bool result = converter.convertImpl(jsonData, xmlFilePath);

    ASSERT_TRUE(result);
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLError eResult = xmlDoc.LoadFile(xmlFilePath.string().c_str());
    ASSERT_EQ(eResult, tinyxml2::XML_SUCCESS);

    tinyxml2::XMLElement* root = xmlDoc.FirstChildElement("root");
    ASSERT_NE(root, nullptr);

    tinyxml2::XMLElement* nameElement = root->FirstChildElement("name");
    ASSERT_NE(nameElement, nullptr);
    EXPECT_STREQ(nameElement->GetText(), "Alice");

    tinyxml2::XMLElement* ageElement = root->FirstChildElement("age");
    ASSERT_NE(ageElement, nullptr);
    EXPECT_STREQ(ageElement->GetText(), "30");

    tinyxml2::XMLElement* cityElement = root->FirstChildElement("city");
    ASSERT_NE(cityElement, nullptr);
    EXPECT_STREQ(cityElement->GetText(), "New York");

    tinyxml2::XMLElement* detailsElement = root->FirstChildElement("details");
    ASSERT_NE(detailsElement, nullptr);

    tinyxml2::XMLElement* hobbiesElement =
        detailsElement->FirstChildElement("hobbies");
    ASSERT_NE(hobbiesElement, nullptr);
    tinyxml2::XMLElement* hobby1Element =
        hobbiesElement->FirstChildElement("hobbies");
    ASSERT_NE(hobby1Element, nullptr);
    EXPECT_STREQ(hobby1Element->GetText(), "reading");
    tinyxml2::XMLElement* hobby2Element =
        hobby1Element->NextSiblingElement("hobbies");
    ASSERT_NE(hobby2Element, nullptr);
    EXPECT_STREQ(hobby2Element->GetText(), "swimming");

    tinyxml2::XMLElement* marriedElement =
        detailsElement->FirstChildElement("married");
    ASSERT_NE(marriedElement, nullptr);
    EXPECT_STREQ(marriedElement->GetText(), "false");

    // Clean up
    fs::remove(xmlFilePath);
}

TEST_F(JsonToXmlConverterTest, SaveToFileImpl_InvalidFilePath_ThrowsException) {
    JsonToXmlConverter converter;
    json jsonData = {
        {"name", "Alice"},
        {"age", 30},
        {"city", "New York"},
        {"details",
         {{"hobbies", {"reading", "swimming"}}, {"married", false}}}};

    fs::path invalidXmlFilePath = "/invalid/path/test.xml";

    EXPECT_THROW(converter.convertImpl(jsonData, invalidXmlFilePath),
                 std::runtime_error);
}
