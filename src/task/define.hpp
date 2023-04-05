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

#include "nlohmann/json.hpp"
#include <spdlog/spdlog.h>

using json = nlohmann::json;

namespace OpenAPT {

    class BasicTask {
    public:
        // Executes the task
        virtual void execute() {}

        // Serializes the task to a JSON object
        virtual nlohmann::json toJson() { return nlohmann::json(); }

        // Returns true if the task is completed
        virtual bool isDone() const { return m_done; }

        // Accessor and mutator for the task ID
        int getId() const { return m_id; }
        void setId(int id) { m_id = id; }

        // Accessor and mutator for the task name
        const std::string& getName() const { return m_name; }
        void setName(const std::string& name) { m_name = name; }

        // Accessor and mutator for the task description
        const std::string& getDescription() const { return m_description; }
        void setDescription(const std::string& description) { m_description = description; }

        // Accessor and mutator for the task priority
        void setPriority(int priority) { m_priority = priority; }
        int getPriority() const { return m_priority; }

    protected:
        // True if the task is completed
        bool m_done = false;

        // True if the task is saved
        bool m_saved = false;

        // Task ID
        int m_id;

        // Task priority
        int m_priority = 0;

        // Task name
        std::string m_name;

        // Task description
        std::string m_description;
    };


    // Conditional task that executes a function based on a condition
    class ConditionalTask : public BasicTask {
    public:
        // Constructor
        ConditionalTask(const std::function<void()> &func,
                        const nlohmann::json &params,
                        const std::function<bool(const nlohmann::json &)> &condition)
                : m_func(func), m_params(params), m_condition(condition) {}

        // Executes the task
        void execute() override {
            if (m_condition(m_params)) {
                m_func();
            }
            m_done = true;
        }

        // Serializes the task to a JSON object
        nlohmann::json toJson() override {
            nlohmann::json j;
            j["type"] = "conditional";
            j["name"] = m_name;
            j["condition"] = m_params;
            j["priority"] = m_priority;
            return j;
        }

    private:
        // Function to execute
        std::function<void()> m_func;

        // Parameters passed to the function
        nlohmann::json m_params;

        // Condition to check before executing the function
        std::function<bool(const nlohmann::json &)> m_condition;
    };


    // Loop task that executes a function for each item in a list
    class LoopTask : public BasicTask {
    public:
        // Constructor
        LoopTask(const std::function<void(const nlohmann::json &)> &func, const nlohmann::json &params)
                : m_func(func), m_params(params) {}

        // Executes the task
        void execute() override {
            for (int i = m_progress; i < m_params["total"].get<int>(); ++i) {
                if (m_cancelled) {
                    break;
                }
                m_func(m_params["items"][i]);
                std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate task execution time
                m_progress = i + 1;
            }
            m_done = true;
        }

        // Cancels the task
        void cancel() { m_cancelled = true; }

        // Serializes the task to a JSON object
        nlohmann::json toJson() override {
            nlohmann::json j;
            j["type"] = "loop";
            j["name"] = m_name;
            j["params"] = m_params;
            j["progress"] = m_progress;
            j["priority"] = m_priority;
            return j;
        }

    private:
        // Function to execute for each item
        std::function<void(const nlohmann::json &)> m_func;

        // List of items to loop over
        nlohmann::json m_params;

        // Current progress through the loop
        int m_progress = 0;

        // True if the task is cancelled
        bool m_cancelled = false;
    };


    // Simple task that executes a function with parameters
    class SimpleTask : public BasicTask {
    public:
        // Constructor
        SimpleTask(const std::function<void(const nlohmann::json &)> &func, const nlohmann::json &params)
                : m_func(func), m_params(params) {}

        // Executes the task
        void execute() override {
            m_func(m_params);
            m_done = true;
        }

        // Serializes the task to a JSON object
        nlohmann::json toJson() override {
            nlohmann::json j;
            j["type"] = "simple";
            j["name"] = m_name;
            j["params"] = m_params;
            j["priority"] = m_priority;
            return j;
        }

    private:
        // Function to execute
        std::function<void(const nlohmann::json &)> m_func;

        // Parameters passed to the function
        nlohmann::json m_params;
    };

}
#endif