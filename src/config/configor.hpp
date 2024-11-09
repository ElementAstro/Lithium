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

#include "atom/error/exception.hpp"
#include "atom/type/json_fwd.hpp"

#include "utils/constant.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

#define GetIntConfig(path)                           \
    GetPtr<ConfigManager>(Constatns::CONFIG_MANAGER) \
        .value()                                     \
        ->getValue(path)                             \
        .value()                                     \
        .get<int>()

#define GetFloatConfig(path)                         \
    GetPtr<ConfigManager>(Constatns::CONFIG_MANAGER) \
        .value()                                     \
        ->getValue(path)                             \
        .value()                                     \
        .get<float>()

#define GetBoolConfig(path)                          \
    GetPtr<ConfigManager>(Constatns::CONFIG_MANAGER) \
        .value()                                     \
        ->getValue(path)                             \
        .value()                                     \
        .get<bool>()

#define GetDoubleConfig(path)                        \
    GetPtr<ConfigManager>(Constatns::CONFIG_MANAGER) \
        .value()                                     \
        ->getValue(path)                             \
        .value()                                     \
        .get<double>()

#define GetStringConfig(path)                        \
    GetPtr<ConfigManager>(Constatns::CONFIG_MANAGER) \
        .value()                                     \
        ->getValue(path)                             \
        .value()                                     \
        .get<std::string>()

#define GET_CONFIG_VALUE(configManager, path, type, outputVar)              \
    type outputVar;                                                         \
    do {                                                                    \
        auto opt = (configManager)->getValue(path);                         \
        if (opt.has_value()) {                                              \
            try {                                                           \
                (outputVar) = opt.value().get<type>();                      \
            } catch (const std::bad_optional_access& e) {                   \
                LOG_F(ERROR, "Bad access to config value for {}: {}", path, \
                      e.what());                                            \
                THROW_BAD_CONFIG_EXCEPTION(e.what());                       \
            } catch (const json::exception& e) {                            \
                LOG_F(ERROR, "Invalid config value for {}: {}", path,       \
                      e.what());                                            \
                THROW_INVALID_CONFIG_EXCEPTION(e.what());                   \
            } catch (const std::exception& e) {                             \
                THROW_UNKOWN(e.what());                                     \
            }                                                               \
        } else {                                                            \
            LOG_F(WARNING, "Config value for {} not found", path);          \
            THROW_OBJ_NOT_EXIST("Config value for", path);                  \
        }                                                                   \
    } while (0)

class BadConfigException : public atom::error::Exception {
    using atom::error::Exception::Exception;
};

#define THROW_BAD_CONFIG_EXCEPTION(...)                                      \
    throw BadConfigException(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                             __VA_ARGS__)

#define THROW_NESTED_BAD_CONFIG_EXCEPTION(...)                        \
    BadConfigException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

class InvalidConfigException : public BadConfigException {
    using BadConfigException::BadConfigException;
};

#define THROW_INVALID_CONFIG_EXCEPTION(...)                      \
    throw InvalidConfigException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                 ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_INVALID_CONFIG_EXCEPTION(...)                        \
    InvalidConfigException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                          ATOM_FUNC_NAME, __VA_ARGS__)

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
     * @return std::optional<json> The optional JSON value if found.
     */
    [[nodiscard]] auto getValue(const std::string& key_path) const
        -> std::optional<json>;

    /**
     * @brief Sets the value for the specified key path.
     * @param key_path The path to set the configuration value.
     * @param value The JSON value to set.
     * @return bool True if the value was successfully set, false otherwise.
     */
    auto setValue(const std::string& key_path, const json& value) -> bool;

    /**
     * @brief Sets the value for the specified key path.
     * @param key_path The path to set the configuration value.
     * @param value The JSON value to set.
     * @return bool True if the value was successfully set, false otherwise.
     */
    auto setValue(const std::string& key_path, json&& value) -> bool;
    /**
     * @brief Appends a value to an array at the specified key path.
     * @param key_path The path to the array.
     * @param value The JSON value to append.
     * @return bool True if the value was successfully appended, false
     * otherwise.
     */
    auto appendValue(const std::string& key_path, const json& value) -> bool;

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
     * @brief Retrieves all keys in the configuration.
     * @return std::vector<std::string> A vector of keys in the configuration.
     */
    [[nodiscard]] auto getKeys() const -> std::vector<std::string>;

    /**
     * @brief Lists all configuration files in specified directory.
     * @return std::vector<std::string> A vector of paths to configuration
     * files.
     */
    [[nodiscard]] auto listPaths() const -> std::vector<std::string>;

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
    void mergeConfig(const json& src);

    /**
     * @brief Asynchronously loads configuration data from a file.
     * @param path The path to the file containing configuration data.
     * @param callback The callback function to invoke upon completion.
     */
    void asyncLoadFromFile(const fs::path& path,
                           std::function<void(bool)> callback);

    /**
     * @brief Asynchronously saves the current configuration to a file.
     * @param file_path The path to save the configuration file.
     * @param callback The callback function to invoke upon completion.
     */
    void asyncSaveToFile(const fs::path& file_path,
                         std::function<void(bool)> callback) const;

private:
    std::unique_ptr<ConfigManagerImpl>
        m_impl_;  ///< Implementation-specific pointer.

    void mergeConfig(const json& src, json& target);
};

}  // namespace lithium

#endif
