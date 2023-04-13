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

#ifndef LUA_SCRIPT_LOADER_H
#define LUA_SCRIPT_LOADER_H

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

            int nArgs = sizeof...(args);
            int nResults = 1; // 默认返回值个数为1

            // 压入函数参数
            int index = 1;
            (push(luaState, std::forward<Args>(args)), ...);

            // 调用函数
            if (lua_pcall(luaState, nArgs, nResults, 0) != 0)
            {
                const char *error = lua_tostring(luaState, -1);
                spdlog::error("LuaScriptLoader: failed to call function '{}': {}", name, error);
                lua_pop(luaState, 1); // 异常出现时，需要将栈顶元素弹出
                return false;
            }

            // 获取返回值
            result = to<T>(luaState, -1);
            lua_pop(luaState, 1); // 弹出返回值
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
        std::unordered_map<std::string, lua_State *> luaStates_{};

        lua_State *GetLuaState(const std::string &scriptName);
    };
} // namespace OpenAPT

#endif
