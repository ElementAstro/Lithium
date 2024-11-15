#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "collection.hpp"

#include "tinyxml2.h"

#include "atom/error/exception.hpp"

namespace fs = std::filesystem;

class INDIDriverCollectionTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = fs::temp_directory_path() / "indi_test_drivers";
        fs::create_directories(testDir);
        collection = std::make_unique<INDIDriverCollection>();
    }

    void TearDown() override { fs::remove_all(testDir); }

    void createXMLFile(const std::string& filename,
                       const std::string& content) {
        std::ofstream file(testDir / filename);
        file << content;
    }

    fs::path testDir;
    std::unique_ptr<INDIDriverCollection> collection;
};

TEST_F(INDIDriverCollectionTest, EmptyDirectory) {
    EXPECT_THROW(collection->parseDrivers(testDir.string()),
                 atom::error::FileNotFound);
    auto families = collection->getFamilies();
    EXPECT_TRUE(families.empty());
}

TEST_F(INDIDriverCollectionTest, InvalidDirectory) {
    EXPECT_THROW(collection->parseDrivers("/nonexistent/path"),
                 atom::error::FileNotFound);
}

TEST_F(INDIDriverCollectionTest, ValidXMLParsing) {
    const char* validXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Test Scope">
                    <driver name="test_scope">indi_test_telescope</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </root>
    )";

    createXMLFile("telescope.xml", validXML);
    EXPECT_TRUE(collection->parseDrivers(testDir.string()));

    auto device = collection->getByLabel("Test Scope");
    ASSERT_NE(device, nullptr);
    EXPECT_EQ(device->name, "test_scope");
    EXPECT_EQ(device->binary, "indi_test_telescope");
    EXPECT_EQ(device->version, "1.0");
    EXPECT_EQ(device->family, "Telescopes");
}

TEST_F(INDIDriverCollectionTest, SkFileExclusion) {
    const char* skXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Ignored Device">
                    <driver name="ignore">indi_ignored</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </root>
    )";

    createXMLFile("telescope_sk.xml", skXML);
    EXPECT_TRUE(collection->parseDrivers(testDir.string()));
    EXPECT_EQ(collection->getByLabel("Ignored Device"), nullptr);
}

TEST_F(INDIDriverCollectionTest, MultipleDeviceGroups) {
    const char* multiGroupXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Scope 1">
                    <driver name="scope1">indi_scope1</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
            <devGroup group="CCDs">
                <device label="Camera 1">
                    <driver name="camera1">indi_camera1</driver>
                    <version>2.0</version>
                </device>
            </devGroup>
        </root>
    )";

    createXMLFile("devices.xml", multiGroupXML);
    EXPECT_TRUE(collection->parseDrivers(testDir.string()));

    auto families = collection->getFamilies();
    EXPECT_EQ(families.size(), 2);
    EXPECT_TRUE(families.contains("Telescopes"));
    EXPECT_TRUE(families.contains("CCDs"));
}

TEST_F(INDIDriverCollectionTest, MalformedXML) {
    const char* malformedXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Bad Device">
                    <driver>Incomplete XML
        </root>
    )";

    createXMLFile("malformed.xml", malformedXML);
    EXPECT_TRUE(collection->parseDrivers(testDir.string()));
    EXPECT_EQ(collection->getByLabel("Bad Device"), nullptr);
}

TEST_F(INDIDriverCollectionTest, SortingFunctionality) {
    const char* multiDeviceXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Z Scope">
                    <driver name="scope_z">indi_scope_z</driver>
                    <version>1.0</version>
                </device>
                <device label="A Scope">
                    <driver name="scope_a">indi_scope_a</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </root>
    )";

    createXMLFile("sorted.xml", multiDeviceXML);
    EXPECT_TRUE(collection->parseDrivers(testDir.string()));

    auto families = collection->getFamilies();
    auto& telescopes = families["Telescopes"];
    ASSERT_EQ(telescopes.size(), 2);
    EXPECT_EQ(telescopes[0], "A Scope");
    EXPECT_EQ(telescopes[1], "Z Scope");
}

TEST_F(INDIDriverCollectionTest, MissingAttributes) {
    const char* missingAttrXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup>
                <device>
                    <driver name="test">indi_test</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </root>
    )";

    createXMLFile("missing_attr.xml", missingAttrXML);
    EXPECT_TRUE(collection->parseDrivers(testDir.string()));

    auto families = collection->getFamilies();
    EXPECT_TRUE(families.empty());
}

