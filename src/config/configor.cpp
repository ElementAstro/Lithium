/*
 * configor.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-30

Description: Configor

**************************************************/

#include "configor.hpp"

#include <fstream>
#include <mutex>
#include <ranges>
#include <shared_mutex>

#include <asio.hpp>

#include "addon/manager.hpp"
#include "script/pycaller.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/env.hpp"
#include "atom/type/json.hpp"

#include "utils/constant.hpp"

using json = nlohmann::json;

namespace lithium {

namespace internal {
auto removeComments(const std::string& json5) -> std::string {
    std::string result;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;
    size_t index = 0;

    while (index < json5.size()) {
        // Check for single-line comments
        if (!inMultiLineComment && !inSingleLineComment &&
            index + 1 < json5.size() && json5[index] == '/' &&
            json5[index + 1] == '/') {
            inSingleLineComment = true;
            index += 2;  // Skip "//"
        }
        // Check for multi-line comments
        else if (!inSingleLineComment && !inMultiLineComment &&
                 index + 1 < json5.size() && json5[index] == '/' &&
                 json5[index + 1] == '*') {
            inMultiLineComment = true;
            index += 2;  // Skip "/*"
        }
        // Handle end of single-line comments
        else if (inSingleLineComment && json5[index] == '\n') {
            inSingleLineComment = false;  // End single-line comment at newline
            result += '\n';               // Keep the newline
            index++;                      // Move to the next character
        }
        // Handle end of multi-line comments
        else if (inMultiLineComment && index + 1 < json5.size() &&
                 json5[index] == '*' && json5[index + 1] == '/') {
            inMultiLineComment = false;  // End multi-line comment at "*/"
            index += 2;                  // Skip "*/"
        }
        // Handle multi-line strings
        else if (!inSingleLineComment && !inMultiLineComment &&
                 json5[index] == '"') {
            result += json5[index];  // Add starting quote
            index++;                 // Move to the string content
            while (index < json5.size() &&
                   (json5[index] != '"' || json5[index - 1] == '\\')) {
                // Check if the end of the string is reached
                if (json5[index] == '\\' && index + 1 < json5.size() &&
                    json5[index + 1] == '\n') {
                    // Handle multi-line strings
                    index += 2;  // Skip backslash and newline
                } else {
                    result += json5[index];
                    index++;
                }
            }
            if (index < json5.size()) {
                result += json5[index];  // Add ending quote
            }
            index++;  // Move to the next character
        }
        // If not in a comment, add character to result
        else if (!inSingleLineComment && !inMultiLineComment) {
            result += json5[index];
            index++;
        } else {
            index++;  // If in a comment, continue moving
        }
    }

    return result;
}

auto trimQuotes(const std::string& str) -> std::string {
    if (str.front() == '"' && str.back() == '"') {
        return str.substr(
            1, str.size() - 2);  // Remove leading and trailing quotes
    }
    return str;
}

auto convertJSON5toJSON(const std::string& json5) -> std::string {
    std::string json = removeComments(json5);

    // Handle keys without quotes
    std::string result;
    bool inString = false;
    size_t index = 0;

    while (index < json.size()) {
        // Check for the start of a string
        if (json[index] == '"') {
            inString = true;
            result += json[index];
        } else if ((std::isspace(static_cast<unsigned char>(json[index])) !=
                    0) &&
                   !inString) {
            result += json[index];  // Keep whitespace
        } else if ((std::isspace(static_cast<unsigned char>(json[index])) ==
                    0) &&
                   !inString &&
                   ((std::isalnum(static_cast<unsigned char>(json[index])) !=
                     0) ||
                    json[index] == '_')) {
            // Add keys without quotes
            size_t start = index;
            while (
                index < json.size() &&
                ((std::isalnum(static_cast<unsigned char>(json[index])) != 0) ||
                 json[index] == '_' || json[index] == '-')) {
                index++;
            }
            result += "\"" + json.substr(start, index - start) +
                      "\"";  // Convert to quoted key
            continue;  // Skip index++, as it has already moved to the end of
                       // the loop
        } else {
            result += json[index];  // Add other characters directly
        }
        index++;
    }

    return result;
}
}  // namespace internal

class ConfigManagerImpl {
public:
    mutable std::shared_mutex rwMutex;
    json config;
    asio::io_context ioContext;
    std::thread ioThread;
};

ConfigManager::ConfigManager()
    : m_impl_(std::make_unique<ConfigManagerImpl>()) {
    asio::executor_work_guard<asio::io_context::executor_type> workGuard(
        m_impl_->ioContext.get_executor());
    m_impl_->ioThread = std::thread([this] { m_impl_->ioContext.run(); });
    if (loadFromFile("config.json")) {
        DLOG_F(INFO, "Config loaded successfully.");
    }
}

ConfigManager::~ConfigManager() {
    m_impl_->ioContext.stop();
    if (m_impl_->ioThread.joinable()) {
        m_impl_->ioThread.join();
    }
    if (saveToFile("config.json")) {
        DLOG_F(INFO, "Config saved successfully.");
    }
}

auto ConfigManager::createShared() -> std::shared_ptr<ConfigManager> {
    static std::shared_ptr<ConfigManager> instance =
        std::make_shared<ConfigManager>();
    return instance;
}

auto ConfigManager::createUnique() -> std::unique_ptr<ConfigManager> {
    return std::make_unique<ConfigManager>();
}

auto ConfigManager::loadFromFile(const fs::path& path) -> bool {
    std::shared_lock lock(m_impl_->rwMutex);
    try {
        std::ifstream ifs(path);
        if (!ifs || ifs.peek() == std::ifstream::traits_type::eof()) {
            LOG_F(ERROR, "Failed to open file: {}", path.string());
            return false;
        }
        json j = json::parse(ifs);
        if (j.empty()) {
            LOG_F(WARNING, "Config file is empty: {}", path.string());
            return false;
        }
        mergeConfig(j);
        LOG_F(INFO, "Config loaded from file: {}", path.string());
        return true;
    } catch (const json::exception& e) {
        LOG_F(ERROR, "Failed to parse file: {}, error message: {}",
              path.string(), e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to load config file: {}, error message: {}",
              path.string(), e.what());
    }
    return false;
}

auto ConfigManager::loadFromDir(const fs::path& dir_path,
                                bool recursive) -> bool {
    std::shared_lock lock(m_impl_->rwMutex);
    std::weak_ptr<ComponentManager> componentManagerPtr;
    GET_OR_CREATE_WEAK_PTR(componentManagerPtr, ComponentManager,
                           Constants::COMPONENT_MANAGER);
    auto componentManager = componentManagerPtr.lock();
    if (!componentManager) {
        LOG_F(ERROR, "ComponentManager not found");
        return false;
    }
    std::shared_ptr<Component> yamlToJsonComponent;
    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                if (entry.path().extension() == ".json" ||
                    entry.path().extension() == ".lithium") {
                    if (!loadFromFile(entry.path())) {
                        LOG_F(WARNING, "Failed to load config file: {}",
                              entry.path().string());
                    }
                } else if (entry.path().extension() == ".json5" ||
                           entry.path().extension() == ".lithium5") {
                    std::ifstream ifs(entry.path());
                    if (!ifs ||
                        ifs.peek() == std::ifstream::traits_type::eof()) {
                        LOG_F(ERROR, "Failed to open file: {}",
                              entry.path().string());
                        return false;
                    }
                    std::string json5((std::istreambuf_iterator<char>(ifs)),
                                      std::istreambuf_iterator<char>());
                    json j = json::parse(internal::convertJSON5toJSON(json5));
                    if (j.empty()) {
                        LOG_F(WARNING, "Config file is empty: {}",
                              entry.path().string());
                        return false;
                    }
                    mergeConfig(j);
                } else if (entry.path().extension() == ".yaml") {
                    // There we will use yaml->json component to convert yaml to
                    // json
                    if (!yamlToJsonComponent) {
                        yamlToJsonComponent =
                            componentManager->getComponent("yamlToJson")
                                .value()
                                .lock();
                        if (!yamlToJsonComponent) {
                            LOG_F(ERROR, "yamlToJson component not found");
                            return false;
                        }
                    }
                    try {
                        yamlToJsonComponent->dispatch("yaml_to_json",
                                                      entry.path().string());
                    } catch (const std::exception& e) {
                        LOG_F(ERROR, "Failed to convert yaml to json: {}",
                              e.what());
                        // Here we will try to use python to convert yaml to
                        // json
                        std::shared_ptr<PythonManager> pythonManager;
                        GET_OR_CREATE_PTR(pythonManager, PythonManager,
                                          Constants::PYTHON_MANAGER);
                        pythonManager->loadScript("yaml_to_json.py",
                                                  "yamlToJson");
                        pythonManager->callFunction<void>(
                            "yamlToJson", "yaml_to_json",
                            entry.path().string());
                        if (!atom::io::isFileExists("yaml_to_json.json")) {
                            LOG_F(ERROR, "Failed to convert yaml to json");
                            return false;
                        }
                    }
                    if (!loadFromFile(entry.path())) {
                        LOG_F(WARNING, "Failed to load config file: {}",
                              entry.path().string());
                    }
                }
            } else if (recursive && entry.is_directory()) {
                loadFromDir(entry.path(), true);
            }
        }
        LOG_F(INFO, "Config loaded from directory: {}", dir_path.string());
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to load config file from: {}, error message: {}",
              dir_path.string(), e.what());
        return false;
    }
}

