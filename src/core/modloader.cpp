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

Description: C++ and Python Modules Loader

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

namespace OpenAPT
{
    /**
     * @brief Read a JSON configuration file and return its content as a JSON object.
     *
     * This function reads a JSON configuration file, stores its content in a JSON object, and returns the object. If it fails to read
     * the file or encounters any exception, it returns an error message as a JSON object.
     *
     * @param file_path (const std::string&) : The path of the configuration file to be read.
     * @return nlohmann::json - A JSON object containing the configuration information or an error message.
     */
    nlohmann::json read_config_file(const std::string &file_path)
    {
        try
        {
            // Open the configuration file
            std::ifstream file_stream(file_path);
            if (!file_stream.is_open())
            {
                spdlog::error("Failed to open config file {}", file_path);
                return {{"error", "Failed to open config file"}};
            }

            // Read the configuration file content into a JSON object
            nlohmann::json config;
            file_stream >> config;

            // Close the file stream
            file_stream.close();
            return config;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to read config file {}: {}", file_path, e.what());
            return {{"error", "Failed to read config file"}};
        }
    }

    /**
     * @brief Traverse the "modules" directory and create a JSON object containing the information of all modules.
     *
     * This function iterates through the "modules" directory and its subdirectories, creates a JSON object for each subdirectory that
     * contains an "info.json" configuration file, and stores the module's name, version, author, license, description, path, and configuration
     * file path in the JSON object. It returns a JSON object containing all module information, or an error message if it fails to iterate
     * the directories or encounters any exception.
     *
     * @return nlohmann::json - A JSON object containing the module information or an error message.
     */
    nlohmann::json iterator_modules_dir()
    {
        // Define the modules directory path
        fs::path modules_dir;
#ifdef _WIN32 // Windows OS
        modules_dir = fs::path(getenv("USERPROFILE")) / "Documents" / "modules";
#else // Linux OS
        modules_dir = "modules";
#endif

        try
        {
            // Create the modules directory if it does not exist
            if (!fs::exists(modules_dir) || !fs::is_directory(modules_dir))
            {
                spdlog::warn("Warning: modules folder not found, creating a new one...");
                fs::create_directory(modules_dir);
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create modules directory: {}", e.what());
            return {{"error", "Failed to create modules directory"}};
        }

        // Create a JSON object to store module information
        nlohmann::json config;

        try
        {
            // Iterate through each subdirectory of the modules directory
            for (auto &dir : fs::recursive_directory_iterator(modules_dir))
            {
                // Check if the current directory is indeed a subdirectory
                if (fs::is_directory(dir))
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
                        spdlog::debug("Module found: {}, config file: {}", dir.path().string(), info_file.string());
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to iterate modules directory: {}", e.what());
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

    ModuleLoader::ModuleLoader()
    {
        spdlog::info("C++ module manager loaded successfully.");
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

    /**
     * @brief   Loads a dynamic module from the given path.
     *
     * This function loads a dynamic module from the given path. If the loading is successful, it returns true and saves the handle to the module in the handles_ map.
     * If the loading fails, it returns false and logs an error message.
     *
     * @param[in]   path    The path of the dynamic module to load.
     * @param[in]   name    The name of the dynamic module.
     * @return      true if the loading is successful, false otherwise.
     */
    bool ModuleLoader::LoadModule(const std::string &path, const std::string &name)
    {
        try
        {
            // Check if the library file exists
            if (!std::filesystem::exists(path))
            {
                spdlog::error("Library {} does not exist", path);
                return false;
            }

            // Load the library file
            void *handle = LOAD_LIBRARY(path.c_str());
            if (!handle)
            {
                spdlog::error("Failed to load library {}: {}", path, LOAD_ERROR());
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

                    spdlog::info("Loaded Module : {} version {} written by {}{}",
                                 config.value("name", "Unknown"), version, author, license.empty() ? "" : " under " + license);
                }
                else
                {
                    spdlog::warn("Missing required fields in {}", config_file_path);
                }
            }
            else
            {
                spdlog::warn("Config file {} does not exist", config_file_path);
            }

            // Store the library handle in handles_ map with the module name as key
            handles_[name] = handle;
            return true;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to load library {}: {}", path, e.what());
            return false;
        }
    }

    /**
     * @brief 卸载指定名称的动态库
     *
     * @param filename [in] 要卸载的动态库的文件名（包括扩展名）
     * @return true 动态库卸载成功
     * @return false 动态库卸载失败
     */
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
                spdlog::info("Unloaded module : {}", filename);
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
            spdlog::error("{}", e.what());
            return false;
        }
    }

    std::shared_ptr<BasicTask> ModuleLoader::GetTaskPointer(const std::string &module_name, const nlohmann::json &config)
    {
        auto handle_it = handles_.find(module_name);
        if (handle_it == handles_.end())
        {
            spdlog::error("Failed to find module {}", module_name);
            return nullptr;
        }

        auto get_task_func = GetFunction<std::shared_ptr<BasicTask>(*)(const nlohmann::json &)>(module_name, "GetTaskInstance");
        if (!get_task_func)
        {
            spdlog::error("Failed to get symbol {} from module {}: {}", "GetTaskInstance", module_name, dlerror());
            return nullptr;
        }

        return get_task_func(config);
    }

    std::shared_ptr<Device> ModuleLoader::GetDevicePointer(const std::string &module_name, const nlohmann::json &config)
    {
        auto handle_it = handles_.find(module_name);
        if (handle_it == handles_.end())
        {
            spdlog::error("Failed to find module {}", module_name);
            return nullptr;
        }

        auto get_device_func = GetFunction<std::shared_ptr<Device>(*)(const nlohmann::json &)>(module_name, "GetDeviceInstance");
        if (!get_device_func)
        {
            spdlog::error("Failed to get symbol {} from module {}: {}", "GetDeviceInstance", module_name, dlerror());
            return nullptr;
        }

        return get_device_func(config);
    }

    bool ModuleLoader::HasModule(const std::string &name) const
    {
        return handles_.count(name) > 0;
    }
}