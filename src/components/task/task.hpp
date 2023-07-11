/*
 * task.hpp
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

Date: 2023-7-1

Description: Task Definition

**************************************************/

#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <thread>
#include <cstddef>
#include <cstdint>

#include "nlohmann/json.hpp"

namespace OpenAPT
{
    class BasicTask
    {
    public:
        // Constructor
        BasicTask(const std::function<void()> &stop_fn = nullptr, bool can_stop = false)
            : stop_fn_(stop_fn), can_stop_(stop_fn != nullptr), stop_flag_(false) {}

        // Executes the task
        virtual nlohmann::json Execute() = 0;

        // Serializes the task to a JSON object
        virtual nlohmann::json ToJson() const
        {
            return {
                {"type", "basic"},
                {"name", name_},
                {"id", id_},
                {"description", description_},
                {"can_stop", can_stop_}};
        };

        // Accessor and mutator for the task ID
        int get_id() const { return id_; }
        void set_id(int id) { id_ = id; }

        // Accessor and mutator for the task name
        const std::string &get_name() const { return name_; }
        void set_name(const std::string &name) { name_ = name; }

        const std::string &get_description() const { return description_; }
        void set_description(const std::string &description) { description_ = description; }

        void set_can_execute(bool can_execute) { can_execute_ = can_execute; }
        bool can_execute() const { return can_execute_; }

        // Set the stop function
        void set_stop_function(const std::function<void()> &stop_fn)
        {
            stop_fn_ = stop_fn;
            can_stop_ = true;
        }

        // Accessor and mutator for the stop flag
        bool get_stop_flag() const { return stop_flag_; }
        void set_stop_flag(bool flag) { stop_flag_ = flag; }

        // Stops the task
        virtual void Stop()
        {
            stop_flag_ = true;
            if (stop_fn_)
            {
                stop_fn_();
            }
        }

    protected:
        // True if the task is completed
        bool done_ = false;

        // Task ID
        int id_;

        // Task name
        std::string name_;

        std::string description_;

        // True if the task can be stopped
        bool can_stop_;

        // Stop function
        std::function<void()> stop_fn_;

        // Stop flag
        bool stop_flag_ = false;

        bool can_execute_ = true;
    };

    class SimpleTask : public BasicTask
    {
    public:
        // Constructor
        SimpleTask(const std::function<nlohmann::json(const nlohmann::json &)> &func, const nlohmann::json &params,
                   const std::function<void()> &stop_fn = nullptr, bool can_stop = false)
            : function_(func), params_(params), BasicTask(stop_fn, can_stop) {}

        // Executes the task
        virtual nlohmann::json Execute() override
        {
            if (!stop_flag_)
            {
                returns_ = function_(params_);
            }
            done_ = true;
            return ToJson();
        }

        // Serializes the task to a JSON object
        virtual nlohmann::json ToJson() const override
        {
            auto j = BasicTask::ToJson();
            j["type"] = "simple";
            j["params"] = params_;
            return j;
        }

        virtual nlohmann::json GetResult() const
        {
            return returns_;
        }

    private:
        // Function to execute
        std::function<nlohmann::json(const nlohmann::json &)> function_;

        // Parameters passed to the function
        nlohmann::json params_;

        // The result of the function
        nlohmann::json returns_;
    };

    class ConditionalTask : public BasicTask
    {
    public:
        // Constructor
        ConditionalTask(const std::function<bool(const nlohmann::json &)> &condition_fn,
                        const nlohmann::json &params,
                        const std::function<void(const nlohmann::json &)> &task_fn,
                        const std::function<void()> &stop_fn = nullptr)
            : BasicTask(stop_fn, stop_fn != nullptr), condition_fn_(condition_fn), params_(params), task_fn_(task_fn) {}

        // Executes the task
        nlohmann::json Execute() override
        {
            if (condition_fn_(params_))
            {
                task_fn_(params_);
            }
            done_ = true;
            return {{"status", "done"}};
        }

        // Serializes the task to a JSON object
        nlohmann::json ToJson() const override
        {
            auto json = BasicTask::ToJson();
            json["type"] = "conditional";
            json["params"] = params_;
            return json;
        }

    private:
        std::function<bool(const nlohmann::json &)> condition_fn_;
        nlohmann::json params_;
        std::function<void(const nlohmann::json &)> task_fn_;
    };

    class LoopTask : public BasicTask
    {
    public:
        // Constructor
        LoopTask(const std::function<void(const nlohmann::json &)> &item_fn,
                 const nlohmann::json &params,
                 const std::function<void()> &stop_fn = nullptr)
            : BasicTask(stop_fn, stop_fn != nullptr), item_fn_(item_fn), params_(params) {}

        // Executes the task
        nlohmann::json Execute() override
        {
            for (const auto &item : params_["items"])
            {
                if (stop_flag_)
                {
                    break;
                }
                item_fn_({{"item", item}});
            }
            done_ = true;
            return {{"status", "done"}};
        }

        // Serializes the task to a JSON object
        nlohmann::json ToJson() const override
        {
            auto json = BasicTask::ToJson();
            json["type"] = "loop";
            json["params"] = params_;
            return json;
        }

    private:
        std::function<void(const nlohmann::json &)> item_fn_;
        nlohmann::json params_;
    };

    class DaemonTask : public BasicTask
    {
    public:
        // Constructor
        DaemonTask(const std::function<void()> &task_fn,
                   const std::function<void()> &stop_fn = nullptr)
            : BasicTask(stop_fn, true), task_fn_(task_fn) {}

        // Executes the task
        nlohmann::json Execute() override
        {
            std::thread t(&DaemonTask::RunTask, this);
            t.detach(); // detach thread so that it runs as a daemon
            return {{"status", "running"}};
        }

        // Serializes the task to a JSON object
        nlohmann::json ToJson() const override
        {
            auto json = BasicTask::ToJson();
            json["type"] = "daemon";
            return json;
        }

    private:
        std::function<void()> task_fn_;

        // Runs the task in a loop
        void RunTask()
        {
            while (!stop_flag_)
            {
                task_fn_();
            }
            done_ = true;
        }
    };

}
