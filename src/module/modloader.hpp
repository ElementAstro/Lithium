/*
 * modloader.h
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

#ifndef _MOD_LOADER_H_
#define _MOD_LOADER_H_

#include "openapt.hpp"

class MyApp;

#include <vector>
#include <unordered_map>
#include <cstdio>
#include <functional>

#if defined(WIN32)
// Windows平台
#include <windows.h>
#define MODULE_HANDLE HMODULE
#define LOAD_LIBRARY(p) LoadLibrary(p)
#define LOAD_ERROR() GetLastError()
#define LOAD_SHARED_LIBRARY(file, size) LoadLibraryEx(reinterpret_cast<const char *>(file), NULL, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)
#define LOAD_FUNCTION(handle, name) GetProcAddress(static_cast<HMODULE>(handle), name)
#define CLOSE_SHARED_LIBRARY(handle) FreeLibrary(static_cast<HMODULE>(handle))

#elif defined(__APPLE__)
// macOS平台
#include <dlfcn.h>
#define MODULE_HANDLE void *
#define LOAD_LIBRARY(p) dlopen_ext(p, RTLD_NOW, (const char *[]){"-undefined", "dynamic_lookup", NULL})
#define LOAD_ERROR() dlerror()

#else
// Linux和其他类UNIX平台
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <elf.h>
#define MODULE_HANDLE void *
#define LOAD_LIBRARY(p) dlopen(p, RTLD_NOW | RTLD_GLOBAL)
#define LOAD_SHARED_LIBRARY(file, size) dlopen(nullptr, RTLD_NOW | RTLD_GLOBAL)
#define UNLOAD_LIBRARY(p) dlclose(p)
#define LOAD_ERROR() dlerror()
#define LOAD_FUNCTION(handle, name) dlsym(handle, name)

#endif

#include <spdlog/spdlog.h>

#include "plugins/thread.hpp"
#include "nlohmann/json.hpp"

namespace OpenAPT
{

    nlohmann::json iterator_modules_dir();

    class ModuleLoader
    {
    public:
        ModuleLoader(MyApp* app);
        ~ModuleLoader();
        bool LoadModule(const std::string &path, const std::string &name);
        bool UnloadModule(const std::string &filename);
        bool LoadBinary(const char *dir_path, const char *out_path, const char *build_path, const char *lib_name);

        template <typename T>
        T GetFunction(const std::string &module_name, const std::string &function_name);

        template <typename T>
        std::function<void(T)> GetFunctionObject(const std::string &module_name, const std::string &function_name)
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
            auto func_ptr = reinterpret_cast<void *>(GetProcAddress(handle, function_name.c_str()));
#else
            auto func_ptr = dlsym(handle, function_name.c_str());
#endif

            if (!func_ptr)
            {
                // 获取出错，输出错误信息
                spdlog::error("Failed to get symbol {} from module {}: {}", function_name, module_name, dlerror());
                return nullptr; // 函数不存在，返回空指针
            }

            return *reinterpret_cast<std::function<void(T)> *>(&func_ptr);
        }

        /**
         * @brief LoadAndRunFunction函数模板，用于在指定模块中动态加载并执行指定函数。
         *
         * @tparam T 返回值类型
         * @tparam class_type 类类型
         * @tparam Args 可变参数模板，表示传入函数的参数列表
         * @param module_name 模块名
         * @param func_name 函数名
         * @param thread_name 线程名
         * @param instance 类实例指针，如果函数是类成员函数，则需要传入类实例指针，否则可以传入nullptr
         * @param args 传入函数的实际参数列表
         * @return typename std::enable_if<std::is_class<class_type>::value, bool>::type 如果加载并执行函数成功，则返回true；否则返回false
         */
        template <typename T, typename class_type, typename... Args>
        typename std::enable_if<std::is_class<class_type>::value, bool>::type
        LoadAndRunFunction(const std::string &module_name, const std::string &func_name,
                           const std::string &thread_name, bool runasync, class_type *instance, Args... args)
        {
            typedef T (class_type::*MemberFunctionPtr)(Args...);
            typedef T (*FunctionPtr)(Args...);
            void *handle = GetHandle(module_name);
            auto sym_ptr = dlsym(handle, func_name.c_str());
            if (!sym_ptr)
            {
                spdlog::error("Failed to load symbol {}: {}", func_name, dlerror());
                return false;
            }
            FunctionPtr func_ptr = reinterpret_cast<FunctionPtr>(sym_ptr);
            MemberFunctionPtr member_func_ptr = nullptr;
            if constexpr (std::is_member_function_pointer_v<MemberFunctionPtr>)
            {
                member_func_ptr = reinterpret_cast<MemberFunctionPtr>(sym_ptr);
            }
            m_App->GetThreadManager()->addThread(std::bind(member_func_ptr, instance, args...), std::bind(func_ptr, args...), thread_name);
            return true;
        }

        /**
         * @brief LoadAndRunFunction函数模板，用于在指定模块中动态加载并执行指定函数。
         *
         * @tparam T 返回值类型
         * @tparam Args 可变参数模板，表示传入函数的参数列表
         * @param module_name 模块名
         * @param func_name 函数名
         * @param thread_name 线程名
         * @param args 传入函数的实际参数列表
         * @return T 函数返回值
         */
        template <typename T, typename... Args>
        T LoadAndRunFunction(const std::string &module_name, const std::string &func_name,
                             const std::string &thread_name, bool runasync, Args &&...args)
        {
            // 定义函数指针类型
            typedef T (*FunctionPtr)(Args...);

            // 获取动态链接库句柄
            void *handle = GetHandle(module_name);

            // 加载函数
            auto sym_ptr = dlsym(handle, func_name.c_str());
            if (!sym_ptr)
            {
                spdlog::error("Failed to load symbol {}: {}", func_name, dlerror());
                return static_cast<T>(0);
            }
            FunctionPtr func_ptr = reinterpret_cast<FunctionPtr>(sym_ptr);

            if (runasync)
            {
                m_App->GetThreadManager()->addThread(std::bind(func_ptr, std::forward<Args>(args)...), thread_name);
            }
            else
            {
                auto funcc = std::bind(func_ptr, std::forward<Args>(args)...);
                funcc();
                spdlog::debug("Simple not async function is executed successfully!");
            }

            // 返回函数返回值
            return static_cast<T>(0);
        }

    public:
        void *GetHandle(const std::string &name) const
        {
            auto it = handles_.find(name);
            if (it == handles_.end())
            {
                return nullptr;
            }
            return it->second;
        }

        bool HasModule(const std::string &name) const;

        nlohmann::json getArgsDesc(void *handle, const std::string &functionName);

    private:
        std::unordered_map<std::string, void *> handles_;
        MyApp* m_App;
    };
}

