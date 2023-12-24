/*
 * switch.hpp
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

Description: Smart Switch just like javascript

**************************************************/

#include <functional>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "exception.hpp"

namespace Atom::Utils
{

    /**
     * @brief A class for implementing a switch statement with string cases.
     *
     * @tparam DefaultFunc The function type for handling the default case.
     * @tparam Args The types of additional arguments to pass to the functions.
     */
    template <typename... Args>
    class StringSwitch
    {
    public:
        using Func = std::function<void(Args...)>; /**< The function type for handling a case. */
        using DefaultFunc = std::function<void(Args...)>;

        /**
         * @brief Registers a case with the given string and function.
         *
         * @param str The string to match against.
         * @param func The function to call if the string matches.
         */
        void registerCase(const std::string &str, Func func);

        /**
         * @brief Matches the given string against the registered cases.
         *
         * @param str The string to match against.
         * @param args Additional arguments to pass to the function.
         * @return true if a match was found, false otherwise.
         */
        bool match(const std::string &str, Args... args);

        /**
         * @brief Sets the default function to be called if no match is found.
         *
         * @param func The function to call for the default case.
         */
        void setDefault(DefaultFunc func);

    private:
        std::unordered_map<std::string, Func> cases_; /**< The map of registered cases. */
        DefaultFunc defaultFunc_;                     /**< The default function to call if no match is found. */
    };

    template <typename... Args>
    void StringSwitch<Args...>::registerCase(const std::string &str, Func func)
    {
        if (cases_.find(str) != cases_.end())
        {
            throw Exception::ObjectAlreadyExist_Error("Case already registered");
        }
        cases_[str] = func;
    }

    template <typename... Args>
    bool StringSwitch<Args...>::match(const std::string &str, Args... args)
    {
        auto iter = cases_.find(str);
        if (iter != cases_.end())
        {
            iter->second(args...);
            return true;
        }

        if constexpr (std::is_invocable_v<DefaultFunc, Args...>)
        {
            defaultFunc_(args...);
            return true;
        }

        return false;
    }

    template <typename... Args>
    void StringSwitch<Args...>::setDefault(DefaultFunc func)
    {
        defaultFunc_ = func;
    }
}