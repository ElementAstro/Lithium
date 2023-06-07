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

#include <spdlog/spdlog.h>

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

    ModuleLoader::ModuleLoader(MyApp *app) : m_App(app)
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
            std::string config_file_path = std::filesystem::path(path).replace_extension(".json");
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

    bool ModuleLoader::LoadBinary(const char *dir_path, const char *out_path, const char *build_path, const char *lib_name)
    {
        DIR *dir;
        struct dirent *ent;
        struct stat file_stat;
        char cmake_path[512];
        char lib_path[512];
        bool ret = true;

        // Open the directory and check for errors
        dir = opendir(dir_path);
        if (!dir)
        {
            spdlog::error("Failed to open directory {}: {}", dir_path, strerror(errno));
            return false;
        }

        // Look for the CMakeLists.txt file
        while ((ent = readdir(dir)) != nullptr)
        {
            if (strcmp(ent->d_name, "CMakeLists.txt") == 0)
            {
                snprintf(cmake_path, sizeof(cmake_path), "%s%s%s", dir_path, PATH_SEPARATOR, ent->d_name);
                break;
            }
        }

        closedir(dir);

        // If CMakeLists.txt is not found, return error
        if (!ent)
        {
            spdlog::error("Could not find CMakeLists.txt in directory {}", dir_path);
            return false;
        }

        // Create the build directory and check for errors
        if (mkdir(build_path, 0777) == -1 && errno != EEXIST)
        {
            spdlog::error("Failed to create build directory: {}", strerror(errno));
            return false;
        }

        // Change the working directory to build and check for errors
        if (chdir(build_path) == -1)
        {
            spdlog::error("Failed to change working directory to {}: {}", build_path, strerror(errno));
            return false;
        }

        // Check if the dynamic library already exists, and copy it if it does
        snprintf(lib_path, sizeof(lib_path), "%s%slib%s.so", build_path, PATH_SEPARATOR, lib_name);
        if (access(lib_path, F_OK) == 0)
        {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "cp -av %s %s%s", lib_path, out_path, PATH_SEPARATOR);
            if (system(cmd) != 0)
            {
                spdlog::error("Failed to copy dynamic library");
                ret = false;
            }

            // Remove the build directory
            if (chdir(dir_path) == -1)
            {
                spdlog::error("Failed to change working directory to {}: {}", dir_path, strerror(errno));
                return false;
            }

            char remove_cmd[512];
#ifndef _WIN32
            snprintf(remove_cmd, sizeof(remove_cmd), "rm -rf %s", build_path);
#else
            snprintf(remove_cmd, sizeof(remove_cmd), "rmdir /s /q %s", build_path);
#endif
            if (system(remove_cmd) != 0)
            {
                spdlog::error("Failed to remove build directory");
            }
            return true;
        }

        try
        {
            // Execute CMake to generate the build files
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "cmake -DCMAKE_BUILD_TYPE=Release -D LIBRARY_NAME=%s ..", lib_name);
            if (system(cmd) != 0)
            {
                spdlog::error("Failed to run cmake");
                return false;
            }

            // Execute Make to build the dynamic library
#ifdef _WIN32
            snprintf(cmd, sizeof(cmd), "nmake");
#else
            snprintf(cmd, sizeof(cmd), "make");
#endif
            if (system(cmd) != 0)
            {
                spdlog::error("Failed to run make");
                return false;
            }

            // Copy the generated dynamic library to the output directory
            snprintf(cmd, sizeof(cmd), "cp -av lib%s.so %s%s", lib_name, out_path, PATH_SEPARATOR);
            if (system(cmd) != 0)
            {
                spdlog::error("Failed to copy dynamic library");
                ret = false;
            }

            // Remove the build directory and check for errors
            if (chdir(dir_path) == -1)
            {
                spdlog::error("Failed to change working directory to {}: {}", dir_path, strerror(errno));
                return false;
            }

            char remove_cmd[512];
#ifndef _WIN32
            snprintf(remove_cmd, sizeof(remove_cmd), "rm -rf %s", build_path);
#else
            snprintf(remove_cmd, sizeof(remove_cmd), "rmdir /s /q %s", build_path);
#endif
            if (system(remove_cmd) != 0)
            {
                spdlog::error("Failed to remove build directory");
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("{}", e.what());
            ret = false;
        }

        // Return the final result
        return ret;
    }

    template <typename T>
    T ModuleLoader::GetFunction(const std::string &module_name, const std::string &function_name)
    {
        auto handle_it = handles_.find(module_name);
        if (handle_it == handles_.end())
        {
            spdlog::error("Failed to find module {}", module_name);
            return nullptr;
        }

#ifdef _WIN32
        auto func_ptr = reinterpret_cast<T>(GetProcAddress(handle_it->second, function_name.c_str()));
#else
        auto func_ptr = reinterpret_cast<T>(dlsym(handle_it->second, function_name.c_str()));
#endif

        if (!func_ptr)
        {
            spdlog::error("Failed to get symbol {} from module {}: {}", function_name, module_name, dlerror());
            return nullptr;
        }

        return func_ptr;
    }

    bool ModuleLoader::HasModule(const std::string &name) const
    {
        return handles_.count(name) > 0;
    }

    nlohmann::json ModuleLoader::getArgsDesc(void *handle, const std::string &functionName)
    {
        nlohmann::json result = nlohmann::json::array();

        if (!handle)
        {
            spdlog::error("Invalid handle passed to getArgsDesc()");
            return {};
        }

        auto sym_ptr = dlsym(handle, functionName.c_str());

        if (!sym_ptr)
        {
            spdlog::error("Failed to load symbol {}: {}", functionName, dlerror());
            return {};
        }

        auto fptr = reinterpret_cast<const char *>(sym_ptr);
        size_t i = 0;
        bool foundParenthesis = false;
        std::string currentArgType;

        while (fptr[i] != '\0' && !foundParenthesis)
        {
            if (fptr[i] == '(')
            {
                foundParenthesis = true;
            }
            i++;
        }

        while (fptr[i] != '\0' && fptr[i] != ')')
        {
            if (fptr[i] == ',')
            {
                result.push_back(std::move(currentArgType));
                currentArgType.clear();
            }
            else
            {
                currentArgType += fptr[i];
            }
            i++;
        }

        if (!currentArgType.empty())
        {
            result.push_back(std::move(currentArgType));
        }

        return result;
    }

}