auto ConfigManager::getValue(const std::string& key_path) const
    -> std::optional<json> {
    std::shared_lock lock(m_impl_->rwMutex);
    const json* p = &m_impl_->config;
    for (const auto& key : key_path | std::views::split('/')) {
        std::string keyStr = std::string(key.begin(), key.end());
        if (p->is_object() && p->contains(keyStr)) {
            p = &(*p)[keyStr];
        } else {
            LOG_F(WARNING, "Key not found: {}", key_path);
            return std::nullopt;
        }
    }
    return *p;
}

auto ConfigManager::setValue(const std::string& key_path,
                             const json& value) -> bool {
    std::unique_lock lock(m_impl_->rwMutex);

    // Check if the key_path is "/" and set the root value directly
    if (key_path == "/") {
        m_impl_->config = value;
        LOG_F(INFO, "Set root config: {}", m_impl_->config.dump());
        return true;
    }

    json* p = &m_impl_->config;
    auto keys = key_path | std::views::split('/');

    for (auto it = keys.begin(); it != keys.end(); ++it) {
        std::string keyStr = std::string((*it).begin(), (*it).end());
        LOG_F(INFO, "Set config: {}", keyStr);

        if (std::next(it) == keys.end()) {  // If this is the last key
            (*p)[keyStr] = value;
            LOG_F(INFO, "Final config: {}", m_impl_->config.dump());
            return true;
        }

        if (!p->contains(keyStr) || !(*p)[keyStr].is_object()) {
            (*p)[keyStr] = json::object();
        }
        p = &(*p)[keyStr];
        LOG_F(INFO, "Current config: {}", p->dump());
    }
    return false;
}

