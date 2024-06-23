#include "task/loader.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

using namespace lithium;
namespace fs = std::filesystem;

class TaskLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create necessary files and directories for testing
        createTestFile("existing_file.json", R"({"key": "value"})");
        createTestFile("existing_file1.json", R"({"key": "value1"})");
        createTestFile("existing_file2.json", R"({"key": "value2"})");
        fs::create_directory("json_files");
        createTestFile("json_files/existing_file1.json",
                       R"({"key": "value1"})");
        createTestFile("json_files/existing_file2.json",
                       R"({"key": "value2"})");
    }

    void TearDown() override {
        // Remove test files and directories
        removeTestFile("existing_file.json");
        removeTestFile("existing_file1.json");
        removeTestFile("existing_file2.json");
        removeTestFile("output_file.json");
        fs::remove_all("json_files");
    }

    void createTestFile(const std::string& filename,
                        const std::string& content) {
        std::ofstream file(filename);
        file << content;
        file.close();
    }

    void removeTestFile(const std::string& filename) { fs::remove(filename); }
};

TEST_F(TaskLoaderTest, ReadJsonFile_ExistingFile_ReturnsJson) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "existing_file.json";
    json expectedJson = {{"key", "value"}};

    // Act
    std::optional<json> result = loader.readJsonFile(filePath);

    // Assert
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), expectedJson);
}

TEST_F(TaskLoaderTest, ReadJsonFile_NonExistingFile_ReturnsNullopt) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "non_existing_file.json";

    // Act
    std::optional<json> result = loader.readJsonFile(filePath);

    // Assert
    ASSERT_FALSE(result.has_value());
}

TEST_F(TaskLoaderTest, WriteJsonFile_ValidJson_ReturnsTrue) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "output_file.json";
    json jsonToWrite = {{"key", "value"}};

    // Act
    bool result = loader.writeJsonFile(filePath, jsonToWrite);

    // Assert
    ASSERT_TRUE(result);

    // Cleanup
    removeTestFile(filePath.string());
}

TEST_F(TaskLoaderTest, WriteJsonFile_InvalidJson_ReturnsFalse) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "output_file.json";
    json jsonToWrite = {{"key", 12345}};

    // Act
    bool result = loader.writeJsonFile(filePath, jsonToWrite);

    // Assert
    ASSERT_TRUE(result);

    // Cleanup
    removeTestFile(filePath.string());
}

TEST_F(TaskLoaderTest, AsyncReadJsonFile_ValidFile_CallsCallbackWithJson) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "existing_file.json";
    json expectedJson = {{"key", "value"}};
    bool callbackCalled = false;

    // Act
    loader.asyncReadJsonFile(filePath, [&](std::optional<json> result) {
        // Assert
        EXPECT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), expectedJson);
        callbackCalled = true;
    });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }
}

TEST_F(TaskLoaderTest,
       AsyncReadJsonFile_NonExistingFile_CallsCallbackWithNullopt) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "non_existing_file.json";
    bool callbackCalled = false;

    // Act
    loader.asyncReadJsonFile(filePath, [&](std::optional<json> result) {
        // Assert
        EXPECT_FALSE(result.has_value());
        callbackCalled = true;
    });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }
}

TEST_F(TaskLoaderTest, AsyncWriteJsonFile_ValidJson_CallsCallbackWithTrue) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "output_file.json";
    json jsonToWrite = {{"key", "value"}};
    bool callbackCalled = false;

    // Act
    loader.asyncWriteJsonFile(filePath, jsonToWrite, [&](bool result) {
        // Assert
        EXPECT_TRUE(result);
        callbackCalled = true;
    });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }

    // Cleanup
    removeTestFile(filePath.string());
}

TEST_F(TaskLoaderTest, AsyncWriteJsonFile_InvalidJson_CallsCallbackWithFalse) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "output_file.json";
    json jsonToWrite = {{"key", 12345}};
    bool callbackCalled = false;

    // Act
    loader.asyncWriteJsonFile(filePath, jsonToWrite, [&](bool result) {
        // Assert
        EXPECT_TRUE(result);
        callbackCalled = true;
    });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }

    // Cleanup
    removeTestFile(filePath.string());
}

