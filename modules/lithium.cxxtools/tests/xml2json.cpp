#include "xml2json.hpp"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace lithium::cxxtools;

class XML2JSONTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::ofstream xmlFile("test.xml");
        xmlFile << R"(
            <root>
                <title>Example Title</title>
                <owner>
                    <name>Tom Preston-Werner</name>
                    <dob>1979-05-27T07:32:00Z</dob>
                </owner>
                <database>
                    <server>192.168.1.1</server>
                    <ports>8001,8001,8002</ports>
                    <connection_max>5000</connection_max>
                    <enabled>true</enabled>
                </database>
            </root>
        )";
        xmlFile.close();
    }

    void TearDown() override {
        fs::remove("test.xml");
        fs::remove("test.json");
    }
};

TEST_F(XML2JSONTest, BasicConversion) {
    EXPECT_TRUE(detail::convertXmlToJson("test.xml", "test.json"));

    std::ifstream jsonFile("test.json");
    ASSERT_TRUE(jsonFile.is_open());

    std::string jsonContent((std::istreambuf_iterator<char>(jsonFile)),
                            std::istreambuf_iterator<char>());
    ASSERT_FALSE(jsonContent.empty());
}

TEST_F(XML2JSONTest, MissingXMLFile) {
    EXPECT_FALSE(detail::convertXmlToJson("nonexistent.xml", "test.json"));
}

TEST_F(XML2JSONTest, InvalidXMLContent) {
    std::ofstream xmlFile("invalid.xml");
    xmlFile << R"(
        <root>
            <title>Example Title</title>
            <owner>
                <name>Tom Preston-Werner</name>
                <dob>1979-05-27T07:32:00Z</dob>
            <owner>
            <database>
                <server>192.168.1.1</server>
    )";
    xmlFile.close();

    EXPECT_FALSE(detail::convertXmlToJson("invalid.xml", "test.json"));

    fs::remove("invalid.xml");
}
