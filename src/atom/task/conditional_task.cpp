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

ConditionalTask::ConditionalTask(const std::function<json(const json &)> &func,
                   const std::function<json(const json &)> &stop_fn, 
                   const json &params_template, 
                    const std::function<bool(const json &)> &condition_fn,
                    bool isForce = false)
    : SimpleTask(func, stop_fn, params_template), m_conditionFunc(condition_fn), m_isForce(isForce){}

// Executes the task
const json ConditionalTask::execute()
{
    m_isExecuting.store(true);
    if (!m_paramsTemplate.is_null() && !m_params.is_null())
    {
        if (!validateJsonValue(m_params, m_paramsTemplate))
        {
            return {{"status", "error"}, {"error", "Incorrect value type for element:"}, {"code", 500}};
        }
    }
    if (!m_conditionFunc(m_params))
    {
        return {{"status", "error"}, {"error", "Condition not met"}, {"code", 400}};
    }
    if (!m_stopFlag)
    {
        m_returns = m_function(m_params);
    }
    m_isExecuting.store(false);
    return m_returns;
}

// Serializes the task to a JSON object
const json ConditionalTask::toJson() const
{
    auto json = SimpleTask::toJson();
    json["type"] = "conditional";
    return json;
}