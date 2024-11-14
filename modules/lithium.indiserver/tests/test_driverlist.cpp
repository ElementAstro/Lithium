#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "driverlist.hpp"

class DriverListTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = std::filesystem::temp_directory_path() / "driver_test";
        std::filesystem::create_directory(testDir);
    }

    void TearDown() override { std::filesystem::remove_all(testDir); }

    void createTestXMLFile(const std::string& filename,
                           const std::string& content) {
        std::ofstream file(testDir / filename);
        file << content;
    }

    std::filesystem::path testDir;
};

TEST_F(DriverListTest, EmptyDirectory) {
    std::vector<Device> devices;
    auto result = parseDevicesFromPath(testDir.string(), devices);

    EXPECT_TRUE(result.empty());
    EXPECT_TRUE(devices.empty());
}

TEST_F(DriverListTest, ValidDeviceXML) {
    const char* validXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <driversList>
            <devGroup group="Telescopes">
                <device label="Test Scope" manufacturer="Test Corp">
                    <driver>indi_test_telescope</driver>
                    <version>1.0</version>
                </device>
            </devGroup>
        </driversList>
    )";

    createTestXMLFile("telescope.xml", validXML);

    std::vector<Device> devices;
    auto groups = parseDevicesFromPath(testDir.string(), devices);

    ASSERT_EQ(groups.size(), 1);
    ASSERT_EQ(groups[0].group, "Telescopes");
    ASSERT_EQ(groups[0].devices.size(), 1);
    ASSERT_EQ(devices.size(), 1);

    const auto& device = groups[0].devices[0];
    EXPECT_EQ(device.label, "Test Scope");
    EXPECT_EQ(device.manufacturer, "Test Corp");
    EXPECT_EQ(device.driverName, "indi_test_telescope");
    EXPECT_EQ(device.version, "1.0");
}

TEST_F(DriverListTest, IgnoreSkXMLFile) {
    const char* skXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <driversList>
            <devGroup group="Telescopes">
                <device label="Should Not Load">
                    <driver>indi_ignored_driver</driver>
                </device>
            </devGroup>
        </driversList>
    )";

    createTestXMLFile("devices_sk.xml", skXML);

    std::vector<Device> devices;
    auto groups = parseDevicesFromPath(testDir.string(), devices);

    EXPECT_TRUE(groups.empty());
    EXPECT_TRUE(devices.empty());
}

TEST_F(DriverListTest, MultipleDeviceGroups) {
    const char* multiGroupXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <driversList>
            <devGroup group="Telescopes">
                <device label="Scope 1">
                    <driver>indi_scope1</driver>
                </device>
            </devGroup>
            <devGroup group="CCDs">
                <device label="Camera 1">
                    <driver>indi_camera1</driver>
                </device>
            </devGroup>
        </driversList>
    )";

    createTestXMLFile("multi_group.xml", multiGroupXML);

    std::vector<Device> devices;
    auto groups = parseDevicesFromPath(testDir.string(), devices);

    ASSERT_EQ(groups.size(), 2);
    EXPECT_EQ(devices.size(), 2);
}

TEST_F(DriverListTest, MalformedXML) {
    const char* malformedXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <driversList>
            <devGroup group="Telescopes">
                <device label="Bad XML
            </devGroup>
        </driversList>
    )";

    createTestXMLFile("malformed.xml", malformedXML);

    std::vector<Device> devices;
    auto groups = parseDevicesFromPath(testDir.string(), devices);

    EXPECT_TRUE(groups.empty());
    EXPECT_TRUE(devices.empty());
}

TEST_F(DriverListTest, MissingAttributes) {
    const char* missingAttrXML = R"(
        <?xml version="1.0" encoding="UTF-8"?>
        <driversList>
            <devGroup>
                <device>
                    <driver>indi_test</driver>
                </device>
            </devGroup>
        </driversList>
    )";

    createTestXMLFile("missing_attr.xml", missingAttrXML);

    std::vector<Device> devices;
    auto groups = parseDevicesFromPath(testDir.string(), devices);

    ASSERT_EQ(groups.size(), 1);
    EXPECT_TRUE(groups[0].group.empty());
    ASSERT_EQ(groups[0].devices.size(), 1);
    EXPECT_TRUE(groups[0].devices[0].label.empty());
}

TEST_F(DriverListTest, NonXMLFiles) {
    std::ofstream(testDir / "not_xml.txt") << "This is not XML";

    std::vector<Device> devices;
    auto groups = parseDevicesFromPath(testDir.string(), devices);

    EXPECT_TRUE(groups.empty());
    EXPECT_TRUE(devices.empty());
}