auto ConfigManager::setValue(const std::string& key_path,
                             json&& value) -> bool {
    std::unique_lock lock(m_impl_->rwMutex);

    // Check if the key_path is "/" and set the root value directly
    if (key_path == "/") {
        m_impl_->config = std::move(value);
        LOG_F(INFO, "Set root config: {}", m_impl_->config.dump());
        return true;
    }

    json* p = &m_impl_->config;
    auto keys = key_path | std::views::split('/');

    for (auto it = keys.begin(); it != keys.end(); ++it) {
        std::string keyStr = std::string((*it).begin(), (*it).end());
        LOG_F(INFO, "Set config: {}", keyStr);

        if (std::next(it) == keys.end()) {  // If this is the last key
            (*p)[keyStr] = std::move(value);
            LOG_F(INFO, "Final config: {}", m_impl_->config.dump());
            return true;
        }

        if (!p->contains(keyStr) || !(*p)[keyStr].is_object()) {
            (*p)[keyStr] = json::object();
        }
        p = &(*p)[keyStr];
        LOG_F(INFO, "Current config: {}", p->dump());
    }
    return false;
}

auto ConfigManager::appendValue(const std::string& key_path,
                                const json& value) -> bool {
    std::unique_lock lock(m_impl_->rwMutex);

    json* p = &m_impl_->config;
    auto keys = key_path | std::views::split('/');

    for (auto it = keys.begin(); it != keys.end(); ++it) {
        std::string keyStr = std::string((*it).begin(), (*it).end());

        if (std::next(it) == keys.end()) {  // If this is the last key
            if (!p->contains(keyStr)) {
                (*p)[keyStr] = json::array();
            }

            if (!(*p)[keyStr].is_array()) {
                LOG_F(ERROR, "Target key is not an array");
                return false;
            }

            (*p)[keyStr].push_back(value);
            LOG_F(INFO, "Appended value to config: {}", m_impl_->config.dump());
            return true;
        }

        if (!p->contains(keyStr) || !(*p)[keyStr].is_object()) {
            (*p)[keyStr] = json::object();
        }
        p = &(*p)[keyStr];
    }
    return false;
}

auto ConfigManager::deleteValue(const std::string& key_path) -> bool {
    std::unique_lock lock(m_impl_->rwMutex);
    json* p = &m_impl_->config;
    std::vector<std::string> keys;
    for (const auto& key : key_path | std::views::split('/')) {
        keys.emplace_back(key.begin(), key.end());
    }
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (std::next(it) == keys.end()) {
            if (p->is_object() && p->contains(*it)) {
                p->erase(*it);
                LOG_F(INFO, "Deleted key: {}", key_path);
                return true;
            }
            LOG_F(WARNING, "Key not found for deletion: {}", key_path);
            return false;
        }
        if (!p->is_object() || !p->contains(*it)) {
            LOG_F(WARNING, "Key not found for deletion: {}", key_path);
            return false;
        }
        p = &(*p)[*it];
    }
    return false;
}

auto ConfigManager::hasValue(const std::string& key_path) const -> bool {
    return getValue(key_path).has_value();
}

