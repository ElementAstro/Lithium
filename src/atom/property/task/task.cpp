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

namespace Atom::Task
{
    BasicTask::BasicTask(const std::function<json(const json &)> &stop_fn, bool can_stop)
        : stop_fn_(stop_fn), can_stop_(stop_fn != nullptr), stop_flag_(false) {}

    BasicTask::~BasicTask()
    {
        if (stop_flag_)
        {
            stop();
        }
    }

    // Serializes the task to a JSON object
    const json BasicTask::toJson() const
    {
        return {
            {"type", "basic"},
            {"name", name_},
            {"id", id_},
            {"description", description_},
            {"can_stop", can_stop_}};
    }

    const json BasicTask::getResult() const
    {
        return {};
    }
    const json BasicTask::getParamsTemplate() const
    {
        return {};
    }
    void BasicTask::setParams(const json &params)
    {
        return;
    }

    int BasicTask::getId() const
    {
        return id_;
    }
    void BasicTask::setId(int id)
    {
        id_ = id;
    }

    const std::string &BasicTask::getName() const
    {
        return name_;
    }
    void BasicTask::setName(const std::string &name)
    {
        name_ = name;
    }

    const std::string &BasicTask::getDescription() const
    {
        return description_;
    }
    void BasicTask::setDescription(const std::string &description)
    {
        description_ = description;
    }

    void BasicTask::setCanExecute(bool can_execute)
    {
        can_execute_ = can_execute;
    }
    bool BasicTask::isExecutable() const
    {
        return can_execute_;
    }

    void BasicTask::setStopFunction(const std::function<json(const json &)> &stop_fn)
    {
        stop_fn_ = stop_fn;
        can_stop_ = true;
    }

    bool BasicTask::getStopFlag() const
    {
        return stop_flag_;
    }
    void BasicTask::setStopFlag(bool flag)
    {
        stop_flag_ = flag;
    }

    void BasicTask::stop()
    {
        stop_flag_ = true;
        if (stop_fn_)
        {
            stop_fn_({});
        }
    }

    bool BasicTask::validateJsonValue(const json &data, const json &templateValue)
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
                if (!validateJsonValue(data.value(key, json()), subTemplateValue))
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
        json jsonData;
        json templateData;
        try
        {
            jsonData = json::parse(jsonString);
            templateData = json::parse(templateString);
        }
        catch (const std::exception &e)
        {
            return false;
        }
        return validateJsonValue(jsonData, templateData);
    }

    SimpleTask::SimpleTask(const std::function<json(const json &)> &func,
                           const json &params_template,
                           const std::function<json(const json &)> &stop_fn, bool can_stop)
        : function_(func), params_template_(params_template), BasicTask(stop_fn, can_stop) {}

    const json SimpleTask::execute()
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
        return toJson();
    }

    void SimpleTask::setParams(const json &params)
    {
        params_ = params;
    }

    const json SimpleTask::toJson() const
    {
        auto j = BasicTask::toJson();
        j["type"] = "simple";
        j["params"] = params_;
        return j;
    }

    const json SimpleTask::getResult() const
    {
        return returns_;
    }

    const json SimpleTask::getParamsTemplate() const
    {
        return params_template_;
    }
}