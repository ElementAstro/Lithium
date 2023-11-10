/*
 * reflection.cpp
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

Date: 2023-11-10

Description: A tiny reflection library

**************************************************/

#include "reflection.hpp"

#include <sstream>

namespace Utilities
{
    void *Reflection::createInstance(const char *name_str)
    {
        // 检查key即name_str是否存在
        if (this->func_map.count(name_str) == 0)
        {
            // 不存在抛出异常
            std::stringstream ss;
            ss << "The type you create was not registered!: " << name_str;
            throw Utilities::NotFound_Error(ss.str().c_str());
        }
        return this->func_map[name_str](); // 调用对应的回调函数
    }

    void *Reflection::createInstance(const std::string &name_str)
    {
        // 检查key即name_str是否存在
        if (this->func_map.count(name_str) == 0)
        {
            // 不存在抛出异常
            std::stringstream ss;
            ss << "The type you create was not registered!: " << name_str;
            throw Utilities::NotFound_Error(ss.str().c_str());
        }
        return this->func_map[name_str](); // 调用对应的回调函数
    }
}