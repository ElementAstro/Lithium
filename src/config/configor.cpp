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
#include <utility>

#include <asio.hpp>

#include "addon/manager.hpp"
#include "script/pycaller.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/env.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/difflib.hpp"
#include "atom/utils/string.hpp"

#include "utils/constant.hpp"

using json = nlohmann::json;

namespace lithium {

namespace internal {
auto removeComments(const std::string& json5) -> std::string {
    std::string result;
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;

    for (size_t index = 0; index < json5.size(); ++index) {
        if (!inMultiLineComment && !inSingleLineComment &&
            json5[index] == '/' && index + 1 < json5.size()) {
            if (json5[index + 1] == '/') {
                inSingleLineComment = true;
                ++index;
            } else if (json5[index + 1] == '*') {
                inMultiLineComment = true;
                ++index;
            } else {
                result += json5[index];
            }
        } else if (inSingleLineComment && json5[index] == '\n') {
            inSingleLineComment = false;
            result += '\n';
        } else if (inMultiLineComment && json5[index] == '*' &&
                   index + 1 < json5.size() && json5[index + 1] == '/') {
            inMultiLineComment = false;
            ++index;
        } else if (!inSingleLineComment && !inMultiLineComment) {
            result += json5[index];
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
    std::string rootPath;
};

ConfigManager::ConfigManager()
    : m_impl_(std::make_unique<ConfigManagerImpl>()) {
    asio::executor_work_guard<asio::io_context::executor_type> workGuard(
        m_impl_->ioContext.get_executor());
    m_impl_->ioThread = std::thread([this] { m_impl_->ioContext.run(); });
    GET_WEAK_PTR(atom::utils::Env, env, ENVIRONMENT);
    m_impl_->rootPath = env->getEnv("LITHIUM_CONFIG_DIR", "./config");
    if (loadFromDir(m_impl_->rootPath)) {
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
    if (!atom::io::isFileExists(path)) {
        LOG_F(ERROR, "Config file not found: {}", path.string());
        return false;
    }
    // Check if the file format is supported
    auto suffix = path.extension().string();
    const std::vector<std::string> SUPPORTED_EXTENSIONS{
        ".json", ".lithium", ".json5", ".lithium5", ".yaml"};
    if (SUPPORTED_EXTENSIONS.end() ==
        std::find_if(SUPPORTED_EXTENSIONS.begin(), SUPPORTED_EXTENSIONS.end(),
                     [&suffix](const std::string& s) { return s == suffix; })) {
        LOG_F(ERROR, "Unsupported config file format: {}", path.string());
        return false;
    }
    try {
        std::ifstream ifs(path);
        if (!ifs) {
            LOG_F(ERROR, "Failed to open file: {}", path.string());
            return false;
        }
        json j = json::parse(ifs, nullptr, false);
        if (j.is_discarded()) {
            LOG_F(ERROR, "Failed to parse file: {}", path.string());
            return false;
        }

        // Merge the loaded config under the appropriate keys
        auto folder = path.parent_path().filename().string();
        auto file = path.stem().string();

        m_impl_->config[folder][file] = j;

        // mergeConfig(j);
        LOG_F(INFO, "Config loaded from file: {}", path.string());
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to load config file: {}, error message: {}",
              path.string(), e.what());
    }
    return false;
}

auto ConfigManager::loadFromDir(const fs::path& dir_path,
                                bool recursive) -> bool {
    std::shared_lock lock(m_impl_->rwMutex);
    if (!atom::io::isFolderExists(dir_path)) {
        LOG_F(ERROR, "Config directory not found: {}", dir_path.string());
        return false;
    }
    GET_WEAK_PTR(ComponentManager, componentManager, COMPONENT_MANAGER);
    std::shared_ptr<Component> yamlToJsonComponent;

    auto processFile = [&](const fs::path& path) {
        if (path.extension() == ".json" || path.extension() == ".lithium") {
            return loadFromFile(path);
        }
        if (path.extension() == ".json5" || path.extension() == ".lithium5") {
            std::ifstream ifs(path);
            if (!ifs || ifs.peek() == std::ifstream::traits_type::eof()) {
                LOG_F(ERROR, "Failed to open file: {}", path.string());
                return false;
            }
            std::string json5((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());
            json j = json::parse(internal::convertJSON5toJSON(json5));
            if (j.empty()) {
                LOG_F(WARNING, "Config file is empty: {}", path.string());
                return false;
            }
            mergeConfig(j);
            return true;
        }
        if (path.extension() == ".yaml") {
            if (!yamlToJsonComponent) {
                yamlToJsonComponent =
                    componentManager->getComponent("yamlToJson").value().lock();
                if (!yamlToJsonComponent) {
                    LOG_F(ERROR, "yamlToJson component not found");
                    return false;
                }
            }
            try {
                yamlToJsonComponent->dispatch("yaml_to_json", path.string());
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Failed to convert yaml to json: {}", e.what());
                GET_WEAK_PTR(PythonManager, pythonManager, PYTHON_MANAGER);
                pythonManager->loadScript("yaml_to_json.py", "yamlToJson");
                if (!atom::io::isFileExists("yaml_to_json.json")) {
                    LOG_F(ERROR, "Failed to convert yaml to json");
                    return false;
                }
            }
            return loadFromFile(path);
        }
        return true;
    };

    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.is_regular_file()) {
                if (!processFile(entry.path())) {
                    LOG_F(WARNING, "Failed to load config file: {}",
                          entry.path().string());
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
    return setValue(key_path, json(value));
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

                if (keys.size() == 1) {
                    auto folder = keys[0];
                    fs::path dirPath = m_impl_->rootPath + folder;
                    if (atom::io::removeDirectory(dirPath)) {
                        LOG_F(INFO, "Deleted folder: {}", dirPath.string());
                        return true;
                    }
                    LOG_F(ERROR, "Folder does not exist: {}", dirPath.string());
                } else if (keys.size() == 2) {
                    auto folder = keys[0];
                    auto file = keys[1];
                    fs::path filePath =
                        m_impl_->rootPath + Constants::PATH_SEPARATOR + folder +
                        Constants::PATH_SEPARATOR + file + ".json";
                    if (atom::io::removeFile(filePath)) {
                        LOG_F(INFO, "Deleted file: {}", filePath.string());
                        return true;
                    }
                    LOG_F(ERROR, "File does not exist: {}", filePath.string());
                }

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

auto ConfigManager::reload(const fs::path& path) -> bool {
    std::unique_lock lock(m_impl_->rwMutex);
    if (fs::is_directory(path)) {
        return loadFromDir(path, true);
    }
    if (fs::is_regular_file(path)) {
        return loadFromFile(path);
    }
    LOG_F(ERROR, "Invalid path to reload: {}", path.string());
    return false;
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

auto ConfigManager::saveToDir(const fs::path& dir_path) const -> bool {
    std::unique_lock lock(m_impl_->rwMutex);
    try {
        for (const auto& [folder, files] : m_impl_->config.items()) {
            fs::path folderPath = dir_path / folder;
            if (!fs::exists(folderPath)) {
                fs::create_directories(folderPath);
            }
            for (const auto& [file, content] : files.items()) {
                fs::path filePath = folderPath / (file + ".json");
                std::ofstream ofs(filePath);
                if (!ofs) {
                    LOG_F(ERROR, "Failed to open file: {}", filePath.string());
                    continue;
                }
                ofs << content.dump(4);
                LOG_F(INFO, "Config saved to file: {}", filePath.string());
            }
        }
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR,
              "Failed to save config to directory: {}, error message: {}",
              dir_path.string(), e.what());
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
            if (j.is_object()) {
                for (const auto& [key, value] : j.items()) {
                    std::string currentPath =
                        path.empty() ? key : path + "/" + key;
                    paths.push_back(currentPath);
                    listPaths(value, currentPath);
                }
            }
        };
    listPaths(m_impl_->config, "");
    return paths;
}

auto ConfigManager::listPaths() const -> std::vector<std::string> {
    std::shared_lock lock(m_impl_->rwMutex);
    std::vector<std::string> paths;
    GET_WEAK_PTR(atom::utils::Env, env, ENVIRONMENT);
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

auto ConfigManager::compareConfig(const json& src) -> std::vector<std::string> {
    std::shared_lock lock(m_impl_->rwMutex);
    return atom::utils::Differ::compare(
        atom::utils::splitString(m_impl_->config.dump(4), '\n'),
        atom::utils::splitString(src.dump(4), '\n'));
}

auto ConfigManager::dumpConfig() const -> std::string {
    std::shared_lock lock(m_impl_->rwMutex);
    // Max: 4 spaces for indentation
    return m_impl_->config.dump(4);
}
}  // namespace lithium
