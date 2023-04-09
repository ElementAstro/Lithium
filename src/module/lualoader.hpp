/*
 * lualoader.hpp
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

Date: 2023-4-9

Description: Lua Module Loader

**************************************************/

#ifndef LUASCRIPTLOADER_H
#define LUASCRIPTLOADER_H

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <fstream>
#include <nlohmann/json.hpp>

#include "spdlog/spdlog.h"

extern "C" {
    #include <lua/lua.h>
    #include <lua/lualib.h>
    #include <lua/lauxlib.h>
}

using json = nlohmann::json;

namespace OpenAPT {
    class LuaScriptLoader {
    public:

        /**
         * @brief 构造函数, 创建一个新的Lua解释器，并打开所有标准库.
         */
        LuaScriptLoader();

        /**
         * @brief 析构函数，关闭当前的LUA状态.
         */
        ~LuaScriptLoader();

        /**
         * @brief LoadScript 函数用于从文件中加载LUA脚本代码并执行.
         * 
         * @throws std::runtime_error 如果加载或执行代码时发生错误, 将抛出异常.
         * 
         * @param path 要加载的文件路径.
         * 
         * @return true 如果成功加载并执行脚本; false 如果无法加载或执行脚本.
         */
        bool LoadScript(const std::string& path);

        /**
         * @brief CallFunction 函数用于调用LUA中的函数，并获取它的返回值.
         * 
         * @throws std::runtime_error 如果无法调用指定的函数或获取其返回值时发生错误，将抛出异常.
         * 
         * @tparam T 要返回的值的类型.
         * @tparam Args 调用函数时传递的参数类型.
         * 
         * @param name 要调用的函数的名称.
         * @param args 调用函数时传递的参数.
         * 
         * @return T 函数返回的值.
         */
        template<typename T, typename... Args>
        T CallFunction(const std::string &name, Args&&... args);

        /**
         * @brief SetGlobal 函数用于在LUA中设置全局变量.
         * 
         * @tparam T 要设置的值的类型.
         * 
         * @param name 全局变量的名称.
         * @param value 要设置的值.
         */
        template<typename T>
        void SetGlobal(const std::string& name, const T& value);

        /**
         * @brief GetGlobal 函数用于从LUA中获取全局变量的值.
         * 
         * @tparam T 要返回的值的类型.
         * 
         * @throws std::runtime_error 如果无法获取指定全局变量的值, 将抛出异常.
         * 
         * @param name 要获取其值的全局变量的名称.
         * 
         * @return T 全局变量的值.
         */
        template<typename T>
        T GetGlobal(const std::string& name);

        /**
         * @brief UnloadScript 函数用于卸载当前加载的LUA脚本.
         */
        void UnloadScript();

        /**
         * @brief InjectFunctions 函数用于将一组C/C++函数注入到LUA解释器中.
         * 
         * @param functions 要注入的函数的名称和指针的映射.
         */
        void InjectFunctions(const std::unordered_map<std::string, lua_CFunction>& functions);

        /**
         * @brief LoadFunctionsFromJsonFile 函数用于从JSON文件中读取字符串表示的LUA函数并注入到LUA解释器中.
         * 
         * @throws std::runtime_error 如果无法加载或解析指定的JSON文件时，将抛出异常.
         * 
         * @param file_path 要加载的JSON文件的路径.
         */
        void LoadFunctionsFromJsonFile(const std::string& file_path);

    private:

        void ClearFunctions();

        lua_State* L; ///< 存储当前LUA状态的指针.
        std::unordered_map<std::string, void*> functions_; ///< 存储已经注册的C/C++函数的名称和指针的映射.
    };
}

#endif // LUASCRIPTLOADER_H
