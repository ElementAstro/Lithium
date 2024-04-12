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
#include <regex>
#include <sstream>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include "atom/utils/string.hpp"

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

namespace Lithium {
class ConfigManagerImpl {
public:
    mutable std::shared_mutex rw_mutex_;
    json config_;
};

ConfigManager::ConfigManager() : m_impl(std::make_unique<ConfigManagerImpl>()) {
    if (loadFromFile("config.json")) {
        DLOG_F(INFO, "Config loaded successfully.");
    }
}

ConfigManager::~ConfigManager() { saveToFile("config.json"); }

std::shared_ptr<ConfigManager> ConfigManager::createShared() {
    static std::shared_ptr<ConfigManager> instance =
        std::make_shared<ConfigManager>();
    return instance;
}

std::unique_ptr<ConfigManager> ConfigManager::createUnique() {
    return std::make_unique<ConfigManager>();
}

bool ConfigManager::loadFromFile(const fs::path& path) {
    std::shared_lock lock(m_impl->rw_mutex_);
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
        LOG_F(ERROR, "Failed to parse file: {}, error message: {}", path.string(),
              e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to load config file: {}, error message: {}", path.string(),
              e.what());
    }
    return false;
}

bool ConfigManager::loadFromDir(const fs::path& dir_path, bool recursive) {
    std::shared_lock lock(m_impl->rw_mutex_);
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

std::optional<json> ConfigManager::getValue(const std::string& key_path) const {
    std::shared_lock lock(m_impl->rw_mutex_);
    const json* p = &m_impl->config_;
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
    std::unique_lock lock(m_impl->rw_mutex_);
    json* p = &m_impl->config_;
    std::vector<std::string_view> keys;
    for (auto sub_range : key_path | std::views::split('/')) {
        keys.emplace_back(sub_range.data(), sub_range.size());
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

bool ConfigManager::deleteValue(const std::string& key_path) {
    std::unique_lock lock(m_impl->rw_mutex_);
    std::vector<std::string_view> keys;
    for (auto sub_range : key_path | std::views::split('/')) {
        keys.emplace_back(sub_range.data(), sub_range.size());
    }
    json* p = &m_impl->config_;
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

bool ConfigManager::hasValue(const std::string& key_path) const {
    return getValue(key_path).has_value();
}

bool ConfigManager::saveToFile(const fs::path& file_path) const {
    std::unique_lock lock(m_impl->rw_mutex_);
    std::ofstream ofs(file_path);
    if (!ofs) {
        LOG_F(ERROR, "Failed to open file: {}", file_path.string());
        return false;
    }
    try {
        ofs << m_impl->config_.dump(4);
        ofs.close();
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to save config to file: {}, error message: {}",
              file_path.string(), e.what());
        return false;
    }
}

void ConfigManager::tidyConfig() {
    std::unique_lock lock(m_impl->rw_mutex_);
    json updated_config;
    for (const auto& [key, value] : m_impl->config_.items()) {
        json* p = &updated_config;
        for (auto sub_key : key | std::views::split('/')) {
            if (!p->contains(std::string(sub_key.begin(), sub_key.end()))) {
                (*p)[std::string(sub_key.begin(), sub_key.end())] =
                    json::object();
            }
            p = &(*p)[std::string(sub_key.begin(), sub_key.end())];
        }
        *p = value;
    }
    m_impl->config_ = std::move(updated_config);
}

void ConfigManager::mergeConfig(const json& src) {
    m_impl->config_.merge_patch(src);
}

void ConfigManager::clearConfig() { m_impl->config_.clear(); }

}  // namespace Lithium
