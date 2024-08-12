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
#include <memory>
#include <optional>
#include <string>
#include "atom/type/json_fwd.hpp"

namespace fs = std::filesystem;

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

namespace lithium {
class ConfigManagerImpl;
/**
 * @brief The ConfigManager class manages configuration data using JSON format.
 *
 * This class provides methods to manipulate configuration values, load from
 * files or directories, save to a file, and perform various operations like
 * merging configurations.
 */
class ConfigManager {
public:
    /**
     * @brief Default constructor.
     */
    ConfigManager();

    /**
     * @brief Destructor.
     */
    ~ConfigManager();

    /**
     * @brief Creates a shared pointer instance of ConfigManager.
     * @return std::shared_ptr<ConfigManager> Shared pointer to ConfigManager
     * instance.
     */
    static auto createShared() -> std::shared_ptr<ConfigManager>;

    /**
     * @brief Creates a unique pointer instance of ConfigManager.
     * @return std::unique_ptr<ConfigManager> Unique pointer to ConfigManager
     * instance.
     */
    static auto createUnique() -> std::unique_ptr<ConfigManager>;

    /**
     * @brief Retrieves the value associated with the given key path.
     * @param key_path The path to the configuration value.
     * @return std::optional<nlohmann::json> The optional JSON value if found.
     */
    [[nodiscard]] auto getValue(const std::string& key_path) const
        -> std::optional<nlohmann::json>;

    /**
     * @brief Sets the value for the specified key path.
     * @param key_path The path to set the configuration value.
     * @param value The JSON value to set.
     * @return bool True if the value was successfully set, false otherwise.
     */
    auto setValue(const std::string& key_path,
                  const nlohmann::json& value) -> bool;

    auto appendValue(const std::string& key_path, const nlohmann::json& value) -> bool;

    /**
     * @brief Deletes the value associated with the given key path.
     * @param key_path The path to the configuration value to delete.
     * @return bool True if the value was successfully deleted, false otherwise.
     */
    auto deleteValue(const std::string& key_path) -> bool;

    /**
     * @brief Checks if a value exists for the given key path.
     * @param key_path The path to check.
     * @return bool True if a value exists for the key path, false otherwise.
     */
    [[nodiscard]] auto hasValue(const std::string& key_path) const -> bool;

    /**
     * @brief Loads configuration data from a file.
     * @param path The path to the file containing configuration data.
     * @return bool True if the file was successfully loaded, false otherwise.
     */
    auto loadFromFile(const fs::path& path) -> bool;

    /**
     * @brief Loads configuration data from a directory.
     * @param dir_path The path to the directory containing configuration files.
     * @param recursive Flag indicating whether to recursively load from
     * subdirectories.
     * @return bool True if the directory was successfully loaded, false
     * otherwise.
     */
    auto loadFromDir(const fs::path& dir_path, bool recursive = false) -> bool;

    /**
     * @brief Saves the current configuration to a file.
     * @param file_path The path to save the configuration file.
     * @return bool True if the configuration was successfully saved, false
     * otherwise.
     */
    [[nodiscard]] auto saveToFile(const fs::path& file_path) const -> bool;

    /**
     * @brief Cleans up the configuration by removing unused entries or
     * optimizing data.
     */
    void tidyConfig();

    /**
     * @brief Clears all configuration data.
     */
    void clearConfig();

    /**
     * @brief Merges the current configuration with the provided JSON data.
     * @param src The JSON object to merge into the current configuration.
     */
    void mergeConfig(const nlohmann::json& src);

private:
    std::unique_ptr<ConfigManagerImpl>
        m_impl_;  ///< Implementation-specific pointer.

    void mergeConfig(const nlohmann::json& src, nlohmann::json& target);
};

}  // namespace lithium

#endif
