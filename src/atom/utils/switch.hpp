/*
 * switch.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-10-27

Description: Smart Switch just like javascript

**************************************************/

#ifndef ATOM_UTILS_SWITCH_HPP
#define ATOM_UTILS_SWITCH_HPP

#include <functional>
#include <string>
#include <optional>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"

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
        using DefaultFunc = std::optional<Func>;   // Optional default function

        /**
         * @brief Registers a case with the given string and function.
         *
         * @param str The string to match against.
         * @param func The function to call if the string matches.
         */
        void registerCase(const std::string &str, Func func);

        /**
         * @brief Unregisters a case with the given string.
         *
         * @param str The string to match against.
         */
        void unregisterCase(const std::string &str);

        /**
         * @brief Clears all registered cases.
         */
        void clearCases();

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

        /**
         * @brief Returns a vector of all registered cases.
         *
         * @return A vector of all registered cases.
         */
        std::vector<std::string> getCases() const;

    private:
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, Func> cases_;
#else
        std::unordered_map<std::string, Func> cases_; /**< The map of registered cases. */
#endif
        DefaultFunc defaultFunc_; /**< The default function to call if no match is found. */
    };

    template <typename... Args>
    void StringSwitch<Args...>::registerCase(const std::string &str, Func func)
    {
        if (cases_.find(str) != cases_.end())
        {
            throw Error::ObjectAlreadyExist("Case already registered");
        }
        cases_[str] = func;
    }

    template <typename... Args>
    void StringSwitch<Args...>::unregisterCase(const std::string &str)
    {
        cases_.erase(str);
    }

    template <typename... Args>
    void StringSwitch<Args...>::clearCases()
    {
        cases_.clear();
    }

    template <typename... Args>
    bool StringSwitch<Args...>::match(const std::string &str, Args... args)
    {
        auto iter = cases_.find(str);
        if (iter != cases_.end())
        {
            std::invoke(iter->second, args...);
            return true;
        }

        if constexpr (!std::is_void_v<DefaultFunc>)
        {
            std::invoke(defaultFunc_.value(), args...);
            return true;
        }

        return false;
    }

    template <typename... Args>
    void StringSwitch<Args...>::setDefault(DefaultFunc func)
    {
        defaultFunc_ = func;
    }

    template <typename... Args>
    std::vector<std::string> StringSwitch<Args...>::getCases() const
    {
        std::vector<std::string> caseList;
        for (const auto &entry : cases_)
        {
            caseList.push_back(entry.first);
        }
        return caseList;
    }

}

#endif