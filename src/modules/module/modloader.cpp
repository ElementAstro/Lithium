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
    nlohmann::json read_config_file(const std::string &file_path)
    {
        try
        {
            // Open the configuration file
            std::ifstream file_stream(file_path);
            if (!file_stream.is_open())
            {
                LOG_F(ERROR, "Failed to open config file %s", file_path.c_str());
                return {{"error", "Failed to open config file"}};
            }

            // Read the configuration file content into a JSON object
            nlohmann::json config = nlohmann::json::parse(file_stream);

            // Close the file stream
            file_stream.close();
            return config;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to read config file %s: %s", file_path.c_str(), e.what());
            return {{"error", "Failed to read config file"}};
        }
    }

    nlohmann::json iterator_modules_dir(const std::string &dir_name)
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
                LOG_F(WARNING, "Warning: modules folder not found, creating a new one...");
                fs::create_directory(modules_dir);
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to create modules directory: %s", e.what());
            return {{"error", "Failed to create modules directory"}};
        }

        // Create a JSON object to store module information
        nlohmann::json config;

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
                        nlohmann::json module_config = read_config_file(info_file.string());
                        config[dir.path().string()]["name"] = module_config["name"];
                        config[dir.path().string()]["version"] = module_config["version"];
                        config[dir.path().string()]["author"] = module_config["author"];
                        config[dir.path().string()]["license"] = module_config.value("license", "");
                        config[dir.path().string()]["description"] = module_config.value("description", "");
                        // Debug message
                        LOG_F(INFO, "Module found: %s, config file: %s", dir.path().string().c_str(), info_file.string().c_str());
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to iterate modules directory: %s", e.what());
            return {{"error", "Failed to iterate modules directory"}};
        }

        // If no module is found, append a message field to the JSON object
        if (config.empty())
        {
            config["message"] = "No module found";
        }

        // Return the JSON object
        return config;
    }

    ModuleLoader::ModuleLoader() : m_dir_name("modules")
    {
        m_ThreadManager = std::make_shared<Thread::ThreadManager>(10);
        LOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this]()
                                       { if(!LoadOnInit()){
                                    LOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            LOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    ModuleLoader::ModuleLoader(std::shared_ptr<Thread::ThreadManager> threadManager) : m_dir_name("modules")
    {
        m_ThreadManager = threadManager;
        LOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this]()
                                       { if(!LoadOnInit()){
                                    LOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            LOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    ModuleLoader::ModuleLoader(const std::string &dir_name) : m_dir_name(dir_name)
    {
        m_ThreadManager = std::make_shared<Thread::ThreadManager>(10);
        LOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this]()
                                       { if(!LoadOnInit()){
                                    LOG_F(ERROR,"Failed to load modules on init");
                                   } },
                                       "LoadOnInit");
        }
        else
        {
            LOG_F(ERROR, "Failed to initialize thread manager in module loader");
        }
    }

    ModuleLoader::ModuleLoader(const std::string &dir_name, std::shared_ptr<Thread::ThreadManager> threadManager) : m_dir_name(dir_name), m_ThreadManager(threadManager)
    {
        LOG_F(INFO, "C++ module manager loaded successfully.");
        if (m_ThreadManager)
        {
            m_ThreadManager->addThread([this]()
                                       { if(!LoadOnInit()){
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

    bool ModuleLoader::LoadOnInit()
    {
        const nlohmann::json dir_info = iterator_modules_dir(m_dir_name);
        LOG_F(INFO, "%s", dir_info.dump(4).c_str());
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
                LOG_F(ERROR, "Library %s does not exist", path.c_str());
                return false;
            }

            // Load the library file
            void *handle = LOAD_LIBRARY(path.c_str());
            if (!handle)
            {
                LOG_F(ERROR, "Failed to load library %s: %s", path.c_str(), LOAD_ERROR());
                return false;
            }

            // Read the configuration file in JSON format
            std::filesystem::path p = path;
            std::string config_file_path = p.replace_extension(".json").string();
            if (std::filesystem::exists(config_file_path))
            {
                nlohmann::json config;
                std::ifstream config_file(config_file_path);
                config_file >> config;

                // Check if the required fields exist in the configuration file
                if (config.contains("name") && config.contains("version") && config.contains("author"))
                {
                    std::string version = config["version"].get<std::string>();
                    std::string author = config["author"].get<std::string>();
                    std::string license = config.value("license", "");

                    LOG_F(INFO, "Loaded Module : %s version %s written by %s%s",
                          config.value("name", "Unknown").c_str(), version.c_str(), author.c_str(), license.empty() ? "" : (" under " + license).c_str());
                }
                else
                {
                    LOG_F(WARNING, "Missing required fields in %s", config_file_path.c_str());
                }
            }
            else
            {
                LOG_F(WARNING, "Config file %s does not exist", config_file_path.c_str());
            }

            // Store the library handle in handles_ map with the module name as key
            handles_[name] = handle;
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to load library %s: %s", path.c_str(), e.what());
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
                throw std::runtime_error("Module " + filename + " is not loaded");
            }

            if (!it->second)
            {
                throw std::runtime_error("Module " + filename + "'s handle is null");
            }

            // Unload the library and remove its handle from handles_ map
            int result = UNLOAD_LIBRARY(it->second);
            if (result == 0)
            {
                LOG_F(INFO, "Unloaded module : %s", filename.c_str());
                handles_.erase(it);
                return true;
            }
            else
            {
                throw std::runtime_error("Failed to unload module " + filename);
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "%s", e.what());
            return false;
        }
    }

    bool ModuleLoader::CheckModuleExists(const std::string &name) const
    {
        void *handle = LOAD_LIBRARY(name.c_str());
        if (handle == nullptr)
        {
            LOG_F(ERROR, "Module %s does not exist.", name.c_str());
            return false;
        }
        LOG_F(INFO, "Module %s is existing.", name.c_str());
        UNLOAD_LIBRARY(handle);
        return true;
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
                LOG_F(ERROR, "Enabled file not found for module %s", module_name.c_str());
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
                LOG_F(ERROR, "Module path not found for module %s", module_name.c_str());
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
                LOG_F(ERROR, "Failed to disable module %s", module_name.c_str());
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
}