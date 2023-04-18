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

#include <string>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "nlohmann/json.hpp"
#include "lua/lua.hpp"

using json = nlohmann::json;

namespace OpenAPT
{
    template <typename T>
    void push(lua_State *L, const T &val)
    {
        luaL_error(L, "LuaScriptLoader: unsupported type");
    }

    template <typename... Args>
    void push(lua_State *L, Args &&...args)
    {
        (lua_pushinteger(L, args), ...);
    }

    template <typename... Args>
    void push(lua_State *L, const int &val, Args &&...args)
    {
        lua_pushinteger(L, val);
        push(L, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(lua_State *L, const double &val, Args &&...args)
    {
        lua_pushnumber(L, val);
        push(L, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(lua_State *L, const std::string &val, Args &&...args)
    {
        lua_pushlstring(L, val.data(), val.length());
        push(L, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void push(lua_State *L, const char *const &val, Args &&...args)
    {
        lua_pushstring(L, val);
        push(L, std::forward<Args>(args)...);
    }

    template <typename T>
    T to(lua_State *L, int idx)
    {
        luaL_error(L, "LuaScriptLoader: unsupported type");
        return {};
    }

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
        size_t len;
        const char *str = lua_tolstring(L, idx, &len);
        if (!str)
        {
            spdlog::error("LuaScriptLoader: failed to convert string value");
            return "";
        }
        return std::string(str, len);
    }

    class LuaScriptLoader
    {
    public:
        LuaScriptLoader();

        ~LuaScriptLoader();

        bool LoadScript(const std::string &name, const std::string &path);

        void UnloadScript(const std::string &name);

        void ClearFunctions();

        template <typename T, typename... Args>
        bool CallFunction(const std::string &name, const std::string &scriptName, T &result, Args &&...args)
        {
            auto luaState = GetLuaState(scriptName);

            lua_getglobal(luaState, name.c_str());
            if (!lua_isfunction(luaState, -1))
            {
                lua_pop(luaState, 1);
                spdlog::error("LuaScriptLoader: failed to call function '{}': Invalid function", name);
                return false;
            }

            // 压入函数参数
            push(luaState, args...);

            // 调用函数
            try
            {
                if (lua_pcall(luaState, sizeof...(args), 1, 0) != 0)
                {
                    throw std::runtime_error(lua_tostring(luaState, -1));
                }

                result = to<T>(luaState, -1);
            }
            catch (const std::exception &ex)
            {
                spdlog::error("LuaScriptLoader: failed to call function '{}': {}", name, ex.what());
                lua_pop(luaState, 1); // 异常出现时，需要将栈顶元素弹出
                return false;
            }

            // 清空 Lua 栈
            lua_pop(luaState, 1);
            return true;
        }

        template <typename T>
        void SetGlobal(const std::string &name, const std::string &scriptName, const T &value)
        {
            auto luaState = GetLuaState(scriptName);

            push(luaState, value);
            lua_setglobal(luaState, name.c_str());
        }

        template <typename T>
        bool GetGlobal(const std::string &name, const std::string &scriptName, T &result)
        {
            auto luaState = GetLuaState(scriptName);

            lua_getglobal(luaState, name.c_str());
            if (!lua_isuserdata(luaState, -1))
            {
                lua_pop(luaState, 1);
                spdlog::error("LuaScriptLoader: failed to get global variable '{}': Invalid global variable", name);
                return false;
            }
            result = to<T>(luaState, -1);
            lua_pop(luaState, 1);
            return true;
        }

        void InjectFunctions(const std::unordered_map<std::string, lua_CFunction> &functions);

        void LoadFunctionsFromJsonFile(const std::string &file_path);

    private:
        std::unordered_map<std::string, lua_State *> luaStates_;

        lua_State *L_;

        lua_State *GetLuaState(const std::string &scriptName);
    };
} // namespace OpenAPT


namespace OpenAPT
{
    LuaScriptLoader::LuaScriptLoader()
    {
        // 打开 Lua 库
        L_ = luaL_newstate();
        luaL_openlibs(L_);
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
        lua_settop(L_, 0);
    }

    void LuaScriptLoader::InjectFunctions(const std::unordered_map<std::string, lua_CFunction> &functions)
    {
        for (const auto &[name, func] : functions)
        {
            lua_register(L_, name.c_str(), func);
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
            if (luaL_loadstring(L_, source.c_str()) || lua_pcall(L_, 0, 0, 0))
            {
                const char *error = lua_tostring(L_, -1);
                spdlog::error("LuaScriptLoader: failed to load function '{}' from JSON file: {}", name, error);
                lua_pop(L_, 1);
            }
            else
            {
                for (const auto &[scriptName, luaState] : luaStates_)
                {
                    lua_pushvalue(L_, -1);
                    lua_setglobal(luaState, name.c_str());
                }
            }
        }
    }

    lua_State *LuaScriptLoader::GetLuaState(const std::string &scriptName)
    {
        auto iter = luaStates_.find(scriptName);
        if (iter != luaStates_.end())
        {
            return iter->second;
        }

        auto luaState = luaL_newstate();
        luaL_openlibs(luaState);

        // 一些初始化操作

        luaStates_[scriptName] = luaState;
        return luaState;
    }

    LuaScriptLoader* LuaScriptLoaderFactory::MakeLuaScriptLoader()
    {
        return new LuaScriptLoader();
    }

}