auto ConfigManager::saveToFile(const fs::path& file_path) const -> bool {
    std::unique_lock lock(m_impl_->rwMutex);
    std::ofstream ofs(file_path);
    if (!ofs) {
        LOG_F(ERROR, "Failed to open file: {}", file_path.string());
        return false;
    }
    try {
        ofs << m_impl_->config.dump(4);
        ofs.close();
        LOG_F(INFO, "Config saved to file: {}", file_path.string());
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to save config to file: {}, error message: {}",
              file_path.string(), e.what());
        return false;
    }
}

void ConfigManager::tidyConfig() {
    std::unique_lock lock(m_impl_->rwMutex);
    json updatedConfig;
    for (const auto& [key, value] : m_impl_->config.items()) {
        json* p = &updatedConfig;
        for (const auto& subKey : key | std::views::split('/')) {
            std::string subKeyStr = std::string(subKey.begin(), subKey.end());
            if (!p->contains(subKeyStr)) {
                (*p)[subKeyStr] = json::object();
            }
            p = &(*p)[subKeyStr];
        }
        *p = value;
    }
    m_impl_->config = std::move(updatedConfig);
    LOG_F(INFO, "Config tidied.");
}

void ConfigManager::mergeConfig(const json& src, json& target) {
    for (auto it = src.begin(); it != src.end(); ++it) {
        LOG_F(INFO, "Merge config: {}", it.key());
        if (it->is_object() && target.contains(it.key()) &&
            target[it.key()].is_object()) {
            mergeConfig(*it, target[it.key()]);
        } else {
            target[it.key()] = *it;
        }
    }
}

void ConfigManager::mergeConfig(const json& src) {
    LOG_F(INFO, "Current config: {}", m_impl_->config.dump());
    std::function<void(json&, const json&)> merge = [&](json& target,
                                                        const json& source) {
        for (auto it = source.begin(); it != source.end(); ++it) {
            if (it.value().is_object() && target.contains(it.key()) &&
                target[it.key()].is_object()) {
                LOG_F(INFO, "Merge config: {}", it.key());
                merge(target[it.key()], it.value());
            } else {
                LOG_F(INFO, "Merge config: {}", it.key());
                target[it.key()] = it.value();
            }
        }
    };

    merge(m_impl_->config, src);
    LOG_F(INFO, "Config merged.");
}

void ConfigManager::clearConfig() {
    std::unique_lock lock(m_impl_->rwMutex);
    m_impl_->config.clear();
    LOG_F(INFO, "Config cleared.");
}

void ConfigManager::asyncLoadFromFile(const fs::path& path,
                                      std::function<void(bool)> callback) {
    asio::post(m_impl_->ioContext, [this, path, callback]() {
        bool success = loadFromFile(path);
        // Post back to caller's thread or IO context
        asio::post(m_impl_->ioContext,
                   [callback, success]() { callback(success); });
    });
}

void ConfigManager::asyncSaveToFile(const fs::path& file_path,
                                    std::function<void(bool)> callback) const {
    asio::post(m_impl_->ioContext, [this, file_path, callback]() {
        bool success = saveToFile(file_path);
        // Post back to caller's thread or IO context
        asio::post(m_impl_->ioContext,
                   [callback, success]() { callback(success); });
    });
}

auto ConfigManager::getKeys() const -> std::vector<std::string> {
    std::shared_lock lock(m_impl_->rwMutex);
    std::vector<std::string> paths;
    std::function<void(const json&, std::string)> listPaths =
        [&](const json& j, std::string path) {
            for (auto it = j.begin(); it != j.end(); ++it) {
                if (it.value().is_object()) {
                    listPaths(it.value(), path + "/" + it.key());
                } else {
                    paths.emplace_back(path + "/" + it.key());
                }
            }
        };
    listPaths(m_impl_->config, "");
    return paths;
}

auto ConfigManager::listPaths() const -> std::vector<std::string> {
    std::shared_lock lock(m_impl_->rwMutex);
    std::vector<std::string> paths;
    std::weak_ptr<atom::utils::Env> envPtr;
    GET_OR_CREATE_WEAK_PTR(envPtr, atom::utils::Env, Constants::ENVIRONMENT);
    auto env = envPtr.lock();
    if (!env) {
        LOG_F(ERROR, "Failed to get environment instance");
        return paths;
    }

    // Get the config directory from the command line arguments
    auto configDir = env->get("config");
    if (configDir.empty() || !atom::io::isFolderExists(configDir)) {
        // Get the config directory from the environment if not set or invalid
        configDir = env->getEnv("LITHIUM_CONFIG_DIR", "./config");
    }

    // Check for JSON files in the config directory
    return atom::io::checkFileTypeInFolder(configDir, {".json"},
                                           atom::io::FileOption::PATH);
}
}  // namespace lithium
