/*
 * task.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Basic and Simple Task Definition

**************************************************/

#include "task.hpp"

SimpleTask::SimpleTask(const std::function<json(const json &)> &func,
               const std::function<json(const json &)> &stop_fn,
               const json &params_template)
    : m_function(func), m_paramsTemplate(params_template), m_stopFn(stop_fn), m_stopFlag(false)
{
    if (stop_fn)
    {
        m_canStop = true;
    }
}

SimpleTask::~SimpleTask()
{
    if (m_stopFlag)
    {
        stop();
    }
}

json SimpleTask::toJson()
{
    return {
        {"type", "merged"},
        {"name", m_name},
        {"id", m_id},
        {"description", m_description},
        {"can_stop", m_canStop}};
}

json SimpleTask::getResult()
{
    return m_returns;
}

json SimpleTask::getParamsTemplate()
{
    return m_paramsTemplate;
}

void SimpleTask::setParams(const json &params)
{
    m_params = params;
}

int SimpleTask::getId() const
{
    return m_id;
}

void SimpleTask::setId(int id)
{
    m_id = id;
}

const std::string &SimpleTask::getName() const
{
    return m_name;
}

void SimpleTask::setName(const std::string &name)
{
    m_name = name;
}

const std::string &SimpleTask::getDescription() const
{
    return m_description;
}

void SimpleTask::setDescription(const std::string &description)
{
    m_description = description;
}

void SimpleTask::setCanExecute(bool can_execute)
{
    m_canExecute = can_execute;
}

bool SimpleTask::isExecutable() const
{
    return m_canExecute;
}

void SimpleTask::setStopFunction(const std::function<json(const json &)> &stop_fn)
{
    m_stopFn = stop_fn;
    m_canStop = true;
}

bool SimpleTask::getStopFlag() const
{
    return m_stopFlag;
}

void SimpleTask::setStopFlag(bool flag)
{
    m_stopFlag = flag;
}

void SimpleTask::stop()
{
    m_stopFlag = true;
    if (m_stopFn)
    {
        m_stopFn({});
    }
}

bool SimpleTask::validateJsonValue(const json &data, const json &templateValue)
{
    if (data.type() != templateValue.type())
    {
        if (templateValue.empty())
        {
            return false;
        }
        if (templateValue.is_object() && !data.is_object())
        {
            return false;
        }
        if (templateValue.is_array() && !data.is_array())
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
            if (!data.contains(key) || !validateJsonValue(data[key], subTemplateValue))
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

bool SimpleTask::validateJsonString(const std::string &jsonString, const std::string &templateString)
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

json SimpleTask::execute()
{
    m_isExecuting.store(true);
    if (!m_paramsTemplate.is_null() && !m_params.is_null())
    {
        if (!validateJsonValue(m_params, m_paramsTemplate))
        {
            return {{"status", "error"}, {"error", "Incorrect value type for element:"}, {"code", 500}};
        }
    }
    if (!m_stopFlag)
    {
        m_returns = m_function(m_params);
    }
    else
    {
        m_returns = {{"status", "error"}, {"error", "Task has been stopped"}, {"code", 500}};
    }
    m_isExecuting.store(false);
    return m_returns;
}