TEST_F(INDIDriverCollectionTest, MultipleFiles) {
    const char* file1XML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Scope 1">
                    <driver name="scope1">indi_scope1</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </root>
    )";

    const char* file2XML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="CCDs">
                <device label="Camera 1">
                    <driver name="camera1">indi_camera1</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </root>
    )";

    createXMLFile("file1.xml", file1XML);
    createXMLFile("file2.xml", file2XML);

    EXPECT_TRUE(collection->parseDrivers(testDir.string()));

    EXPECT_NE(collection->getByLabel("Scope 1"), nullptr);
    EXPECT_NE(collection->getByLabel("Camera 1"), nullptr);
}

TEST_F(INDIDriverCollectionTest, ParseDeviceValidComplete) {
    const char* validXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <root>
            <devGroup group="Telescopes">
                <device label="Test Device" skel="test.xml">
                    <driver name="test_driver">test_binary</driver>
                    <version>2.0</version>
                </device>
            </devGroup>
        </root>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(validXML), tinyxml2::XML_SUCCESS);

    auto* device = doc.FirstChildElement("root")
                       ->FirstChildElement("devGroup")
                       ->FirstChildElement("device");

    auto result = collection->parseDevice(device, "Telescopes");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->label, "Test Device");
    EXPECT_EQ(result->name, "test_driver");
    EXPECT_EQ(result->binary, "test_binary");
    EXPECT_EQ(result->version, "2.0");
    EXPECT_EQ(result->family, "Telescopes");
    EXPECT_EQ(result->skeleton, "test.xml");
}

TEST_F(INDIDriverCollectionTest, ParseDeviceMissingLabel) {
    const char* missingLabelXML = R"(
        <device>
            <driver name="test_driver">test_binary</driver>
            <version>1.0</version>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(missingLabelXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    EXPECT_EQ(result, nullptr);
}

TEST_F(INDIDriverCollectionTest, ParseDeviceMissingDriver) {
    const char* missingDriverXML = R"(
        <device label="Test Device">
            <version>1.0</version>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(missingDriverXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    EXPECT_EQ(result, nullptr);
}

TEST_F(INDIDriverCollectionTest, ParseDeviceMissingDriverName) {
    const char* missingNameXML = R"(
        <device label="Test Device">
            <driver>test_binary</driver>
            <version>1.0</version>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(missingNameXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    EXPECT_EQ(result, nullptr);
}

TEST_F(INDIDriverCollectionTest, ParseDeviceMissingBinary) {
    const char* missingBinaryXML = R"(
        <device label="Test Device">
            <driver name="test_driver"></driver>
            <version>1.0</version>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(missingBinaryXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    EXPECT_EQ(result, nullptr);
}

TEST_F(INDIDriverCollectionTest, ParseDeviceDefaultVersion) {
    const char* noVersionXML = R"(
        <device label="Test Device">
            <driver name="test_driver">test_binary</driver>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(noVersionXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->version, "0.0");
}

TEST_F(INDIDriverCollectionTest, ParseDeviceEmptyVersion) {
    const char* emptyVersionXML = R"(
        <device label="Test Device">
            <driver name="test_driver">test_binary</driver>
            <version></version>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(emptyVersionXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->version, "0.0");
}

TEST_F(INDIDriverCollectionTest, ParseDeviceNoSkeleton) {
    const char* noSkelXML = R"(
        <device label="Test Device">
            <driver name="test_driver">test_binary</driver>
            <version>1.0</version>
        </device>
    )";

    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(noSkelXML), tinyxml2::XML_SUCCESS);

    auto result =
        collection->parseDevice(doc.FirstChildElement("device"), "Telescopes");
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->skeleton.empty());
}

TEST_F(INDIDriverCollectionTest, ParseDeviceNullArguments) {
    auto result = collection->parseDevice(nullptr, "Telescopes");
    EXPECT_EQ(result, nullptr);

    const char* validXML =
        "<device label='Test'><driver name='test'>binary</driver></device>";
    tinyxml2::XMLDocument doc;
    ASSERT_EQ(doc.Parse(validXML), tinyxml2::XML_SUCCESS);

    result = collection->parseDevice(doc.FirstChildElement("device"), nullptr);
    EXPECT_EQ(result, nullptr);
}