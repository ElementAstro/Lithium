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

#include <openssl/md5.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <curl/curl.h>

namespace fs = std::filesystem;
namespace pt = boost::property_tree;

namespace OpenAPT{

    // Function: read_config_file()
    // Description: Read a JSON configuration file and return its content as a JSON object.
    // Parameters:
    // - file_path (const std::string&) : The path of the configuration file to be read.
    // Return value: nlohmann::json - A JSON object containing the configuration information or an error message.
    /**
     * @brief 读取一个 JSON 配置文件，将其内容以 JSON 对象的形式返回。
     *
     * 该函数接收一个 JSON 配置文件的路径，打开文件流，读取文件内容到 JSON 对象中，然后关闭文件流并返回 JSON 对象。如果读取过程中出现任何异常，
     * 则返回一个包含 "error" 字段和错误消息的 JSON 对象。
     *
     * @param file_path (const std::string&) : 要读取的配置文件路径。
     * @return nlohmann::json - 包含配置信息或错误消息的 JSON 对象。
     */
    nlohmann::json read_config_file(const std::string& file_path) {
        try {
            // Open the configuration file
            std::ifstream file_stream(file_path);
            if (!file_stream.is_open()) {
                spdlog::error("Failed to open config file {}", file_path);
                return { {
                        "error", "Failed to open config file"
                    }
                }
                ;
            }
            // Read the configuration file content into a JSON object
            nlohmann::json config = nlohmann::json::parse(file_stream);
            // Close the file stream
            file_stream.close();
            return config;
        }
        catch (const std::exception& e) {
            spdlog::error("Failed to read config file {}: {}", file_path, e.what());
            return { {
                    "error", "Failed to read config file"
                }
            }
            ;
        }
    }
    
    // Function: iterator_modules_dir()
    // Description: Traverse the "modules" directory and create a JSON object containing the information of all modules.
    // Return value: nlohmann::json - A JSON object containing the module information or an error message.
    /**
     * @brief 遍历 "modules" 目录及其子目录，创建包含所有模块信息的 JSON 对象。
     *
     * 该函数遍历 "modules" 目录及其子目录，在每个包含 "info.json" 配置文件的子目录中创建一个 JSON 对象，其中包含子目录的路径、配置文件的路径，
     * 以及模块的名称、版本、作者、许可证和描述。如果没有找到任何模块，则返回一个包含消息指示没有模块的 JSON 对象。如果出现任何异常，
     * 则返回一个包含 "error" 字段和错误消息的 JSON 对象。
     *
     * @return nlohmann::json - 包含所有模块信息或错误消息的 JSON 对象。
     */
    nlohmann::json iterator_modules_dir() {
        // Define the modules directory path
        fs::path modules_dir;
        #ifdef _WIN32 // Windows OS
        modules_dir = fs::path(getenv("USERPROFILE")) / "Documents" / "modules";
        # else // Linux OS
        modules_dir = "modules";
        #endif
        try {
            // Create the modules directory if it does not exist
            if (!fs::exists(modules_dir) || !fs::is_directory(modules_dir)) {
                spdlog::warn("Warning: modules folder not found, creating a new one...");
                fs::create_directory(modules_dir);
            }
        } catch (const std::exception& e) {
            spdlog::error("Failed to create modules directory: {}", e.what());
            return { {
                    "error", "Failed to create modules directory"
                }
            }
            ;
        }
        // Create a JSON object to store module information
        nlohmann::json config;
        try {
            // Iterate through each subdirectory of the modules directory
            for (auto& dir : fs::directory_iterator(modules_dir)) {
                // Check if the current directory is indeed a subdirectory
                if (fs::is_directory(dir)) {
                    // Get the path of the info.json file within the subdirectory
                    fs::path info_file = dir.path() / "info.json";
                    // If the info.json exists
                    if (fs::exists(info_file)) {
                        // Append necessary information to the JSON object
                        config[dir.path().string()]["path"] = dir.path().string();
                        config[dir.path().string()]["config"] = info_file.string();
                        // Read the module configuration from the info.json file and append to the JSON object
                        nlohmann::json module_config = read_config_file(info_file.string());
                        config[dir.path().string()]["name"] = module_config["name"];
                        config[dir.path().string()]["version"] = module_config["version"];
                        config[dir.path().string()]["author"] = module_config["author"];
                        config[dir.path().string()]["license"] = module_config["license"];
                        config[dir.path().string()]["description"] = module_config["description"];
                        // Debug message
                        spdlog::debug("Module found: {}, config file: {}", dir.path().string(), info_file.string());
                    }
                }
            }
        } catch (const std::exception& e) {
            spdlog::error("Failed to iterate modules directory: {}", e.what());
            return { {
                    "error", "Failed to iterate modules directory"
                }
            }
            ;
        }
        // If no module is found, append a message field to the JSON object
        if(config.empty()) {
            config["message"] = "No module found";
        }
        // Return the JSON object
        return config;
    }

