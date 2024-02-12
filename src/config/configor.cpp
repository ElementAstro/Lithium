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
#include <filesystem>
#include <sstream>
#include <regex>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

namespace Lithium
{
    ConfigManager::ConfigManager()
    {
        if (loadFromFile("config.json"))
        {
            DLOG_F(INFO, "current config: {}", config_.dump(4));
        }
    }

    ConfigManager::~ConfigManager()
    {
        saveToFile("config.json");
    }

    std::shared_ptr<ConfigManager> ConfigManager::createShared()
    {
        return std::make_shared<ConfigManager>();
    }

    bool ConfigManager::loadFromFile(const std::string &path)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        try
        {
            std::ifstream ifs(path);
            if (!ifs.is_open())
            {
                LOG_F(ERROR, "Failed to open file: {}", path);
                return false;
            }
            json j = json::parse(ifs);
            const std::string basename = fs::path(path).stem().string();
            config_[basename] = j["config"];
            DLOG_F(INFO, "Loaded config file {} successfully", path);
            return true;
        }
        catch (const json::exception &e)
        {
            LOG_F(ERROR, "Failed to parse file: {}, error message: {}", path, e.what());
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to load config file: {}, error message: {}", path, e.what());
        }
        return false;
    }

    bool ConfigManager::loadFromDir(const std::string &dir_path, bool recursive)
    {
        std::lock_guard<std::shared_mutex> lock(rw_mutex_);
        for (const auto &entry : fs::directory_iterator(dir_path))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".json")
            {
                loadFromFile(entry.path().string());
            }
            else if (recursive && entry.is_directory())
            {
                const std::string subdir_path = entry.path().string();
                const std::string config_file_path = subdir_path + "/config.json";
                if (fs::exists(config_file_path))
                {
                    try
                    {
                        json j = json::parse(std::ifstream(config_file_path));
                        auto merged_j = json::object();
                        merged_j[dir_path][entry.path().filename().string()] = j;
                        mergeConfig(merged_j);
                    }
                    catch (const json::exception &e)
                    {
                        LOG_F(ERROR, "Failed to parse file: {}, error message: {}", config_file_path, e.what());
                    }
                    catch (const std::exception &e)
                    {
                        LOG_F(ERROR, "Failed to open file {}", config_file_path);
                    }
                }
                loadFromDir(subdir_path, true);
            }
        }
        return true;
    }

    json ConfigManager::getValue(const std::string &key_path) const
    {
        // std::lock_guard<std::shared_mutex> lock(rw_mutex_);
        try
        {
            const json *p = &config_;
            for (const auto &key : split(key_path, "/"))
            {
                if (p->is_object() && p->contains(key))
                {
                    p = &(*p)[key];
                }
                else
                {
                    LOG_F(ERROR, "Key not found: {}", key_path);
                    return nullptr;
                }
            }
            return *p;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to get value: {} {}", key_path, e.what());
            return nullptr;
        }
    }

    bool ConfigManager::setValue(const std::string &key_path, const json &value)
    {
        // std::lock_guard<std::shared_mutex> lock(rw_mutex_);
        try
        {
            json *p = &config_;
            for (const auto &key : split(key_path, "/"))
            {
                if (!p->is_object())
                {
                    LOG_F(ERROR, "Invalid key path: {}", key_path);
                    return false;
                }
                p = &(*p)[key];
            }
            *p = value;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to set key: {} with {}", key_path, e.what());
            return false;
        }
        return true;
    }

    bool ConfigManager::deleteValue(const std::string &key_path)
    {
        std::lock_guard<std::shared_mutex> lock(rw_mutex_);
        std::vector<std::string> keys = split(key_path, "/");
        json *p = &config_;
        for (const auto &key : keys)
        {
            if (!p->is_object())
            {
                return false;
            }
            auto it = p->find(key);
            if (it == p->end())
            {
                return false;
            }
            p = &(*p)[key];
        }
        p->clear();
        return true;
    }

    bool ConfigManager::hasValue(const std::string &key_path) const
    {
        return getValue(key_path) != nullptr;
    }

    void ConfigManager::tidyConfig()
    {
        std::lock_guard<std::mutex> lock(mutex_);

        json updated_config;

        for (const auto &[key, value] : config_.items())
        {
            std::vector<std::string> keys = split(key, "/");
            json *p = &updated_config;

            for (const auto &sub_key : keys)
            {
                if (!p->contains(sub_key))
                {
                    (*p)[sub_key] = json::object();
                }

                p = &(*p)[sub_key];
            }

            *p = value;
        }

        config_ = std::move(updated_config);
    }

    void ConfigManager::mergeConfig(const json &j)
    {
        config_.merge_patch(j);
    }

    bool ConfigManager::saveToFile(const std::string &file_path) const
    {
        std::ofstream ofs(file_path);
        if (!ofs.is_open())
        {
            LOG_F(ERROR, "Failed to open file: {}", file_path);
            return false;
        }
        try
        {
            ofs << config_.dump(4);
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to save config to file: {}, error message: {}", file_path, e.what());
            ofs.close();
            return false;
        }
        ofs.close();
        DLOG_F(INFO, "Save config to file: {}", file_path);
        return true;
    }

    std::vector<std::string> ConfigManager::split(const std::string &s, const std::string &delimiter) const
    {
        std::vector<std::string> tokens;
        std::regex regex(delimiter);
        std::copy(std::sregex_token_iterator(s.begin(), s.end(), regex, -1),
                  std::sregex_token_iterator(),
                  std::back_inserter(tokens));
        return tokens;
    }

}
