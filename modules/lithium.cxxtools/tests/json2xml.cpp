#include "json2xml.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "tinyxml2/tinyxml2.h"

namespace fs = std::filesystem;
using namespace lithium::cxxtools;

class JSON2XMLTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a sample JSON file
        std::ofstream jsonFile("test.json");
        jsonFile << R"({
            "title": "Example Title",
            "owner": {
                "name": "Tom Preston-Werner",
                "dob": "1979-05-27T07:32:00Z"
            },
            "database": {
                "server": "192.168.1.1",
                "ports": [ 8001, 8001, 8002 ],
                "connection_max": 5000,
                "enabled": true
            }
        })";
        jsonFile.close();
    }

    void TearDown() override {
        // Clean up the created files
        fs::remove("test.json");
        fs::remove("test.xml");
    }
};

TEST_F(JSON2XMLTest, BasicConversion) {
    EXPECT_TRUE(detail::convertJsonToXml("test.json", "test.xml"));

    tinyxml2::XMLDocument xmlDoc;
    ASSERT_EQ(xmlDoc.LoadFile("test.xml"), tinyxml2::XML_SUCCESS);

    tinyxml2::XMLElement *root = xmlDoc.RootElement();
    ASSERT_NE(root, nullptr);
    EXPECT_STREQ(root->Name(), "root");

    tinyxml2::XMLElement *title = root->FirstChildElement("title");
    ASSERT_NE(title, nullptr);
    EXPECT_STREQ(title->GetText(), "Example Title");
}

TEST_F(JSON2XMLTest, MissingJSONFile) {
    EXPECT_FALSE(detail::convertJsonToXml("nonexistent.json", "test.xml"));
}

TEST_F(JSON2XMLTest, InvalidJSONContent) {
    std::ofstream jsonFile("invalid.json");
    jsonFile << R"({ "title": "Example Title", "owner": { "name": "Tom" )";
    jsonFile.close();

    EXPECT_FALSE(detail::convertJsonToXml("invalid.json", "test.xml"));

    fs::remove("invalid.json");
}
