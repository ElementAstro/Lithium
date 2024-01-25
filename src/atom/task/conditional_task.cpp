/*
 * conditional_task.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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