    ModuleLoader::ModuleLoader(){
        spdlog::info("C++ module manager loaded successfully.");
        Py_Initialize();
        spdlog::info("Python module manager loaded successfully.");
    }

    ModuleLoader::~ModuleLoader(){
        if (!handles_.empty()) {
            for (auto& entry : handles_) {
                UNLOAD_LIBRARY(entry.second);
            }
        }
        
        if (!python_modules_.empty()) {
            for (auto& [scriptName, moduleObj] : python_modules_) {
                Py_DECREF(moduleObj);
            }
        }
        Py_Finalize();
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
    bool ModuleLoader::LoadModule(const std::string& path, const std::string& name) {
        try {
            if (!std::filesystem::exists(path)) {  // 检查文件是否存在
                spdlog::error("Library {} does not exist", path);
                return false;
            }

            // 加载动态库
            void* handle = LOAD_LIBRARY(path.c_str());
            if (!handle) {
                spdlog::error("Failed to load library {}: {}", path, LOAD_ERROR());
                return false;
            }

            // 读取配置文件
            std::string config_file_path = std::filesystem::path(path).replace_extension(".json");
            if (std::filesystem::exists(config_file_path)) {
                nlohmann::json config;
                std::ifstream config_file(config_file_path);
                config_file >> config;

                if (config.contains("name") && config.contains("version") && config.contains("author")) {
                    std::string version = config["version"].get<std::string>();
                    std::string author = config["author"].get<std::string>();
                    std::string license = config.value("license", "");

                    spdlog::info("Loaded Module : {} version {} written by {}{}",
                        config.value("name", "Unknown"), version, author, license.empty() ? "" : " under " + license);
                } else {
                    spdlog::warn("Missing required fields in {}", config_file_path);
                }
            } else {
                spdlog::warn("Config file {} does not exist", config_file_path);
            }

            // 将句柄保存到handles_中
            handles_[name] = handle;
            return true;
        } catch (const std::exception& e) {
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
    bool ModuleLoader::UnloadModule(const std::string& filename) {
        try {
            auto it = handles_.find(filename);
            if (it == handles_.end()) {
                throw std::runtime_error("Module " + filename + " is not loaded");
            }

            if (!it->second) {
                throw std::runtime_error("Module " + filename + "'s handle is null");
            }

            int result = UNLOAD_LIBRARY(it->second);
            if (result == 0) {
                spdlog::info("Unloaded module : {}", filename);
                handles_.erase(it);
                return true;
            } else {
                throw std::runtime_error("Failed to unload module " + filename);
            }
        } catch (const std::exception& e) {
            spdlog::error("{}", e.what());
            return false;
        }
    }


    /**
     * @brief 从指定目录中编译生成指定名称的动态库，并将其复制到指定路径中。
     * 
     * @param dir_path [in] 动态库源码目录的绝对路径
     * @param out_path [in] 动态库输出目录的绝对路径
     * @param build_path [in] 编译过程中生成的临时文件目录的绝对路径
     * @param lib_name [in] 要生成的动态库的名称（不包括扩展名）
     * @return true 动态库生成并复制成功
     * @return false 动态库生成或复制失败
     */
    #ifdef _WIN32 // 判断操作系统，Windows下使用不同的路径分隔符
    #define PATH_SEPARATOR "\\"
    #else
    #define PATH_SEPARATOR "/"
    #endif

    bool ModuleLoader::LoadBinary(const char *dir_path, const char *out_path, const char *build_path, const char *lib_name)
    {
        DIR *dir;
        struct dirent *ent;
        struct stat file_stat;
        char cmake_path[512];
        char lib_path[512];
        bool ret = true;

        // 检查目录是否存在
        dir = opendir(dir_path);
        if (!dir)
        {
            spdlog::error("Failed to open directory {}: {}", dir_path, strerror(errno));
            return false;
        }

        // 查找CMakeLists.txt文件
        while ((ent = readdir(dir)) != nullptr)
        {
            if (strcmp(ent->d_name, "CMakeLists.txt") == 0)
            {
                snprintf(cmake_path, sizeof(cmake_path), "%s%s%s", dir_path, PATH_SEPARATOR, ent->d_name);
                break;
            }
        }

        closedir(dir);

        // 如果找不到CMakeLists.txt文件，则返回错误
        if (!ent)
        {
            spdlog::error("Could not find CMakeLists.txt in directory {}", dir_path);
            return false;
        }

        // 创建build目录
        if (mkdir(build_path, 0777) == -1 && errno != EEXIST)
        {
            spdlog::error("Failed to create build directory: {}", strerror(errno));
            return false;
        }

        // 切换到build目录
        if (chdir(build_path) == -1)
        {
            spdlog::error("Failed to change working directory to {}: {}", build_path, strerror(errno));
            return false;
        }

        // 检查动态库是否已存在，若已存在则直接复制
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

            // 删除build目录
            if (chdir(dir_path) == -1)
            {
                spdlog::error("Failed to change working directory to {}: {}", dir_path, strerror(errno));
                return false;
            }

            char remove_cmd[512];
    #ifndef _WIN32 // Windows下没有rm命令，使用del命令
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

        // 执行cmake
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "cmake -DCMAKE_BUILD_TYPE=Release -D LIBRARY_NAME=%s ..", lib_name);
        if (system(cmd) != 0)
        {
            spdlog::error("Failed to run cmake");
            ret = false;
        }

        // 执行make
    #ifdef _WIN32 // Windows下使用nmake命令
        snprintf(cmd, sizeof(cmd), "nmake");
    #else
        snprintf(cmd, sizeof(cmd), "make");
    #endif
        if (system(cmd) != 0)
        {
            spdlog::error("Failed to run make");
            ret = false;
        }

        // 复制生成的动态库
        snprintf(cmd, sizeof(cmd), "cp -av lib%s.so %s%s", lib_name, out_path, PATH_SEPARATOR);
        if (system(cmd) != 0)
        {
            spdlog::error("Failed to copy dynamic library");
            ret = false;
        }

        // 删除build目录
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

        // 返回最终结果
        return ret;
    }


    /**
     * @brief 获取动态库中指定符号名的函数指针
     * 
     * @tparam T 函数指针类型
     * @param module_name 动态库名称
     * @param function_name 函数符号名
     * @return T 函数指针
     */
    template <typename T>
    T ModuleLoader::GetFunction(const std::string& module_name, const std::string& function_name)
    {
        // 获取动态库句柄
        auto handle_it = handles_.find(module_name);
        if (handle_it == handles_.end())
        {
            spdlog::error("Failed to find module {}", module_name);
            return nullptr; // 动态库不存在，返回空指针
        }
        auto handle = handle_it->second;

    #ifdef _WIN32
        auto func_ptr = reinterpret_cast<void*>(GetProcAddress(handle, function_name.c_str()));
    #else
        auto func_ptr = dlsym(handle, function_name.c_str());
    #endif

        if (!func_ptr)
        {
            // 获取出错，输出错误信息
            spdlog::error("Failed to get symbol {} from module {}: {}", function_name, module_name, dlerror());
            return nullptr; // 函数不存在，返回空指针
        }
        return reinterpret_cast<T>(func_ptr);
    }


    /**
     * @brief 加载Python脚本
     * 
     * @param scriptName Python脚本的文件名
     * @return true 加载成功
     * @return false 加载失败
     */
    bool ModuleLoader::LoadPythonScript(const std::string& scriptName) {
        PyObject* pModule = PyImport_ImportModule(scriptName.c_str());
        if (!pModule) {
            spdlog::error("Failed to load Python module: {}", scriptName);
            PyErr_Print(); // 输出异常信息
            return false;
        }
        python_modules_[scriptName] = pModule;
        return true;
    }

    /**
     * @brief 卸载Python脚本
     * 
     * @param scriptName Python脚本的文件名
     */
    void ModuleLoader::UnloadPythonScript(const std::string& scriptName) {
        auto iter = python_modules_.find(scriptName);
        if (iter != python_modules_.end()) {
            Py_DECREF(iter->second);
            python_modules_.erase(iter);
        }
    }

    /**
     * @brief 获取Python脚本中定义的所有函数名
     * 
     * @param scriptName Python脚本的文件名
     * @return std::vector<std::string> 所有函数名列表
     */
    std::vector<std::string> ModuleLoader::getPythonFunctions(const std::string& scriptName) {
        std::vector<std::string> result;

        auto iter = python_modules_.find(scriptName);
        if (iter == python_modules_.end()) {
            spdlog::error("Script not found: {}", scriptName);
            PyErr_Print(); // 输出异常信息
            return result;
        }

        PyObject* pDict = PyModule_GetDict(iter->second);
        if (!pDict) {
            spdlog::error("Failed to get dictionary of module: {}", scriptName);
            PyErr_Print(); // 输出异常信息
            return result;
        }

        PyObject* pKey, * pValue;
        Py_ssize_t pos = 0;

        while (PyDict_Next(pDict, &pos, &pKey, &pValue)) {
            if (PyCallable_Check(pValue)) {
                PyObject* pStr = PyObject_Str(pKey);
                if (!pStr) {
                    PyErr_Print(); // 输出异常信息
                    continue;
                }
                std::string functionName = PyUnicode_AsUTF8(pStr);
                result.push_back(functionName);
                Py_DECREF(pStr);
            }
        }

        return result;
    }

            /**
     * @brief 调用Python脚本中的函数
     * 
     * @tparam Args 函数参数列表
     * @param scriptName Python脚本的文件名
     * @param functionName 函数名
     * @param args 函数参数列表
     * @return true 调用成功
     * @return false 调用失败
     */
    template<typename... Args>
    bool ModuleLoader::RunPythonFunction(const std::string& scriptName, const std::string& functionName, Args... args) {
        auto iter = python_modules_.find(scriptName);
        if (iter == python_modules_.end()) {
            spdlog::error("Error: Script not found: {}",scriptName);
            return false;
        }

        PyObject* pModule = iter->second;
        PyObject* pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
        if (!pFunc || !PyCallable_Check(pFunc)) {
            spdlog::error("Error: Function not found: {}",functionName);
            Py_XDECREF(pFunc);
            return false;
        }

        PyObject* pArgs = PyTuple_New(sizeof...(args));
        int i = 0;
        (void)std::initializer_list<int>{(PyTuple_SetItem(pArgs, i++, Py_BuildValue("d", args)), 0)...};

        PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
        if (!pResult) {
            PyErr_Print();
            Py_XDECREF(pFunc);
            Py_XDECREF(pArgs);
            return false;
        }

        Py_XDECREF(pFunc);
        Py_XDECREF(pArgs);
        Py_XDECREF(pResult);

        return true;
    }

    /**
     * @brief 异步调用Python脚本中的函数
     * 
     * @tparam F 回调函数类型
     * @tparam Args 函数参数列表
     * @param scriptName Python脚本的文件名
     * @param functionName 函数名
     * @param scriptLoader ModuleLoader对象引用，用于异步调用
     * @param callback 回调函数，在Python函数执行结束后调用
     * @param args 函数参数列表
     */
    template<typename F, typename... Args>
    void AsyncRunPythonFunction(const std::string& scriptName, const std::string& functionName, ModuleLoader& scriptLoader, F&& callback, Args&&... args) {
        auto task = [scriptName, functionName, &scriptLoader, args..., callback]() {
            PyGILState_STATE gstate = PyGILState_Ensure();

            if (!scriptLoader.RunPythonFunction(scriptName, functionName, std::forward<Args>(args)...)) {
                spdlog::error("Error: Failed to run Python function: {}",functionName);
            }

            PyGILState_Release(gstate);

            if (callback) {
                callback();
            }
        };

        if (callback) {
            std::async(std::launch::async, std::move(task));
        } else {
            task();
        }
    }
}