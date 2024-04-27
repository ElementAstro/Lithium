/*
 * configor.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-4

Description: Configor

**************************************************/

#ifndef LITHIUM_CONFIG_CONFIGOR_HPP
#define LITHIUM_CONFIG_CONFIGOR_HPP

#include <filesystem>
#include <mutex>
#include <shared_mutex>
namespace fs = std::filesystem;

#include "error/error_code.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

#define GetIntConfig(path)                  \
    GetPtr<ConfigManager>("lithium.config") \
        .value()                            \
        ->getValue(path)                    \
        .value()                            \
        .get<int>()

#define GetFloatConfig(path)                \
    GetPtr<ConfigManager>("lithium.config") \
        .value()                            \
        ->getValue(path)                    \
        .value()                            \
        .get<float>()

#define GetBoolConfig(path)                 \
    GetPtr<ConfigManager>("lithium.config") \
        .value()                            \
        ->getValue(path)                    \
        .value()                            \
        .get<bool>()

#define GetDoubleConfig(path)               \
    GetPtr<ConfigManager>("lithium.config") \
        .value()                            \
        ->getValue(path)                    \
        .value()                            \
        .get<double>()

#define GetStringConfig(path)               \
    GetPtr<ConfigManager>("lithium.config") \
        .value()                            \
        ->getValue(path)                    \
        .value()                            \
        .get<std::string>()

class ConfigManagerImpl;

class ConfigManager {
public:
    /**
     * @brief 构造函数
     *
     * Constructor.
     */
    ConfigManager();

    /**
     * @brief 析构函数
     *
     * Destructor.
     */
    ~ConfigManager();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * @brief 创建ConfigManager的共享指针，但是全局唯一
     *
     * Create a shared pointer of ConfigManager. But global only
     */
    static std::shared_ptr<ConfigManager> createShared();

    static std::unique_ptr<ConfigManager> createUnique();

    // -------------------------------------------------------------------
    // Config methods
    // -------------------------------------------------------------------

    /**
     * @brief 获取一个配置项的值
     *
     * Get the value of a configuration item.
     *
     * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如
     * "database/username"
     * @return json 配置项对应的值，如果键不存在则返回 nullptr
     */
    [[nodiscard("config value should not be ignored!")]] std::optional<json>
    getValue(const std::string& key_path) const;

    /**
     * @brief 添加或更新一个配置项
     *
     * Add or update a configuration item.
     *
     * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如
     * "database/username"
     * @param value 配置项的值，使用 JSON 格式进行表示
     * @return bool 成功返回 true，失败返回 false
     */
    bool setValue(const std::string& key_path, const json& value);
    /**
     * @brief 删除一个配置项
     *
     * Delete a configuration item.
     *
     * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如
     * "database/username"
     */

    bool deleteValue(const std::string& key_path);

    /**
     * @brief 判断一个配置项是否存在
     *
     * Determine if a configuration item exists.
     *
     * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如
     * "database/username"
     * @return bool 存在返回 true，不存在返回 false
     */
    [[nodiscard("status of the value should not be ignored")]] bool hasValue(
        const std::string& key_path) const;

    /**
     * @brief 从指定文件中加载JSON配置，并与原有配置进行合并
     *
     * Load JSON configuration from the specified file and merge with the
     * existing configuration.
     *
     * @param path 配置文件路径
     */
    bool loadFromFile(const fs::path& path);

    /**
     * @brief 加载指定目录下的所有JSON配置文件
     *
     * Load all JSON configuration files in the specified directory.
     *
     * @param dir_path 配置文件所在目录的路径
     */
    bool loadFromDir(const fs::path& dir_path, bool recursive = false);

    /**
     * @brief 将当前配置保存到指定文件
     *
     * Save the current configuration to the specified file.
     *
     * @param file_path 目标文件路径
     */
    bool saveToFile(const fs::path& file_path) const;

    /**
     * @brief 清理配置项
     *
     * Clean up configuration items.
     */
    void tidyConfig();

    /**
     * @brief 清除所有配置（测试用）
     *
     * Clear all of the configurations, only used for test
     */
    void clearConfig();

private:
    std::unique_ptr<ConfigManagerImpl> m_impl;

    /**
     * @brief 将 JSON 配置合并到当前配置中
     *
     * Merge JSON configuration to the current configuration.
     *
     * @param j JSON 配置
     */
    void mergeConfig(const json& j);
};

#endif
