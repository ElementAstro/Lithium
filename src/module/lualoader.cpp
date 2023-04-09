/*
 * lualoader.cpp
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

#include "lualoader.hpp"

namespace OpenAPT
{
    template <typename T>
    void push(lua_State *L, const T &val) {}

    template <>
    void push(lua_State *L, const int &val)
    {
        lua_pushinteger(L, val);
    }

    template <>
    void push(lua_State *L, const double &val)
    {
        lua_pushnumber(L, val);
    }

    template <>
    void push(lua_State *L, const std::string &val)
    {
        lua_pushstring(L, val.c_str());
    }

    template <>
    void push(lua_State *L, const char *const &val)
    {
        lua_pushstring(L, val);
    }

    template <typename T>
    T to(lua_State *L, int idx) { return {}; }

    template <>
    int to(lua_State *L, int idx)
    {
        if (!lua_isinteger(L, idx))
        {
            throw std::runtime_error("Invalid integer value");
        }
        return static_cast<int>(lua_tointeger(L, idx));
    }

    template <>
    double to(lua_State *L, int idx)
    {
        if (!lua_isnumber(L, idx))
        {
            throw std::runtime_error("Invalid number value");
        }
        return lua_tonumber(L, idx);
    }

    template <>
    std::string to(lua_State *L, int idx)
    {
        if (!lua_isstring(L, idx))
        {
            throw std::runtime_error("Invalid string value");
        }
        return lua_tostring(L, idx);
    }

    LuaScriptLoader::LuaScriptLoader()
        : L(luaL_newstate())
    {

        // 打开 Lua 库
        luaL_openlibs(L);
    }

    LuaScriptLoader::~LuaScriptLoader()
    {
        lua_close(L);
    }

    bool LuaScriptLoader::LoadScript(const std::string &path)
    {
        // 加载脚本文件
        if (luaL_loadfile(L, path.c_str()) || lua_pcall(L, 0, 0, 0))
        {
            const char *error = lua_tostring(L, -1);
            throw std::runtime_error(error);
        }

        return true;
    }

    template <typename T, typename... Args>
    T LuaScriptLoader::CallFunction(const std::string &name, Args &&...args)
    {
        lua_getglobal(L, name.c_str());
        if (!lua_isfunction(L, -1))
        {
            throw std::runtime_error("Invalid function");
        }

        int nArgs = sizeof...(args);
        int nResults = 1; // 默认返回值个数为1

        // 压入函数参数
        int index = 1;
        (push(L, std::forward<Args>(args)), ...);

        // 调用函数
        if (lua_pcall(L, nArgs, nResults, 0) != 0)
        {
            const char *error = lua_tostring(L, -1);
            lua_pop(L, 1); // 异常出现时，需要将栈顶元素弹出
            spdlog::error("LuaScriptLoader: failed to call function '{}': {}", name, error);
            throw std::runtime_error(error);
        }

        // 获取返回值
        T result = to<T>(L, -1);
        lua_pop(L, 1); // 弹出返回值
        return result;
    }

    template <typename T>
    void LuaScriptLoader::SetGlobal(const std::string &name, const T &value)
    {
        push(L, value);
        lua_setglobal(L, name.c_str());
    }

    template <typename T>
    T LuaScriptLoader::GetGlobal(const std::string &name)
    {
        lua_getglobal(L, name.c_str());
        if (!lua_isuserdata(L, -1))
        {
            throw std::runtime_error("Invalid global variable");
        }
        T result = to<T>(L, -1);
        lua_pop(L, 1);
        return result;
    }

    void LuaScriptLoader::UnloadScript()
    {
        ClearFunctions();
    }

    void LuaScriptLoader::ClearFunctions()
    {
        for (const auto &[name, ptr] : functions_)
        {
            lua_pushnil(L);
            lua_setglobal(L, name.c_str());
        }
        functions_.clear();
    }

    void LuaScriptLoader::InjectFunctions(const std::unordered_map<std::string, lua_CFunction> &functions)
    {
        for (const auto &[name, func] : functions)
        {
            lua_register(L, name.c_str(), func);
        }
    }

    void LuaScriptLoader::LoadFunctionsFromJsonFile(const std::string &file_path)
    {
        std::ifstream input(file_path);
        if (input.fail())
        {
            spdlog::error("LuaScriptLoader: failed to load functions from JSON file '{}'", file_path);
            throw std::runtime_error("Failed to load JSON file");
        }

        json j;
        try
        {
            input >> j;
        }
        catch (json::parse_error &e)
        {
            spdlog::error("LuaScriptLoader: failed to parse JSON file '{}': {}", file_path, e.what());
            throw std::runtime_error("Failed to parse JSON file");
        }

        // 从 JSON 中读取函数并注入到 Lua 解释器中
        for (const auto &[name, body] : j.items())
        {
            std::string source = "-- function " + name + "\n" + body.get<std::string>();
            if (luaL_loadstring(L, source.c_str()) || lua_pcall(L, 0, 0, 0))
            {
                const char *error = lua_tostring(L, -1);
                spdlog::error("LuaScriptLoader: failed to load function '{}': {}", name, error);
                throw std::runtime_error(error);
            }
        }
    }

}
