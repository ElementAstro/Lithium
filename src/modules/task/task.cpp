/*
 * task.cpp
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

Date: 2023-7-19

Description: Basic and Simple Task Definition

**************************************************/

#include "task.hpp"

namespace Lithium
{
    BasicTask::BasicTask(const std::function<nlohmann::json(const nlohmann::json &)> &stop_fn, bool can_stop)
        : stop_fn_(stop_fn), can_stop_(stop_fn != nullptr), stop_flag_(false) {}

    BasicTask::~BasicTask()
    {
        if (stop_flag_)
        {
            Stop();
        }
    }

    // Serializes the task to a JSON object
    const nlohmann::json BasicTask::ToJson() const
    {
        return {
            {"type", "basic"},
            {"name", name_},
            {"id", id_},
            {"description", description_},
            {"can_stop", can_stop_}};
    }

    // Accessor and mutator for the task ID
    int BasicTask::get_id() const { return id_; }
    void BasicTask::set_id(int id) { id_ = id; }

    // Accessor and mutator for the task name
    const std::string &BasicTask::get_name() const { return name_; }
    void BasicTask::set_name(const std::string &name) { name_ = name; }

    const std::string &BasicTask::get_description() const { return description_; }
    void BasicTask::set_description(const std::string &description) { description_ = description; }

    void BasicTask::set_can_execute(bool can_execute) { can_execute_ = can_execute; }
    bool BasicTask::can_execute() const { return can_execute_; }

    // Set the stop function
    void BasicTask::set_stop_function(const std::function<nlohmann::json(const nlohmann::json &)> &stop_fn)
    {
        stop_fn_ = stop_fn;
        can_stop_ = true;
    }

    // Accessor and mutator for the stop flag
    bool BasicTask::get_stop_flag() const { return stop_flag_; }
    void BasicTask::set_stop_flag(bool flag) { stop_flag_ = flag; }

    // Stops the task
    void BasicTask::Stop()
    {
        stop_flag_ = true;
        if (stop_fn_)
        {
            stop_fn_({});
        }
    }

    bool BasicTask::validateJsonValue(const nlohmann::json &data, const nlohmann::json &templateValue)
    {
        if (data.type() != templateValue.type())
        {
            if (templateValue.empty())
            {
                return false;
            }
        }
        if (data.is_object())
        {
            for (auto it = templateValue.begin(); it != templateValue.end(); ++it)
            {
                const std::string &key = it.key();
                const auto &subTemplateValue = it.value();
                if (!validateJsonValue(data.value(key, nlohmann::json()), subTemplateValue))
                {
                    return false;
                }
            }
        }
        else if (data.is_array())
        {
            if (templateValue.size() > 0 && data.size() != templateValue.size())
            {
                return false;
            }

            for (size_t i = 0; i < data.size(); ++i)
            {
                if (!validateJsonValue(data[i], templateValue[0]))
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool BasicTask::validateJsonString(const std::string &jsonString, const std::string &templateString)
    {
        nlohmann::json jsonData;
        nlohmann::json templateData;
        try
        {
            jsonData = nlohmann::json::parse(jsonString);
            templateData = nlohmann::json::parse(templateString);
        }
        catch (const std::exception &e)
        {
            return false;
        }
        return validateJsonValue(jsonData, templateData);
    }

    SimpleTask::SimpleTask(const std::function<nlohmann::json(const nlohmann::json &)> &func,
                           const nlohmann::json &params_template,
                           const std::function<nlohmann::json(const nlohmann::json &)> &stop_fn, bool can_stop)
        : function_(func), params_template_(params_template), BasicTask(stop_fn, can_stop) {}

    nlohmann::json SimpleTask::Execute()
    {
        if (!params_template_.is_null() && !params_.is_null())
        {
            if (!validateJsonValue(params_, params_template_))
            {
                return {"error", "Incorrect value type for element:"};
            }
        }
        if (!stop_flag_)
        {
            returns_ = function_(params_);
        }
        done_ = true;
        return ToJson();
    }

    void SimpleTask::SetParams(const nlohmann::json &params)
    {
        params_ = params;
    }

    const nlohmann::json SimpleTask::ToJson() const
    {
        auto j = BasicTask::ToJson();
        j["type"] = "simple";
        j["params"] = params_;
        return j;
    }

    const nlohmann::json SimpleTask::GetResult() const
    {
        return returns_;
    }

    const nlohmann::json SimpleTask::GetParamsTemplate() const
    {
        return params_template_;
    }
}