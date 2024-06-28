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

#include <filesystem>
#include <fstream>
#include <mutex>
#include <shared_mutex>
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {

class ConfigManagerImpl {
public:
    mutable std::shared_mutex rwMutex;
    json config;
};

ConfigManager::ConfigManager()
    : m_impl_(std::make_unique<ConfigManagerImpl>()) {
    if (loadFromFile("config.json")) {
        DLOG_F(INFO, "Config loaded successfully.");
    }
}

ConfigManager::~ConfigManager() {
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

auto ConfigManager::loadFromDir(const fs::path& dir_path, bool recursive) -> bool {
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

auto ConfigManager::getValue(const std::string& key_path) const -> std::optional<json> {
    std::shared_lock lock(m_impl_->rwMutex);
    const json* p = &m_impl_->config;
    for (const auto& key : key_path | std::views::split('/')) {
        if (p->is_object() &&
            p->contains(std::string(key.begin(), key.end()))) {
            p = &(*p)[std::string(key.begin(), key.end())];
        } else {
            return std::nullopt;
        }
    }
    return *p;
}

bool ConfigManager::setValue(const std::string& key_path, const json& value) {
    std::unique_lock lock(m_impl_->rwMutex);
    json* p = &m_impl_->config;
    std::vector<std::string_view> keys;
    for (auto subRange : key_path | std::views::split('/')) {
        keys.emplace_back(subRange.data(), subRange.size());
    }

    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (it + 1 == keys.end()) {
            (*p)[*it] = value;
            return true;
        }

        if (!p->is_object()) {
            return false;
        }

        if (!p->contains(*it)) {
            (*p)[*it] = json::object();
        }

        p = &(*p)[*it];
    }

    return true;
}

auto ConfigManager::deleteValue(const std::string& key_path) -> bool {
    std::unique_lock lock(m_impl_->rwMutex);
    std::vector<std::string_view> keys;
    for (auto subRange : key_path | std::views::split('/')) {
        keys.emplace_back(subRange.data(), subRange.size());
    }
    json* p = &m_impl_->config;
    for (auto it = keys.begin(); it != keys.end(); ++it) {
        if (it + 1 == keys.end()) {  // Last key
            if (p->contains(*it)) {
                p->erase(*it);
                return true;
            }
            return false;
        }
        if (!p->contains(*it) || !(*p)[*it].is_object()) {
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
        for (auto subKey : key | std::views::split('/')) {
            if (!p->contains(std::string(subKey.begin(), subKey.end()))) {
                (*p)[std::string(subKey.begin(), subKey.end())] =
                    json::object();
            }
            p = &(*p)[std::string(subKey.begin(), subKey.end())];
        }
        *p = value;
    }
    m_impl_->config = std::move(updatedConfig);
}

void ConfigManager::mergeConfig(const json& src) {
    m_impl_->config.merge_patch(src);
}

void ConfigManager::clearConfig() { m_impl_->config.clear(); }
}  // namespace lithium
