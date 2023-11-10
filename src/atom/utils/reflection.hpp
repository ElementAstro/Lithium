/*
 * reflection.hpp
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

#ifndef REFLECTION_H
#define REFLECTION_H

#include <functional>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "exception.hpp"

// 变量初始化宏
#define INIT                       \
    std::function<void *(void)> f; \
    std::string str_name;

// lambda表达式宏，用于创建类型className的构造回调函数，并通过包装器将其插入到map中

#define REGISTER(className)   \
    f = []() {                \
        return new className; \
    };                        \
    str_name = #className;    \
    this->func_map.insert(pair<std::string, std::function<void *(void)>>(str_name, f));

#define REGISTER_BY_OTHERNAME(className, regist_name) \
    f = []() {                                        \
        return new className;                         \
    };                                                \
    str_name = #regist_name;                          \
    this->func_map.insert(pair<std::string, std::function<void *(void)>>(str_name, f));

namespace Utilities
{
    class Reflection;
};

namespace Utilities
{
    // 抽象类，需要用户去派生
    class Reflection
    {
    public:
        Reflection(){};
        virtual void load() = 0;
        virtual void *createInstance(const char *name_str);
        virtual void *createInstance(const std::string &name_str);

    protected:
        std::unordered_map<std::string, std::function<void *(void)>> func_map;
    };
}
#endif