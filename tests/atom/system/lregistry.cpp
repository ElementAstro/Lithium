#include <gtest/gtest.h>
#include "atom/system/lregistry.hpp"

#include <filesystem>

using namespace atom::system;
namespace fs = std::filesystem;

// Helper function to clean up test files
void cleanupTestFiles() {
    fs::remove("registry_data.txt");
    fs::remove("test_backup.txt");
    fs::remove("test_restore.txt");
}

// Test fixture for setting up common test environment
class RegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Cleanup before each test
        cleanupTestFiles();
    }

    void TearDown() override {
        // Cleanup after each test
        cleanupTestFiles();
    }

    Registry registry;
};

// Test createKey and keyExists functions
TEST_F(RegistryTest, CreateKey) {
    std::string keyName = "TestKey";
    registry.createKey(keyName);
    EXPECT_TRUE(registry.keyExists(keyName));
}

// Test deleteKey function
TEST_F(RegistryTest, DeleteKey) {
    std::string keyName = "TestKey";
    registry.createKey(keyName);
    registry.deleteKey(keyName);
    EXPECT_FALSE(registry.keyExists(keyName));
}

// Test setValue and getValue functions
TEST_F(RegistryTest, SetValueAndGetValue) {
    std::string keyName = "TestKey";
    std::string valueName = "TestValue";
    std::string data = "Data";
    registry.createKey(keyName);
    registry.setValue(keyName, valueName, data);
    EXPECT_EQ(registry.getValue(keyName, valueName), data);
}

// Test deleteValue function
TEST_F(RegistryTest, DeleteValue) {
    std::string keyName = "TestKey";
    std::string valueName = "TestValue";
    std::string data = "Data";
    registry.createKey(keyName);
    registry.setValue(keyName, valueName, data);
    registry.deleteValue(keyName, valueName);
    EXPECT_EQ(registry.getValue(keyName, valueName), "Value not found");
}

// Test backupRegistryData and restoreRegistryData functions
TEST_F(RegistryTest, BackupAndRestoreRegistryData) {
    std::string keyName = "TestKey";
    std::string valueName = "TestValue";
    std::string data = "Data";
    registry.createKey(keyName);
    registry.setValue(keyName, valueName, data);

    registry.backupRegistryData();

    Registry newRegistry;
    newRegistry.restoreRegistryData("registry_backup_" + std::to_string(std::time(nullptr)) + ".txt");

    EXPECT_EQ(newRegistry.getValue(keyName, valueName), data);
}

// Test getValueNames function
TEST_F(RegistryTest, GetValueNames) {
    std::string keyName = "TestKey";
    registry.createKey(keyName);
    registry.setValue(keyName, "Value1", "Data1");
    registry.setValue(keyName, "Value2", "Data2");

    auto valueNames = registry.getValueNames(keyName);
    EXPECT_EQ(valueNames.size(), 2);
    EXPECT_NE(std::find(valueNames.begin(), valueNames.end(), "Value1"), valueNames.end());
    EXPECT_NE(std::find(valueNames.begin(), valueNames.end(), "Value2"), valueNames.end());
}

// Test valueExists function
TEST_F(RegistryTest, ValueExists) {
    std::string keyName = "TestKey";
    std::string valueName = "TestValue";
    std::string data = "Data";
    registry.createKey(keyName);
    registry.setValue(keyName, valueName, data);
    EXPECT_TRUE(registry.valueExists(keyName, valueName));
    EXPECT_FALSE(registry.valueExists(keyName, "NonexistentValue"));
}

// Test notifyEvent function (implicitly tested via DLOG_F messages, manual check needed)
TEST_F(RegistryTest, NotifyEvent) {
    std::string keyName = "TestKey";
    registry.createKey(keyName);  // Check for "KeyCreated" event in logs
    registry.setValue(keyName, "ValueName", "Data");  // Check for "ValueSet" event in logs
    registry.deleteKey(keyName);  // Check for "KeyDeleted" event in logs
}
