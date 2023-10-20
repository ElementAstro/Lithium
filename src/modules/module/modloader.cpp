/*
 * modloader.cpp
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

#include "modloader.hpp"

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

namespace Lithium
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
                DLOG_F(ERROR, "Failed to open config file {}", file_path);
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
            DLOG_F(ERROR, "Failed to read config file {}: {}", file_path, e.what());
            return {{"error", "Failed to read config file"}};
        }
    }

    json iterator_modules_dir(const std::string &dir_name)
    {
        if (dir_name == "")
        {
            DLOG_F(ERROR, "DIR name should not be null");
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
            DLOG_F(ERROR, "Failed to create modules directory: {}", e.what());
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
                    fs::path info_file = dir.path() / "info.json";
                    // If the info.json exists
                    if (fs::exists(info_file))
                    {
                        // Append necessary information to the JSON object
                        config[dir.path().string()]["path"] = dir.path().string();
                        config[dir.path().string()]["config"] = info_file.string();
                        // Read the module configuration from the info.json file and append to the JSON object
                        json module_config = read_config_file(info_file.string());
                        config[dir.path().string()]["name"] = module_config["name"];
                        config[dir.path().string()]["version"] = module_config["version"];
                        config[dir.path().string()]["author"] = module_config["author"];
                        config[dir.path().string()]["license"] = module_config.value("license", "");
                        config[dir.path().string()]["description"] = module_config.value("description", "");
                        // Debug message
                        DLOG_F(INFO, "Module found: {}, config file: {}", dir.path().string(), info_file.string());
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Failed to iterate modules directory: {}", e.what());
            return {{"error", "Failed to iterate modules directory"}};
        }
        if (config.empty())
        {
            config["message"] = "No module found";
        }
        return config;
    }

    ModuleLoader::ModuleLoader()
    {
        m_ThreadManager = std::make_shared<Thread::ThreadManager>(10);
        DLOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this]()
                                       { if(!LoadOnInit("modules")){
                                    DLOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            DLOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    ModuleLoader::ModuleLoader(std::shared_ptr<Thread::ThreadManager> threadManager)
    {
        m_ThreadManager = threadManager;
        DLOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this]()
                                       { if(!LoadOnInit("modules")){
                                    DLOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            DLOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    ModuleLoader::ModuleLoader(const std::string &dir_name)
    {
        m_ThreadManager = std::make_shared<Thread::ThreadManager>(10);
        DLOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this, dir_name]()
                                       { if(!LoadOnInit(dir_name)){
                                    DLOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            DLOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    ModuleLoader::ModuleLoader(const std::string &dir_name, std::shared_ptr<Thread::ThreadManager> threadManager)
    {
        m_ThreadManager = threadManager;
        DLOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this, dir_name]()
                                       { if(!LoadOnInit(dir_name)){
                                    DLOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            DLOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    std::shared_ptr<ModuleLoader> ModuleLoader::createShared()
    {
        return std::make_shared<ModuleLoader>();
    }

    std::shared_ptr<ModuleLoader> ModuleLoader::createShared(const std::string &dir_name)
    {
        return std::make_shared<ModuleLoader>(dir_name);
    }

    std::shared_ptr<ModuleLoader> ModuleLoader::createShared(std::shared_ptr<Thread::ThreadManager> threadManager)
    {
        return std::make_shared<ModuleLoader>(threadManager);
    }
    std::shared_ptr<ModuleLoader> ModuleLoader::createShared(const std::string &dir_name, std::shared_ptr<Thread::ThreadManager> threadManager)
    {
        return std::make_shared<ModuleLoader>(dir_name, threadManager);
    }

    ModuleLoader::~ModuleLoader()
    {
        if (!handles_.empty())
        {
            for (auto &entry : handles_)
            {
                UNLOAD_LIBRARY(entry.second);
            }
        }
    }

    bool ModuleLoader::LoadOnInit(const std::string &dir_name)
    {
        if (dir_name.empty())
        {
            DLOG_F(ERROR, "Directory name is empty");
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
                DLOG_F(ERROR, "Library {} does not exist", path);
                return false;
            }

            // Load the library file
            void *handle = LOAD_LIBRARY(path.c_str());
            if (!handle)
            {
                DLOG_F(ERROR, "Failed to load library {}: {}", path, LOAD_ERROR());
                return false;
            }

            // Read the configuration file in JSON format
            std::filesystem::path p = path;
            std::string config_file_path = p.replace_extension(".json").string();
            if (std::filesystem::exists(config_file_path))
            {
                json config;
                std::ifstream config_file(config_file_path);
                config_file >> config;

                // Check if the required fields exist in the configuration file
                if (config.contains("name") && config.contains("version") && config.contains("author"))
                {
                    std::string version = config["version"].get<std::string>();
                    std::string author = config["author"].get<std::string>();
                    std::string license = config.value("license", "");

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
            handles_[name] = handle;
            return true;
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Failed to load library {}: {}", path, e.what());
            return false;
        }
    }

    bool ModuleLoader::UnloadModule(const std::string &filename)
    {
        try
        {
            // Check if the module is loaded and has a valid handle
            auto it = handles_.find(filename);
            if (it == handles_.end())
            {
                DLOG_F(ERROR, "Module {} is not loaded", filename);
                return false;
            }

            if (!it->second)
            {
                DLOG_F(ERROR, "Module {}'s handle is null", filename);
                return false;
            }

            // Unload the library and remove its handle from handles_ map
            int result = UNLOAD_LIBRARY(it->second);
            if (result == 0)
            {
                DLOG_F(INFO, "Unloaded module : {}", filename);
                handles_.erase(it);
                return true;
            }
            else
            {
                DLOG_F(ERROR, "Failed to unload module {}", filename);
                return false;
            }
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "{}", e.what());
            return false;
        }
    }

    bool ModuleLoader::CheckModuleExists(const std::string &name) const
    {
        void *handle = LOAD_LIBRARY(name.c_str());
        if (handle == nullptr)
        {
            DLOG_F(ERROR, "Module {} does not exist.", name);
            return false;
        }
        DLOG_F(INFO, "Module {} is existing.", name);
        UNLOAD_LIBRARY(handle);
        return true;
    }

    void *ModuleLoader::GetHandle(const std::string &name) const
    {
        auto it = handles_.find(name);
        if (it == handles_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    bool ModuleLoader::HasModule(const std::string &name) const
    {
        return handles_.count(name) > 0;
    }

    bool ModuleLoader::EnableModule(const std::string &module_name)
    {
        auto it = disabled_modules_.find(module_name);
        if (it != disabled_modules_.end())
        {
            std::string disabled_file = it->second;
            std::string enabled_file = disabled_file.substr(0, disabled_file.size() - 8);
            if (CheckModuleExists(enabled_file))
            {
                if (UnloadModule(enabled_file))
                {
                    std::rename(disabled_file.c_str(), enabled_file.c_str());
                    disabled_modules_.erase(it);
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                DLOG_F(ERROR, "Enabled file not found for module {}", module_name);
                return false;
            }
        }
        return true;
    }

    bool ModuleLoader::DisableModule(const std::string &module_name)
    {
        auto it = handles_.find(module_name);
        if (it != handles_.end())
        {
            void *handle = it->second;
            std::string module_path = GetModulePath(module_name);
            if (module_path.empty())
            {
                DLOG_F(ERROR, "Module path not found for module {}", module_name);
                return false;
            }
            std::string disabled_file = module_path + ".disabled";
            if (std::rename(module_path.c_str(), disabled_file.c_str()) == 0)
            {
                handles_.erase(it);
                disabled_modules_.insert(std::make_pair(module_name, disabled_file));
                return true;
            }
            else
            {
                DLOG_F(ERROR, "Failed to disable module {}", module_name);
                return false;
            }
        }
        return true;
    }

    std::string ModuleLoader::GetModulePath(const std::string &module_name)
    {
        auto it = handles_.find(module_name);
        if (it != handles_.end())
        {
            Dl_info dl_info;
            if (dladdr(it->second, &dl_info) != 0)
            {
                return dl_info.dli_fname;
            }
        }
        return "";
    }

    const std::vector<std::string> ModuleLoader::GetAllExistedModules() const
    {
        std::vector<std::string> modules_name;
        if (handles_.empty())
        {
            return modules_name;
        }
        for (auto module_ : handles_)
        {
            modules_name.push_back(module_.first);
        }
        return modules_name;
    }
}