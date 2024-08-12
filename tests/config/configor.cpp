#include "config/configor.hpp"
#include <gtest/gtest.h>
#include "atom/type/json.hpp"
#include "macro.hpp"

#include <fstream>
using json = nlohmann::json;
using namespace lithium;

class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        config_manager_ = ConfigManager::createUnique();
        config_manager_->clearConfig();

        std::fstream file("config.json");
        file << R"({"test":{"key":"file_value"}})";
        file.close();
    }

    std::unique_ptr<ConfigManager> config_manager_;
};

TEST_F(ConfigManagerTest, SetGetValue) {
    json value = "test_value";
    ASSERT_TRUE(config_manager_->setValue("test/key", value));
    auto result = config_manager_->getValue("test/key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), value);
}

TEST_F(ConfigManagerTest, AppendValue) {
    json initialValue = {{"key1", json::array({"initial_value1"})},
                          {"key2", "initial_value2"}};
    config_manager_->setValue("/", initialValue);

    json newValue = "new_value";
    ASSERT_TRUE(config_manager_->appendValue("key1", newValue));

    auto result1 = config_manager_->getValue("key1");
    auto result2 = config_manager_->getValue("key2");

    ASSERT_TRUE(result1.has_value());
    EXPECT_TRUE(result1.value().is_array());
    EXPECT_EQ(result1.value().size(), 2);
    EXPECT_EQ(result1.value()[0], "initial_value1");
    EXPECT_EQ(result1.value()[1], "new_value");

    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "initial_value2");

    // Test appending to a non-existent array key
    ASSERT_TRUE(config_manager_->appendValue("key3", newValue));
    auto result3 = config_manager_->getValue("key3");

    ASSERT_TRUE(result3.has_value());
    EXPECT_TRUE(result3.value().is_array());
    EXPECT_EQ(result3.value().size(), 1);
    EXPECT_EQ(result3.value()[0], "new_value");

    // Test appending to a non-array key
    ASSERT_FALSE(config_manager_->appendValue("key2", newValue));
}

TEST_F(ConfigManagerTest, DeleteValue) {
    json value = "test_value";
    config_manager_->setValue("test/key", value);
    ASSERT_TRUE(config_manager_->deleteValue("test/key"));
    auto result = config_manager_->getValue("test/key");
    ASSERT_FALSE(result.has_value());
}

TEST_F(ConfigManagerTest, HasValue) {
    json value = "test_value";
    config_manager_->setValue("test/key", value);
    EXPECT_TRUE(config_manager_->hasValue("test/key"));
    config_manager_->deleteValue("test/key");
    EXPECT_FALSE(config_manager_->hasValue("test/key"));
}

TEST_F(ConfigManagerTest, LoadFromFile) {
    std::ofstream file("test_config.json");
    file << R"({"test":{"key":"file_value"}})";
    file.close();

    ASSERT_TRUE(config_manager_->loadFromFile("test_config.json"));
    auto result = config_manager_->getValue("test/key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "file_value");

    ATOM_UNREF_PARAM(std::remove("test_config.json"));
}

TEST_F(ConfigManagerTest, SaveToFile) {
    json value = "test_value";
    config_manager_->setValue("test/key", value);
    ASSERT_TRUE(config_manager_->saveToFile("test_save.json"));

    ConfigManager newManager;
    ASSERT_TRUE(newManager.loadFromFile("test_save.json"));
    auto result = newManager.getValue("test/key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), value);

    ATOM_UNUSED_RESULT(std::remove("test_save.json"));
}

TEST_F(ConfigManagerTest, LoadFromDir) {
    fs::create_directory("test_dir");
    std::ofstream file1("test_dir/config1.json");
    file1 << R"({"config1":{"key1":"value1"}})";
    file1.close();
    std::ofstream file2("test_dir/config2.json");
    file2 << R"({"config2":{"key2":"value2"}})";
    file2.close();

    ASSERT_TRUE(config_manager_->loadFromDir("test_dir"));
    auto result1 = config_manager_->getValue("config1/key1");
    auto result2 = config_manager_->getValue("config2/key2");

    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "value1");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "value2");

    fs::remove_all("test_dir");
}

TEST_F(ConfigManagerTest, TidyConfig) {
    json value1 = "value1";
    json value2 = "value2";
    config_manager_->setValue("config1/key1", value1);
    config_manager_->setValue("config2/key2", value2);

    config_manager_->tidyConfig();

    auto result1 = config_manager_->getValue("config1/key1");
    auto result2 = config_manager_->getValue("config2/key2");

    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), value1);
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), value2);
}

TEST_F(ConfigManagerTest, MergeConfig) {
    config_manager_->clearConfig();
    json initialValue = {{"key1", "initial_value1"},
                         {"key2", "initial_value2"}};
    config_manager_->setValue("/", initialValue);

    json newValue = {{"key2", "new_value2"}, {"key3", "new_value3"}};
    config_manager_->mergeConfig(newValue);

    auto result1 = config_manager_->getValue("key1");
    auto result2 = config_manager_->getValue("key2");
    auto result3 = config_manager_->getValue("key3");

    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "initial_value1");
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(result2.value(), "new_value2");
    ASSERT_TRUE(result3.has_value());
    EXPECT_EQ(result3.value(), "new_value3");
}

TEST_F(ConfigManagerTest, ClearConfig) {
    json value = "test_value";
    config_manager_->setValue("test/key", value);
    config_manager_->clearConfig();
    auto result = config_manager_->getValue("test/key");
    ASSERT_FALSE(result.has_value());
}
