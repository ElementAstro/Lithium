/*
 * define.hpp
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

Date: 2023-3-28

Description: Define All of the Tasks

**************************************************/

#ifndef _TASK_DEFINE_HPP_
#define _TASK_DEFINE_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <thread>
#include <cstddef>
#include <cstdint>

#include "nlohmann/json.hpp"
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace OpenAPT
{
class BasicTask {
public:
    // Constructor
    BasicTask(const std::function<void()>& stopFunc = nullptr, bool stopFlag = false)
        : m_stopFunc(stopFunc), m_canStop(stopFunc != nullptr), m_stopFlag(stopFlag) {}

    // Executes the task
    virtual nlohmann::json execute() = 0;

    // Serializes the task to a JSON object
    virtual nlohmann::json toJson() const {
        return {
            {"type", "basic"},
            {"name", m_name},
            {"id", m_id}
        };
    };

    // Accessor and mutator for the task ID
    int getId() const { return m_id; }
    void setId(int id) { m_id = id; }

    // Accessor and mutator for the task name
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    const std::string &getDescription() const { return m_description; }
    void setDescription(const std::string &description) { m_description = description; }

    void setCanExecute(bool canExecute) { m_canExecute = canExecute; }
    bool canExecute() const { return m_canExecute; }

    // Set the stop function
    void setStopFunction(const std::function<void()>& stopFunc) {
        m_stopFunc = stopFunc;
        m_canStop = true;
    }

    // Accessor and mutator for the stop flag
    bool getStopFlag() const { return m_stopFlag; }
    void setStopFlag(bool flag) { m_stopFlag = flag; }
    
    // Stops the task
    virtual void stop() {
        m_stopFlag = true;
        if (m_stopFunc) {
            m_stopFunc();
        }
    }
    
protected:
    // True if the task is completed
    bool m_done = false;

    // Task ID
    int m_id;

    // Task name
    std::string m_name;

    std::string m_description;

    // True if the task can be stopped
    bool m_canStop;

    // Stop function
    std::function<void()> m_stopFunc;

    // Stop flag
    bool m_stopFlag = false;

    bool m_canExecute = true;
};


// Conditional task that executes a function based on a condition
class ConditionalTask : public BasicTask {
public:
    // Constructor
    ConditionalTask(const std::function<void(const nlohmann::json&)>& func,
        const nlohmann::json& params,
        const std::function<bool(const nlohmann::json&)>& condition,
        const std::function<void()>& stopFunc = nullptr, bool stopFlag = false)
        : m_func(func), m_params(params), m_condition(condition), BasicTask(stopFunc, stopFlag) {}

    // Executes the task
    nlohmann::json execute() override {
        if (!m_stopFlag && m_condition(m_params)) {
            m_func(m_params);
        }
        m_done = true;
        return toJson();
    }

    // Serializes the task to a JSON object
    nlohmann::json toJson() const override {
        auto j = BasicTask::toJson();
        j["type"] = "conditional";
        j["condition"] = m_params;
        return j;
    }

private:
    // Function to execute
    std::function<void(const nlohmann::json&)> m_func;

    // Parameters passed to the function
    nlohmann::json m_params;

    // Condition to check before executing the function
    std::function<bool(const nlohmann::json&)> m_condition;
};

// Loop task that executes a function for each item in a list
class LoopTask : public BasicTask {
public:
    // Constructor
    LoopTask(const std::function<void(const nlohmann::json&)>& func, const nlohmann::json& params,
        const std::function<void()>& stopFunc = nullptr, bool stopFlag = false)
        : m_func(func), m_params(params), BasicTask(stopFunc, stopFlag) {}

    // Executes the task
    nlohmann::json execute() override {
        for (int i = m_progress; i < m_params["total"].get<int>(); ++i) {
            if (m_stopFlag) {
                break;
            }
            m_func(m_params["items"][i]);
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate task execution time
            m_progress = i + 1;
        }
        m_done = true;
        return toJson();
    }

    // Serializes the task to a JSON object
    nlohmann::json toJson() const override {
        auto j = BasicTask::toJson();
        j["type"] = "loop";
        j["params"] = m_params;
        j["progress"] = m_progress;
        return j;
    }

private:
    // Function to execute for each item
    std::function<void(const nlohmann::json&)> m_func;

    // List of items to loop over
    nlohmann::json m_params;

    // Current progress through the loop
    int m_progress = 0;
};

// Simple task that executes a function with parameters
class SimpleTask : public BasicTask {
public:
    // Constructor
    SimpleTask(const std::function<void(const nlohmann::json&)>& func, const nlohmann::json& params,
        const std::function<void()>& stopFunc = nullptr, bool stopFlag = false)
        : m_func(func), m_params(params), BasicTask(stopFunc, stopFlag) {}

    // Executes the task
    nlohmann::json execute() override {
        if (!m_stopFlag) {
            m_func(m_params);
        }
        m_done = true;
        return toJson();
    }

    // Serializes the task to a JSON object
    nlohmann::json toJson() const override {
        auto j = BasicTask::toJson();
        j["type"] = "simple";
        j["params"] = m_params;
        return j;
    }

private:
    // Function to execute
    std::function<void(const nlohmann::json&)> m_func;

    // Parameters passed to the function
    nlohmann::json m_params;
};

    using hash_t = std::uint64_t;
    constexpr hash_t basis{0xcbf29ce484222325};
    constexpr hash_t prime{0x100000001b3};

    constexpr hash_t hash_compile_time(char const *str, hash_t last_value = basis)
    {
        return (*str) ? hash_compile_time(str + 1, (*str ^ last_value) * prime) : last_value;
    }

    constexpr hash_t hash_(char const *str)
    {
        hash_t ret{basis};
        while (*str)
        {
            ret ^= *str;
            ret *= prime;
            str++;
        }
        return ret;
    }

    constexpr unsigned long long operator""_hash(char const *p, std::size_t) noexcept
    {
        return hash_compile_time(p);
    }

}
#endif