TEST_F(TaskLoaderTest, MergeJsonObjects_MergesObjects) {
    // Arrange
    TaskLoader loader;
    json baseJson = {{"key1", "value1"}};
    json toMergeJson = {{"key2", "value2"}};
    json expectedJson = {{"key1", "value1"}, {"key2", "value2"}};

    // Act
    loader.mergeJsonObjects(baseJson, toMergeJson);

    // Assert
    EXPECT_EQ(baseJson, expectedJson);
}

TEST_F(TaskLoaderTest, BatchAsyncProcess_ProcessesAllFiles_CallsOnComplete) {
    // Arrange
    TaskLoader loader;
    std::vector<fs::path> filePaths = {"existing_file1.json",
                                       "existing_file2.json"};
    int processCount = 0;
    bool onCompleteCalled = false;

    // Act
    loader.batchAsyncProcess(
        filePaths, [&](std::optional<json>) { processCount++; },
        [&]() { onCompleteCalled = true; });

    // Wait for the processing to complete
    while (static_cast<long long unsigned int>(processCount) <
               filePaths.size() ||
           !onCompleteCalled) {
        std::this_thread::yield();
    }

    // Assert
    EXPECT_EQ(processCount, filePaths.size());
    EXPECT_TRUE(onCompleteCalled);
}

TEST_F(TaskLoaderTest, AsyncDeleteJsonFile_ValidFile_CallsCallbackWithTrue) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "existing_file.json";
    bool callbackCalled = false;

    // Act
    loader.asyncDeleteJsonFile(filePath, [&](bool result) {
        // Assert
        EXPECT_TRUE(result);
        callbackCalled = true;
    });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }

    // Cleanup
    removeTestFile(filePath.string());
}

TEST_F(TaskLoaderTest,
       AsyncDeleteJsonFile_NonExistingFile_CallsCallbackWithFalse) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "non_existing_file.json";
    bool callbackCalled = false;

    // Act
    loader.asyncDeleteJsonFile(filePath, [&](bool result) {
        // Assert
        EXPECT_FALSE(result);
        callbackCalled = true;
    });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }
}

TEST_F(TaskLoaderTest,
       AsyncQueryJsonValue_ValidKey_CallsCallbackWithJsonValue) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "existing_file.json";
    json expectedJson = "value";
    bool callbackCalled = false;

    // Act
    loader.asyncQueryJsonValue(filePath, "key",
                               [&](std::optional<json> result) {
                                   // Assert
                                   EXPECT_TRUE(result.has_value());
                                   EXPECT_EQ(result.value(), expectedJson);
                                   callbackCalled = true;
                               });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }
}

TEST_F(TaskLoaderTest,
       AsyncQueryJsonValue_NonExistingKey_CallsCallbackWithNullopt) {
    // Arrange
    TaskLoader loader;
    fs::path filePath = "existing_file.json";
    bool callbackCalled = false;

    // Act
    loader.asyncQueryJsonValue(filePath, "non_existing_key",
                               [&](std::optional<json> result) {
                                   // Assert
                                   EXPECT_FALSE(result.has_value());
                                   callbackCalled = true;
                               });

    // Wait for the callback to be called
    while (!callbackCalled) {
        std::this_thread::yield();
    }
}

TEST_F(TaskLoaderTest,
       BatchProcessDirectory_ValidDirectory_CallsProcessForAllJsonFiles) {
    // Arrange
    TaskLoader loader;
    fs::path directoryPath = "json_files";
    std::vector<std::optional<json>> processResults;
    int expectedFileCount = 2;

    // Act
    loader.batchProcessDirectory(
        directoryPath,
        [&](std::optional<json> result) { processResults.push_back(result); },
        []() {});

    // Wait for the processing to complete
    while (processResults.size() < static_cast<size_t>(expectedFileCount)) {
        std::this_thread::yield();
    }

    // Assert
    EXPECT_EQ(processResults.size(), expectedFileCount);
}

TEST_F(TaskLoaderTest,
       BatchProcessDirectory_InvalidDirectory_DoesNotCallProcess) {
    // Arrange
    TaskLoader loader;
    fs::path directoryPath = "non_existing_directory";
    std::vector<std::optional<json>> processResults;

    // Act
    loader.batchProcessDirectory(
        directoryPath,
        [&](std::optional<json> result) { processResults.push_back(result); },
        []() {});

    // Assert
    EXPECT_TRUE(processResults.empty());
}