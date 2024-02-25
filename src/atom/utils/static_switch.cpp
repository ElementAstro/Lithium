/*
 * static_switch.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, StringSwitch::Func> &StringSwitch::cases()
#else
    std::unordered_map<std::string, StringSwitch::Func> &StringSwitch::cases()
#endif
    {
#if ENABLE_FASTHASH
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