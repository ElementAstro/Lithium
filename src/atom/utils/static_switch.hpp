/*
 * static_switch.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: Smart Static Switch just like javascript (One Instance Per Process)

**************************************************/

#ifndef ATOM_UTILS_STATIC_SWITCH_HPP
#define ATOM_UTILS_STATIC_SWITCH_HPP

#include <string>
#include <functional>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

namespace Atom::Utils
{
    /**
     * @brief A class for implementing a string switch statement.
     */
    class StringSwitch
    {
    public:
        using Func = std::function<void()>;        /**< The function type for handling a case. */
        using DefaultFunc = std::function<void()>; /**< The function type for handling the default case. */

        /**
         * @brief Registers a case with the given string and function.
         *
         * @param str The string to match against.
         * @param func The function to call if the string matches.
         */
        static void registerCase(const std::string &str, Func func);

        /**
         * @brief Matches the given string against the registered cases.
         *
         * @tparam Args The types of the arguments to pass to the function.
         * @param str The string to match against.
         * @param args The arguments to pass to the function.
         * @return true if a match was found, false otherwise.
         */
        template <typename... Args>
        static bool match(const std::string &str, Args &&...args);

        /**
         * @brief Sets the default function to be called if no match is found.
         *
         * @param func The function to call for the default case.
         */
        static void setDefault(DefaultFunc func);

    private:
#if ENABLE_FASTHASH
        static emhash8::HashMap<std::string, Func> &cases();
#else
        static std::unordered_map<std::string, Func> &cases(); /**< Returns the map of registered cases. */
#endif
        static DefaultFunc defaultFunc;                        /**< The default function to call if no match is found. */
    };

    template <typename... Args>
    bool StringSwitch::match(const std::string &str, Args &&...args)
    {
        auto iter = cases().find(str);
        if (iter != cases().end())
        {
            if constexpr (sizeof...(args) > 0)
            {
                iter->second(std::forward<Args>(args)...);
            }
            return true;
        }

        if (defaultFunc)
        {
            defaultFunc();
            return true;
        }

        return false;
    }
}

#endif
