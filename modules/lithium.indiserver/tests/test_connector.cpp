#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "iconnector.hpp"

#include <filesystem>
#include <fstream>

class INDIConnectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        testConfigPath =
            std::filesystem::temp_directory_path() / "indi_test_config";
        testDataPath =
            std::filesystem::temp_directory_path() / "indi_test_data";
        testFifoPath =
            std::filesystem::temp_directory_path() / "indi_test_fifo";

        std::filesystem::create_directories(testConfigPath);
        std::filesystem::create_directories(testDataPath);

        connector = std::make_unique<INDIConnector>(
            "localhost", 7624, testConfigPath.string(), testDataPath.string(),
            testFifoPath.string());
    }

    void TearDown() override {
        if (connector && connector->isRunning()) {
            connector->stopServer();
        }
        std::filesystem::remove_all(testConfigPath);
        std::filesystem::remove_all(testDataPath);
        if (std::filesystem::exists(testFifoPath)) {
            std::filesystem::remove(testFifoPath);
        }
    }

    std::unique_ptr<INDIConnector> connector;
    std::filesystem::path testConfigPath;
    std::filesystem::path testDataPath;
    std::filesystem::path testFifoPath;
};

TEST_F(INDIConnectorTest, StartServerSuccess) {
    ASSERT_FALSE(connector->isRunning());
    ASSERT_TRUE(connector->startServer());
    ASSERT_TRUE(connector->isRunning());

    // Verify FIFO file was created
    EXPECT_TRUE(std::filesystem::exists(testFifoPath));

    // Cleanup
    ASSERT_TRUE(connector->stopServer());
}

TEST_F(INDIConnectorTest, StartServerInvalidPort) {
    auto invalidConnector = std::make_unique<INDIConnector>(
        "localhost",
        70000,  // Invalid port number
        testConfigPath.string(), testDataPath.string(), testFifoPath.string());

    ASSERT_FALSE(invalidConnector->startServer());
    ASSERT_FALSE(invalidConnector->isRunning());
}

TEST_F(INDIConnectorTest, StartServerAlreadyRunning) {
    // Start server first time
    ASSERT_TRUE(connector->startServer());
    ASSERT_TRUE(connector->isRunning());

    // Try starting again
    ASSERT_TRUE(connector->startServer());
    ASSERT_TRUE(connector->isRunning());

    // Cleanup
    ASSERT_TRUE(connector->stopServer());
}

TEST_F(INDIConnectorTest, StartServerFifoCleanup) {
    // Create a dummy FIFO file
    std::ofstream fifo(testFifoPath);
    fifo << "test data" << std::endl;
    fifo.close();

    ASSERT_TRUE(std::filesystem::exists(testFifoPath));

    // Starting server should clean up existing FIFO
    ASSERT_TRUE(connector->startServer());

    // Verify FIFO was recreated
    ASSERT_TRUE(std::filesystem::exists(testFifoPath));

    // Cleanup
    ASSERT_TRUE(connector->stopServer());
}

TEST_F(INDIConnectorTest, StartServerRetryMechanism) {
    // Create a temporary file that blocks the port
    auto tempServer = std::make_unique<INDIConnector>(
        "localhost", 7624, testConfigPath.string(), testDataPath.string(),
        testFifoPath.string());

    ASSERT_TRUE(tempServer->startServer());

    // Try starting another server on same port
    auto conflictingConnector = std::make_unique<INDIConnector>(
        "localhost", 7624, testConfigPath.string(), testDataPath.string(),
        (testFifoPath.string() + "_alt"));

    // Should fail after MAX_RETRY_COUNT attempts
    ASSERT_FALSE(conflictingConnector->startServer());

    // Cleanup
    ASSERT_TRUE(tempServer->stopServer());
}

TEST_F(INDIConnectorTest, StartServerMultipleInstances) {
    // Start first server
    ASSERT_TRUE(connector->startServer());

    // Try starting second server on different port
    auto secondConnector = std::make_unique<INDIConnector>(
        "localhost", 7625, testConfigPath.string(), testDataPath.string(),
        (testFifoPath.string() + "_second"));

    ASSERT_TRUE(secondConnector->startServer());

    // Verify both are running
    ASSERT_TRUE(connector->isRunning());
    ASSERT_TRUE(secondConnector->isRunning());

    // Cleanup
    ASSERT_TRUE(connector->stopServer());
    ASSERT_TRUE(secondConnector->stopServer());
}

TEST_F(INDIConnectorTest, StartServerLogFileCreation) {
    ASSERT_TRUE(connector->startServer());

    // Verify log file was created
    EXPECT_TRUE(std::filesystem::exists("/tmp/indiserver.log"));

    // Cleanup
    ASSERT_TRUE(connector->stopServer());
}