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

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium {

class ConfigManagerImpl {
public:
    mutable std::shared_mutex rwMutex;
    json config;
    asio::io_context ioContext_;
    std::thread ioThread_;
};

ConfigManager::ConfigManager()
    : m_impl_(std::make_unique<ConfigManagerImpl>()) {
    asio::executor_work_guard<asio::io_context::executor_type> workGuard(
        m_impl_->ioContext_.get_executor());
    m_impl_->ioThread_ = std::thread([this] { m_impl_->ioContext_.run(); });
    if (loadFromFile("config.json")) {
        DLOG_F(INFO, "Config loaded successfully.");
    }
}

ConfigManager::~ConfigManager() {
    m_impl_->ioContext_.stop();
    if (m_impl_->ioThread_.joinable()) {
        m_impl_->ioThread_.join();
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
            return false;
        }
        mergeConfig(j);
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
    try {
        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".json") {
                loadFromFile(entry.path());
            } else if (recursive && entry.is_directory()) {
                loadFromDir(entry.path(), true);
            }
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to load config file from: {}, error message: {}",
              dir_path.string(), e.what());
        return false;
    }
    return true;
}

auto ConfigManager::getValue(const std::string& key_path) const
    -> std::optional<json> {
    std::shared_lock lock(m_impl_->rwMutex);
    const json* p = &m_impl_->config;
    for (const auto& key : key_path | std::views::split('/')) {
        std::string key_str = std::string(key.begin(), key.end());
        if (p->is_object() && p->contains(key_str)) {
            p = &(*p)[key_str];
        } else {
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
                return true;
            }
            return false;
        }
        if (!p->is_object() || !p->contains(*it)) {
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
}

void ConfigManager::clearConfig() { m_impl_->config.clear(); }

void ConfigManager::asyncLoadFromFile(const fs::path& path,
                                      std::function<void(bool)> callback) {
    asio::post(m_impl_->ioContext_, [this, path, callback]() {
        bool success = loadFromFile(path);
        // Post back to caller's thread or IO context
        asio::post(m_impl_->ioContext_,
                   [callback, success]() { callback(success); });
    });
}

void ConfigManager::asyncSaveToFile(const fs::path& file_path,
                                    std::function<void(bool)> callback) const {
    asio::post(m_impl_->ioContext_, [this, file_path, callback]() {
        bool success = saveToFile(file_path);
        // Post back to caller's thread or IO context
        asio::post(m_impl_->ioContext_,
                   [callback, success]() { callback(success); });
    });
}

}  // namespace lithium
