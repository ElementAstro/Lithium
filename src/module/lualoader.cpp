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
            spdlog::error("LuaScriptLoader: failed to convert integer value");
            return 0;
        }
        return static_cast<int>(lua_tointeger(L, idx));
    }

    template <>
    double to(lua_State *L, int idx)
    {
        if (!lua_isnumber(L, idx))
        {
            spdlog::error("LuaScriptLoader: failed to convert number value");
            return 0.0;
        }
        return lua_tonumber(L, idx);
    }

    template <>
    std::string to(lua_State *L, int idx)
    {
        if (!lua_isstring(L, idx))
        {
            spdlog::error("LuaScriptLoader: failed to convert string value");
            return "";
        }
        return lua_tostring(L, idx);
    }
    
    LuaScriptLoader::LuaScriptLoader()
    {
        // 打开 Lua 库
        luaL_openlibs(luaL_newstate());
    }

    LuaScriptLoader::~LuaScriptLoader()
    {
        for (auto &[name, state] : luaStates_)
        {
            lua_close(state);
        }
    }

    bool LuaScriptLoader::LoadScript(const std::string &name, const std::string &path)
    {
        if (luaStates_.count(name))
        {
            return true;
        }

        auto luaState = luaL_newstate();
        if (!luaState)
        {
            return false;
        }

        luaStates_[name] = luaState;

        // 加载脚本文件
        if (luaL_loadfile(luaState, path.c_str()) || lua_pcall(luaState, 0, 0, 0))
        {
            const char *error = lua_tostring(luaState, -1);
            spdlog::error("LuaScriptLoader: failed to load script '{}': {}", name, error);
            lua_pop(luaState, 1);
            return false;
        }

        return true;
    }

    void LuaScriptLoader::UnloadScript(const std::string &name)
    {
        auto it = luaStates_.find(name);
        if (it == luaStates_.end())
        {
            return;
        }

        auto luaState = it->second;
        ClearFunctions();

        lua_close(luaState);
        luaStates_.erase(it);
    }

    void LuaScriptLoader::ClearFunctions()
    {
        for (const auto &[name, luaState] : luaStates_)
        {
            lua_pushnil(luaState);
            lua_setglobal(luaState, name.c_str());
        }
    }

    void LuaScriptLoader::InjectFunctions(const std::unordered_map<std::string, lua_CFunction> &functions)
    {
        for (const auto &[name, func] : functions)
        {
            for (const auto &[scriptName, luaState] : luaStates_)
            {
                lua_register(luaState, name.c_str(), func);
            }
        }
    }

    void LuaScriptLoader::LoadFunctionsFromJsonFile(const std::string &file_path)
    {
        std::ifstream input(file_path);
        if (!input)
        {
            spdlog::error("LuaScriptLoader: failed to load functions from JSON file '{}'", file_path);
            return;
        }

        json j;
        try
        {
            input >> j;
        }
        catch (json::parse_error &e)
        {
            spdlog::error("LuaScriptLoader: failed to parse JSON file '{}': {}", file_path, e.what());
            return;
        }

        // 从 JSON 中读取函数并注入到 Lua 解释器中
        for (const auto &[name, body] : j.items())
        {
            std::string source = "-- function " + name + "\n" + body.get<std::string>();
            for (const auto &[scriptName, luaState] : luaStates_)
            {
                if (luaL_loadstring(luaState, source.c_str()) || lua_pcall(luaState, 0, 0, 0))
                {
                    const char *error = lua_tostring(luaState, -1);
                    spdlog::error("LuaScriptLoader: failed to load function '{}' in script '{}': {}", name, scriptName, error);
                    lua_pop(luaState, 1);
                    continue;
                }
                lua_pushcfunction(luaState, [](lua_State *L) -> int
                                  {
            lua_Debug ar;
            lua_getstack(L, 1, &ar);
            lua_getinfo(L, "Snl", &ar);
            spdlog::warn("LuaScriptLoader: invalid function call at line {}: {}", ar.currentline, lua_tostring(L, -1));
            lua_pop(L, 1);
            return 0; });
                lua_setglobal(luaState, name.c_str());
            }
        }
    }

    lua_State *LuaScriptLoader::GetLuaState(const std::string &scriptName)
    {
        auto it = luaStates_.find(scriptName);
        if (it == luaStates_.end())
        {
            throw std::runtime_error("Failed to get Lua state for script: " + scriptName);
        }
        return it->second;
    }

}
