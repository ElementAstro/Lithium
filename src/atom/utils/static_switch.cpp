/*
 * static_switch.cpp
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

Date: 2023-10-27

Description: Smart Static Switch just like javascript

**************************************************/

#include "static_switch.hpp"

namespace Atom::Utils
{
    void StringSwitch::registerCase(const std::string &str, Func func)
    {
        cases()[str] = std::move(func);
    }

    void StringSwitch::setDefault(DefaultFunc func)
    {
        defaultFunc = std::move(func);
    }

#ifdef ENABLE_FASTHASH
    emhash8::HashMap<std::string, StringSwitch::Func> &StringSwitch::cases()
#else
    std::unordered_map<std::string, StringSwitch::Func> &StringSwitch::cases()
#endif
    {
#ifdef ENABLE_FASTHASH
        static emhash8::HashMap<std::string, Func> cases;
#else
        static std::unordered_map<std::string, Func> cases;
#endif
        return cases;
    }

    StringSwitch::DefaultFunc StringSwitch::defaultFunc;
}

/*

int main()
{
    StringSwitch::registerCase("hello", [](int i)
                               { std::cout << "Hello " << i << std::endl; });

    StringSwitch::setDefault([]()
                             { std::cout << "Default case" << std::endl; });

    StringSwitch::match("hello", 42);
    StringSwitch::match("world");
}

*/