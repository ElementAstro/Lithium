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

    nlohmann::json read_config_file(const std::string& file_path) {
        try {
            // 打开文件流
            std::ifstream file_stream(file_path);
            // 读取文件内容到json对象
            nlohmann::json config = nlohmann::json::parse(file_stream);
            // 关闭文件流
            file_stream.close();
            return config;
        } catch (const std::exception& e) {
            spdlog::error("Failed to read config file {}: {}", file_path, e.what());
            return {{"error", "Failed to read config file"}};
        }
    }
    
    nlohmann::json iterator_modules_dir() {
        fs::path modules_dir{"modules"};
        try {
            if (!fs::exists(modules_dir) || !fs::is_directory(modules_dir)) {
                spdlog::warn("Warning: modules folder not found, creating a new one...");
                fs::create_directory(modules_dir);
            }
        } catch (const std::exception& e) {
            spdlog::error("Failed to create modules directory: {}", e.what());
            return {{"error", "Failed to create modules directory"}};
        }
        nlohmann::json config;
        // 遍历modules目录下的所有子目录
        try {
            for (auto& dir : fs::directory_iterator(modules_dir)) {
                // 如果当前目录是子目录
                if (fs::is_directory(dir)) {
                    // 获取子目录下的info.json文件路径
                    fs::path info_file = dir.path() / "info.json";
                    // 如果info.json文件存在
                    if (fs::exists(info_file)) {
                        // 在json对象中添加子目录路径和info.json文件路径
                        config[dir.path().string()]["path"] = dir.path().string();
                        config[dir.path().string()]["config"] = info_file.string();
                        // 调用读取配置文件的函数，将读取到的信息存入config对象
                        nlohmann::json module_config = read_config_file(info_file.string());
                        config[dir.path().string()]["name"] = module_config["name"];
                        config[dir.path().string()]["version"] = module_config["version"];
                        config[dir.path().string()]["author"] = module_config["author"];
                        config[dir.path().string()]["license"] = module_config["license"];
                        config[dir.path().string()]["description"] = module_config["description"];
                        // 调试输出信息
                        spdlog::debug("Module found: {}, config file: {}", dir.path().string(), info_file.string());
                    }
                }
            }
        } catch (const std::exception& e) { // 捕获异常
            spdlog::error("Failed to iterate modules directory: {}", e.what());
            return {{"error", "Failed to iterate modules directory"}};
        }
        if(config.empty()){
            config["message"] = "No module found";
        }
        // 返回json对象
        return config;
    }


    ModuleLoader::ModuleLoader(){
        spdlog::info("C++ module manager loaded successfully.");
        Py_Initialize();
        spdlog::info("Python module manager loaded successfully.");
    }

    ModuleLoader::~ModuleLoader(){
        for (auto& entry : handles_) {
            UNLOAD_LIBRARY(entry.second);
        }

        for (auto& [scriptName, moduleObj] : python_modules_) {
            Py_DECREF(moduleObj);
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
        void* handle = nullptr;
        try {
            if (!std::filesystem::exists(path)) {  // 检查文件是否存在
                spdlog::error("Library {} does not exist", path);
                return false;
            }

            // 加载动态库
            handle = LOAD_LIBRARY(path.c_str());

            // 读取版本信息
            std::string ini_file_path = std::filesystem::path(path).replace_extension(".ini");  // 构建info.ini文件路径
            try {
                if (std::filesystem::exists(ini_file_path)) {
                    boost::property_tree::ptree tree;
                    boost::property_tree::ini_parser::read_ini(ini_file_path, tree);
                    boost::optional<std::string> name = tree.get_optional<std::string>("name");
                    boost::optional<std::string> version = tree.get_optional<std::string>("version");
                    boost::optional<std::string> author = tree.get_optional<std::string>("author");
                    boost::optional<std::string> license = tree.get_optional<std::string>("license");
                    if (version && author) {
                        spdlog::info("Loaded Module : {} version {} written by {}{}",
                            name ? *name : "Unknown", *version, *author,
                            license ? " under " + *license : "");
                    } else {
                        spdlog::warn("Missing required fields in {}", ini_file_path);
                    }
                } else {
                    spdlog::warn("Info file {} does not exist", ini_file_path);
                }
            } catch (const std::exception& e) {
                spdlog::error("Failed to load info from {}: {}", ini_file_path, e.what());
            }
        } catch (const std::exception& e) {
            spdlog::error("Failed to load library {}: {}", path, e.what());
            return false;
        }

        if (!handle) {
            spdlog::error("Failed to load library {}: {}", path, LOAD_ERROR());
            return false;
        }
        // 将句柄保存到handles_中
        handles_[name] = handle;
        return true;
    }

    /**
     * @brief 卸载指定名称的动态库
     * 
     * @param filename [in] 要卸载的动态库的文件名（包括扩展名）
     * @return true 动态库卸载成功
     * @return false 动态库卸载失败
     */
    bool ModuleLoader::UnloadModule(const std::string& filename) {
        auto it = handles_.find(filename);
        if (it != handles_.end()) {
            // 尝试卸载模块
            int result;
            #if defined(_WIN32) || defined(_WIN64)
                result = FREE_LIBRARY(it->second);
            #else
                result = dlclose(it->second);
            #endif
            if (result == 0) {
                // 卸载成功，移除句柄记录
                spdlog::info("Unloaded module : {}", filename);
                handles_.erase(it);
                return true;
            }
            else {
                spdlog::error("Failed to unload module : {}", filename);
                return false;
            }
        }
        else {
            // 模块未加载
            spdlog::error("Module {} not loaded", filename);
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
    bool ModuleLoader::LoadBinary(const char *dir_path, const char *out_path, const char *build_path, const char *lib_name)
    {
        DIR *dir;
        struct dirent *ent;
        struct stat file_stat;
        char cmake_path[512];
        char lib_path[512];
        bool ret = true;    // 初始化返回值为true

        // 检查目录是否存在
        dir = opendir(dir_path);
        if (!dir)
        {
            spdlog::error("Failed to open directory {}: {}", dir_path, strerror(errno));
            return false;
        }

        // 查找CMakeLists.txt文件
        while ((ent = readdir(dir)) != NULL)
        {
            if (strcmp(ent->d_name, "CMakeLists.txt") == 0)
            {
                snprintf(cmake_path, sizeof(cmake_path), "%s/%s", dir_path, ent->d_name);
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
        snprintf(lib_path, sizeof(lib_path), "%s/lib%s.so", build_path, lib_name);
        if (access(lib_path, F_OK) == 0)
        {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "cp -av %s %s/", lib_path, out_path);
            if (system(cmd) != 0)
            {
                spdlog::error("Failed to copy dynamic library");
                ret = false;    // 执行命令失败，设置返回值为false
            }

            // 删除build目录
            if (chdir(dir_path) == -1)
            {
                spdlog::error("Failed to change working directory to {}: {}", dir_path, strerror(errno));
                return false;
            }

            char remove_cmd[512];
            printf("\nDo you really want to remove build directory %s? (y/n): ", build_path);
            fflush(stdout);
            snprintf(remove_cmd, sizeof(remove_cmd), "rm -rf %s", build_path);
            if (system(remove_cmd) != 0)
            {
                spdlog::error("Failed to remove build directory");
                // 只是删除build目录失败，并不影响主逻辑，不需要退出
            }
            return true;
        }

        // 执行cmake
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "cmake -DCMAKE_BUILD_TYPE=Release -D LIBRARY_NAME=%s ..", lib_name);
        if (system(cmd) != 0)
        {
            spdlog::error("Failed to run cmake");
            ret = false;    // 执行命令失败，设置返回值为false
        }

        // 执行make
        if (system("make") != 0)
        {
            spdlog::error("Failed to run make");
            ret = false;    // 执行命令失败，设置返回值为false
        }

        // 复制生成的动态库
        snprintf(cmd, sizeof(cmd), "cp -av lib%s.so %s/", lib_name, out_path);
        if (system(cmd) != 0)
        {
            spdlog::error("Failed to copy dynamic library");
            ret = false;    // 执行命令失败，设置返回值为false
        }

        // 删除build目录
        if (chdir(dir_path) == -1)
        {
            spdlog::error("Failed to change working directory to {}: {}", dir_path, strerror(errno));
            return false;
        }

        char remove_cmd[512];
        printf("\nDo you really want to remove build directory %s? (y/n): ", build_path);
        fflush(stdout);
        snprintf(remove_cmd, sizeof(remove_cmd), "rm -rf %s", build_path);
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
    template<typename T>
    T ModuleLoader::GetFunction(const std::string& module_name, const std::string& function_name) {
        // 获取动态库句柄
        auto handle = handles_[module_name];
        // 获取函数指针
        void* func_ptr;
        #if defined(_WIN32) || defined(_WIN64)
            func_ptr = reinterpret_cast<void*>(GetProcAddress(handle, function_name.c_str()));
        #else
            func_ptr = dlsym(handle, function_name.c_str());
        #endif
        if (!func_ptr) {
            spdlog::error("Failed to get symbol {} from module {}", function_name, module_name);
            return nullptr; // 函数不存在，返回空指针
        }
        // 转换函数指针类型，并返回该函数指针
        return reinterpret_cast<T>(func_ptr);
    }

    bool ModuleLoader::LoadPythonScript(const std::string& scriptName){
        PyObject* pModule = PyImport_ImportModule(scriptName.c_str());
        if (!pModule) {
            spdlog::error("Failed to load Python module: {}",scriptName);
            return false;
        }
        python_modules_[scriptName] = pModule;
        return true;
    }

    void ModuleLoader::UnloadPythonScript(const std::string& scriptName) {
        auto iter = python_modules_.find(scriptName);
        if (iter != python_modules_.end()) {
            Py_DECREF(iter->second);
            python_modules_.erase(iter);
        }
    }

    std::vector<std::string> ModuleLoader::getPythonFunctions(const std::string& scriptName) {
        std::vector<std::string> result;

        auto iter = python_modules_.find(scriptName);
        if (iter == python_modules_.end()) {
            std::cerr << "Script not found: " << scriptName << std::endl;
            return result;
        }

        PyObject* pDict = PyModule_GetDict(iter->second);
        PyObject* pKey, * pValue;
        Py_ssize_t pos = 0;

        while (PyDict_Next(pDict, &pos, &pKey, &pValue)) {
            if (PyCallable_Check(pValue)) {
                std::string functionName = PyUnicode_AsUTF8(pKey);
                result.push_back(functionName);
            }
        }

        return result;
    }

            template<typename... Args>
            bool ModuleLoader::runPythonFunction(const std::string& scriptName, const std::string& functionName, Args... args) {
                auto iter = python_modules_.find(scriptName);
                if (iter == python_modules_.end()) {
                    spdlog::error("Script not found: {}",scriptName);
                    return false;
                }

                PyObject* pModule = iter->second;
                PyObject* pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
                if (!pFunc || !PyCallable_Check(pFunc)) {
                    spdlog::error("Function not found: {}",functionName);
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

            /*
            runFunction("my_script.py", "myFunction", scriptLoader, [](){
                std::cout << "myFunction has finished executing." << std::endl;
            }, 42, "Hello, world!");
            */
            template<typename F, typename... Args>
            void ModuleLoader::AysncRunPythonFunction(const std::string& scriptName, const std::string& functionName, ModuleLoader& scriptLoader, F&& callback, Args&&... args) {
                auto task = [scriptName, functionName, &scriptLoader, args..., callback]() {
                    PyGILState_STATE gstate = PyGILState_Ensure();

                    scriptLoader.AysncRunPythonFunction(scriptName, functionName, std::forward<Args>(args)...);

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