/*
 * module_loader.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: C++ and Modules Loader

**************************************************/

#include "module_loader.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <typeinfo>
#include <cxxabi.h>
#include <regex>

#include "thread/thread.hpp"

namespace fs = std::filesystem;

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#define SET_CONFIG_VALUE(key) \
    config[dir.path().string()][#key] = module_config.value(#key, "");

namespace Atom::Module
{
    /**
     * @brief Read a JSON configuration file and return its content as a JSON object.
     *
     * This function reads a JSON configuration file, stores its content in a JSON object, and returns the object. If it fails to read
     * the file or encounters any exception, it returns an error message as a JSON object.
     *
     * 读取JSON配置文件，将其内容存储在一个JSON对象中，并返回该对象。如果无法读取文件或遇到任何异常，则返回错误消息作为JSON对象。
     *
     * @param file_path (const std::string&) : The path of the configuration file to be read.
     *                                         要读取的配置文件的路径。
     * @return json - A JSON object containing the configuration information or an error message.
     *                包含配置信息或错误消息的JSON对象。
     */
    json read_config_file(const std::string &file_path)
    {
        try
        {
            // Open the configuration file
            std::ifstream file_stream(file_path);
            if (!file_stream.is_open())
            {
                LOG_F(ERROR, "Failed to open config file {}", file_path);
                return {{"error", "Failed to open config file"}};
            }

            // Read the configuration file content into a JSON object
            json config = json::parse(file_stream);

            // Close the file stream
            file_stream.close();
            return config;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to read config file {}: {}", file_path, e.what());
            return {{"error", "Failed to read config file"}};
        }
    }

    json iterator_modules_dir(const std::string &dir_name)
    {
        if (dir_name == "")
        {
            LOG_F(ERROR, "DIR name should not be null");
            return {{"error", "dir name should not be null"}};
        }
        // Define the modules directory path
        fs::path modules_dir;
        modules_dir = fs::absolute(std::filesystem::current_path() / "modules" / dir_name);
        try
        {
            // Create the modules directory if it does not exist
            if (!fs::exists(modules_dir) || !fs::is_directory(modules_dir))
            {
                DLOG_F(WARNING, "Warning: modules folder not found, creating a new one...");
                fs::create_directory(modules_dir);
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to create modules directory: {}", e.what());
            return {{"error", "Failed to create modules directory"}};
        }

        // Create a JSON object to store module information
        json config;

        try
        {
            // Iterate through each subdirectory of the modules directory
            for (const auto &dir : fs::recursive_directory_iterator(modules_dir))
            {
                // Check if the current directory is indeed a subdirectory
                if (dir.is_directory())
                {
                    // Get the path of the info.json file within the subdirectory
                    fs::path info_file = dir.path() / "config.json";
                    // If the info.json exists
                    if (fs::exists(info_file))
                    {
                        // Append necessary information to the JSON object
                        config[dir.path().string()]["path"] = dir.path().string();
                        config[dir.path().string()]["config"] = info_file.string();
                        DLOG_F(INFO, "Module found: {}, config file: {}", dir.path().string(), info_file.string());
                        // Read the module configuration from the info.json file and append to the JSON object
                        json module_config = read_config_file(info_file.string());
                        SET_CONFIG_VALUE(name)
                        SET_CONFIG_VALUE(version)
                        SET_CONFIG_VALUE(author)
                        SET_CONFIG_VALUE(type)
                        SET_CONFIG_VALUE(dependencies)
                        SET_CONFIG_VALUE(url)
                        SET_CONFIG_VALUE(homepage)
                        SET_CONFIG_VALUE(keywords)
                        SET_CONFIG_VALUE(repository)
                        SET_CONFIG_VALUE(bugs)
                        SET_CONFIG_VALUE(readme)
                        SET_CONFIG_VALUE(license)
                        SET_CONFIG_VALUE(description)
                    }
                }
            }
        }
        catch (const fs::filesystem_error &e)
        {
            LOG_F(ERROR, "Failed to iterate modules directory: {}", e.what());
            return {{"error", "Failed to iterate modules directory"}};
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to iterate modules directory: {}", e.what());
            return {{"error", "Failed to iterate modules directory"}};
        }
        if (config.empty())
        {
            config["message"] = "No module found";
        }
        return config;
    }

    ModuleLoader::ModuleLoader(const std::string &dir_name = "modules", std::shared_ptr<Thread::ThreadManager> threadManager = Thread::ThreadManager::createShared())
    {
        m_ThreadManager = threadManager;
        DLOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this, dir_name]()
                                       { if(!LoadOnInit(dir_name)){
                                    LOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            LOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    std::shared_ptr<ModuleLoader> ModuleLoader::createShared()
    {
        return std::make_shared<ModuleLoader>("modules", Thread::ThreadManager::createShared());
    }

    std::shared_ptr<ModuleLoader> ModuleLoader::createShared(const std::string &dir_name = "modules", std::shared_ptr<Thread::ThreadManager> threadManager = Thread::ThreadManager::createShared())
    {
        return std::make_shared<ModuleLoader>(dir_name, threadManager);
    }

    ModuleLoader::~ModuleLoader()
    {
        if (!modules_.empty())
        {
            if (!UnloadAllModules())
            {
                LOG_F(ERROR, "Failed to unload all modules");
            }
            modules_.clear();
        }
    }

    bool ModuleLoader::LoadOnInit(const std::string &dir_name)
    {
        if (dir_name.empty())
        {
            LOG_F(ERROR, "Directory name is empty");
            return false;
        }
        const json dir_info = iterator_modules_dir(dir_name);
        DLOG_F(INFO, "{}", dir_info.dump(4));
        if (!dir_info.empty())
        {
            if (dir_info["message"].get<std::string>() != "No module found")
            {
                for (auto module_ : dir_info)
                {
                    const std::string name = module_.value("name", "");
                    const std::string path = module_.value("path", "");
                    if (name == "" || path == "")
                        continue;
                    if (!LoadModule(path, name))
                        continue;
                }
            }
        }
        return true;
    }

    bool ModuleLoader::LoadModule(const std::string &path, const std::string &name)
    {
        try
        {
            // Check if the library file exists
            if (!std::filesystem::exists(path))
            {
                LOG_F(ERROR, "Library {} does not exist", path);
                return false;
            }
            // Max : The mod's name should be unique, so we check if it already exists
            if (HasModule(name))
            {
                LOG_F(ERROR, "Module {} already loaded", name);
                return false;
            }

            std::shared_ptr<Mod> mod = std::make_shared<Mod>();
            // Load the library file
            void *handle = LOAD_LIBRARY(path.c_str());
            if (!handle)
            {
                LOG_F(ERROR, "Failed to load library {}: {}", path, LOAD_ERROR());
                return false;
            }
            mod->handle = handle;
            // Read the configuration file in JSON format. We will support other formats in the future
            std::filesystem::path p = path;
            std::string config_file_path = p.replace_extension(".json").string();
            if (std::filesystem::exists(config_file_path))
            {
                json config;
                try
                {
                    std::ifstream config_file(config_file_path);
                    config_file >> config;
                }
                catch (const json::parse_error &e)
                {
                    LOG_F(ERROR, "Failed to parse config file {}: {}", config_file_path, e.what());
                }
                mod->config_path = config_file_path;
                mod->config_file = p.string();
                mod->config = config;
                // Check if the required fields exist in the configuration file
                if (config.contains("name") && config.contains("version") && config.contains("author") && config.contains("type"))
                {
                    std::string version = config["version"].get<std::string>();
                    std::string author = config["author"].get<std::string>();
                    std::string license = config.value("license", "");
                    std::string type = config["type"].get<std::string>();

                    mod->version = version;
                    mod->author = author;
                    mod->license = license;
                    mod->type = type;

                    DLOG_F(INFO, "Loaded Module : {} version {} written by {}{}",
                           config.value("name", "Unknown"), version, author, license.empty() ? "" : (" under " + license));
                }
                else
                {
                    DLOG_F(WARNING, "Missing required fields in {}", config_file_path);
                }
            }
            else
            {
                DLOG_F(WARNING, "Config file {} does not exist", config_file_path);
            }

            // Store the library handle in handles_ map with the module name as key
            // handles_[name] = handle;
            modules_[name] = mod;
            DLOG_F(INFO, "Loaded module : {}", name);
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to load library {}: {}", path, e.what());
            return false;
        }
    }

    bool ModuleLoader::UnloadModule(const std::string &name)
    {
        try
        {
            // Check if the module is loaded and has a valid handle
            if (!HasModule(name))
            {
                LOG_F(ERROR, "Module {} is not loaded", name);
                return false;
            };
            // Unload the library and remove its handle from handles_ map
            int result = UNLOAD_LIBRARY(GetModule(name)->handle);
            if (result != 0)
            {
                LOG_F(ERROR, "Failed to unload module {}", name);
                return false;
            }
            modules_.erase(name);
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "{}", e.what());
            return false;
        }
    }

    bool ModuleLoader::UnloadAllModules()
    {
        for (auto entry : modules_)
        {
            int result = UNLOAD_LIBRARY(entry.second->handle);
            if (result != 0)
            {
                LOG_F(ERROR, "Failed to unload module {}", entry.first);
                return false;
            }
        }
        modules_.clear();
        return true;
    }

    bool ModuleLoader::CheckModuleExists(const std::string &name) const
    {
        // Max : Directly check if the library exists seems to be a litle bit slow. May we use filesystem instead?
        void *handle = LOAD_LIBRARY(name.c_str());
        if (handle == nullptr)
        {
            LOG_F(ERROR, "Module {} does not exist.", name);
            return false;
        }
        DLOG_F(INFO, "Module {} is existing.", name);
        UNLOAD_LIBRARY(handle);
        return true;
    }

    std::shared_ptr<Mod> ModuleLoader::GetModule(const std::string &name) const
    {
        auto it = modules_.find(name);
        if (it == modules_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    void *ModuleLoader::GetHandle(const std::string &name) const
    {
        auto it = modules_.find(name);
        if (it == modules_.end())
        {
            return nullptr;
        }
        return it->second->handle;
    }

    bool ModuleLoader::HasModule(const std::string &name) const
    {
        return modules_.count(name) > 0;
    }

    bool ModuleLoader::EnableModule(const std::string &name)
    {
        // Check if the module is loaded
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Module {} is not loaded", name);
            return false;
        }
        std::shared_ptr<Mod> mod = GetModule(name);
        if (!mod->enabled.load())
        {
            mod->enabled.store(true);
            std::string disabled_file = mod->path;
            std::string enabled_file = disabled_file.substr(0, disabled_file.size() - 8);
            if (CheckModuleExists(enabled_file))
            {
                if (UnloadModule(enabled_file))
                {
                    std::rename(disabled_file.c_str(), enabled_file.c_str());
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                LOG_F(ERROR, "Enabled file not found for module {}", name);
                return false;
            }
        }
        return true;
    }

    bool ModuleLoader::DisableModule(const std::string &name)
    {
        // Check if the module is loaded
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Module {} is not loaded", name);
            return false;
        }
        std::shared_ptr<Mod> mod = GetModule(name);
        if (mod->enabled.load())
        {
            mod->enabled.store(false);
            std::string module_path = GetModulePath(name);
            if (module_path.empty())
            {
                LOG_F(ERROR, "Module path not found for module {}", name);
                return false;
            }
            std::string disabled_file = module_path + ".disabled";
            if (std::rename(module_path.c_str(), disabled_file.c_str()) == 0)
            {
                modules_.erase(name);
                return true;
            }
            else
            {
                LOG_F(ERROR, "Failed to disable module {}", name);
                return false;
            }
        }
        return true;
    }

    bool ModuleLoader::IsModuleEnabled(const std::string &name) const
    {
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Module {} is not loaded", name);
            return false;
        }
        if (GetModule(name)->enabled.load())
        {
            return true;
        }
        return false;
    }

    std::string ModuleLoader::GetModuleVersion(const std::string &name)
    {
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetVersion")();
        }
        return "";
    }

    std::string ModuleLoader::GetModuleDescription(const std::string &name)
    {
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetDescription")();
        }
        return "";
    }

    std::string ModuleLoader::GetModuleAuthor(const std::string &name)
    {
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetAuthor")();
        }
        return "";
    }

    std::string ModuleLoader::GetModuleLicense(const std::string &name)
    {
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetLicense")();
        }
        return "";
    }

    std::string ModuleLoader::GetModulePath(const std::string &name)
    {
        auto it = modules_.find(name);
        if (it != modules_.end())
        {
            Dl_info dl_info;
            if (dladdr(it->second->handle, &dl_info) != 0)
            {
                return dl_info.dli_fname;
            }
        }
        return "";
    }

    json ModuleLoader::GetModuleConfig(const std::string &name)
    {
        if (HasModule(name))
        {
            return GetFunction<json (*)()>(name, "GetConfig")();
        }
        return {};
    }

    const std::vector<std::string> ModuleLoader::GetAllExistedModules() const
    {
        std::vector<std::string> modules_name;
        if (modules_.empty())
        {
            return modules_name;
        }
        for (auto module_ : modules_)
        {
            modules_name.push_back(module_.first);
        }
        return modules_name;
    }
}