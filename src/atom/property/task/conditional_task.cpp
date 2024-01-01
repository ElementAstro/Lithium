/*
 * conditional_task.cpp
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

Description: Conditional Task Definition

**************************************************/

#include "conditional_task.hpp"

ConditionalTask::ConditionalTask(const std::function<bool(const json &)> &condition_fn,
                                 const json &params,
                                 const std::function<void(const json &)> &task_fn,
                                 std::function<json(const json &)> &stop_fn)
    : BasicTask(stop_fn, stop_fn != nullptr), condition_fn_(condition_fn), params_(params), task_fn_(task_fn) {}

// Executes the task
const json ConditionalTask::execute()
{
    if (condition_fn_(params_))
    {
        task_fn_(params_);
    }
    done_ = true;
    return {{"status", "done"}};
}

// Serializes the task to a JSON object
const json ConditionalTask::toJson() const
{
    auto json = BasicTask::toJson();
    json["type"] = "conditional";
    json["params"] = params_;
    return json;
}