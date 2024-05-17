#include "config/configor.cpp"
#include <gtest/gtest.h>


class ConfigManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化 ConfigManager 实例
        config_manager_ = ConfigManager::createShared();
    }

    void TearDown() override {
        // 清理 ConfigManager 实例
        config_manager_.reset();
    }

    std::shared_ptr<ConfigManager> config_manager_;
};

TEST_F(ConfigManagerTest, SetValueAndGet) {
    // 设置配置值
    config_manager_->setValue("test/key1", "value1");
    config_manager_->setValue("test/key2", 42);

    // 获取配置值并验证
    auto value1 = config_manager_->getValue("test/key1");
    EXPECT_TRUE(value1.has_value());
    EXPECT_EQ(value1.value(), "value1");

    auto value2 = config_manager_->getValue("test/key2");
    EXPECT_TRUE(value2.has_value());
    EXPECT_EQ(value2.value(), 42);
}

TEST_F(ConfigManagerTest, DeleteValue) {
    // 设置配置值
    config_manager_->setValue("test/key1", "value1");

    // 删除配置值并验证
    EXPECT_TRUE(config_manager_->deleteValue("test/key1"));
    EXPECT_FALSE(config_manager_->hasValue("test/key1"));
}

TEST_F(ConfigManagerTest, SaveAndLoadFromFile) {
    // 设置配置值
    config_manager_->setValue("test/key1", "value1");
    config_manager_->setValue("test/key2", 42);

    // 保存配置到文件
    std::string file_path = "test_config.json";
    EXPECT_TRUE(config_manager_->saveToFile(file_path));

    // 创建新的 ConfigManager 实例并从文件加载配置
    auto new_config_manager = ConfigManager::createShared();
    EXPECT_TRUE(new_config_manager->loadFromFile(file_path));

    // 验证新的实例中是否包含相同的配置值
    EXPECT_TRUE(new_config_manager->hasValue("test/key1"));
    EXPECT_TRUE(new_config_manager->hasValue("test/key2"));
    EXPECT_EQ(new_config_manager->getValue("test/key1").value(), "value1");
    EXPECT_EQ(new_config_manager->getValue("test/key2").value(), 42);
}

TEST_F(ConfigManagerTest, TidyConfig) {
    // 设置配置值
    config_manager_->setValue("test/key1", "value1");
    config_manager_->setValue("test/key2", 42);

    // 设置包含子对象的配置值
    config_manager_->setValue("test/sub/key3", "value3");

    // 整理配置
    config_manager_->tidyConfig();

    // 验证整理后的配置
    EXPECT_TRUE(config_manager_->hasValue("test"));
    EXPECT_TRUE(config_manager_->hasValue("test/sub"));
    EXPECT_TRUE(config_manager_->hasValue("test/key1"));
    EXPECT_TRUE(config_manager_->hasValue("test/key2"));
    EXPECT_TRUE(config_manager_->hasValue("test/sub/key3"));
}

// 测试异常情况：尝试获取不存在的配置键时返回空的 std::optional
TEST_F(ConfigManagerTest, GetValueNonExistentKey) {
    auto value = config_manager_->getValue("non_existent_key");
    EXPECT_FALSE(value.has_value());
}

// 测试异常情况：尝试删除不存在的配置键时返回 false
TEST_F(ConfigManagerTest, DeleteNonExistentKey) {
    EXPECT_FALSE(config_manager_->deleteValue("non_existent_key"));
}

// 测试异常情况：尝试从无效路径加载配置文件时返回 false
TEST_F(ConfigManagerTest, LoadFromInvalidFile) {
    EXPECT_FALSE(config_manager_->loadFromFile("invalid_file_path.json"));
}

// 测试边界条件：当配置文件为空时加载成功但不包含任何配置项
TEST_F(ConfigManagerTest, LoadEmptyConfigFile) {
    // 创建一个空的配置文件
    std::string file_path = "empty_config.json";
    std::ofstream ofs(file_path);
    ofs.close();

    // 加载空的配置文件
    EXPECT_FALSE(config_manager_->loadFromFile(file_path));

    // 验证配置文件不包含任何配置项
    EXPECT_FALSE(config_manager_->hasValue("a"));
}

// 测试边界条件：在没有配置键值对的情况下保存配置文件
TEST_F(ConfigManagerTest, SaveEmptyConfigToFile) {
    // 保存空的配置到文件
    std::string file_path = "empty_config.json";
    config_manager_->clearConfig();
    EXPECT_TRUE(config_manager_->saveToFile(file_path));

    // 加载保存的配置文件
    EXPECT_FALSE(config_manager_->loadFromFile(file_path));

    // 验证配置文件不包含任何配置项
    EXPECT_FALSE(config_manager_->hasValue("a"));
}