#endif

/*
Example:

    loader.loadLuaModule("module1", "module1.lua");
    loader.loadLuaModule("module2", "module2.lua");
    const auto& functionList1 = loader.getFunctionList("module1");
    const auto& functionList2 = loader.getFunctionList("module2");
    std::cout << "Functions in module1:" << std::endl;
    for (const auto& functionName : functionList1) {
        std::cout << "  " << functionName << std::endl;
    }
    std::cout << "Functions in module2:" << std::endl;
    for (const auto& functionName : functionList2) {
        std::cout << "  " << functionName << std::endl;
    }

    chai

    if (loader.load_script("myscript.chai")) {
        // 获取函数和变量列表
        const std::unordered_map<std::string, std::variant<chaiscript::Type_Info, chaiscript::Boxed_Value>>& chai_functions_ = loader.get_functions_and_variables();
        // 遍历函数和变量列表，输出每个函数和变量的名称和类型
        for (const auto& item : chai_functions_) {
            std::ostringstream oss;
            oss << "Name: " << item.first << ", Type: ";
            if (item.second.index() == 0) {
                chaiscript::Type_Info info = std::get<0>(item.second);
                oss << "Function, Return Type: " << info.bare_name();
            } else if (item.second.index() == 1) {
                chaiscript::Boxed_Value value = std::get<1>(item.second);
                oss << "Variable, Type: " << value.get_type_info().bare_name();
            }
            logger->info(oss.str());
        }
    }
*/