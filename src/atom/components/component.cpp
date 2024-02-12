/*
 * component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Basic Component Definition

**************************************************/

#include "component.hpp"

#include "atom/utils/exception.hpp"

#include <format>

Component::Component()
{
    // Just for safety, initialize the members
    m_CommandDispatcher = std::make_unique<CommandDispatcher<json, json>>();
    m_VariableRegistry = std::make_unique<VariableRegistry>();
}

Component::~Component()
{

    m_CommandDispatcher->RemoveAll();
    m_VariableRegistry->RemoveAll();
    m_CommandDispatcher.reset();
    m_VariableRegistry.reset();
}

bool Component::Initialize()
{
    return true;
}

bool Component::Destroy()
{
    return true;
}

std::string Component::GetName() const
{
    return m_name;
}

bool Component::LoadConfig(const std::string &path)
{
    try
    {
        m_Config->load(path);
    }
    catch (const Atom::Utils::Exception::FileNotReadable_Error &e)
    {
        return false;
    }
    m_ConfigPath = path;
    return true;
}

std::string Component::getJsonConfig() const
{
    return m_Config->toJson();
}

std::string Component::getXmlConfig() const
{
    return m_Config->toXml();
}

std::string Component::GetVariableInfo(const std::string &name) const
{
    if (!m_VariableRegistry->HasVariable(name))
    {
        return "";
    }
    return m_VariableRegistry->GetDescription(name);
}

bool Component::RunFunc(const std::string &name, const json &params)
{
    if (!m_CommandDispatcher->HasHandler(name))
    {
        return false;
    }
    m_CommandDispatcher->Dispatch(name, params);
    return true;
}

json Component::GetFuncInfo(const std::string &name)
{
    if (m_CommandDispatcher->HasHandler(name))
    {
        json args;
        args = {
            {"name", name},
            {"description", m_CommandDispatcher->GetFunctionDescription(name)}};
        return args;
    }
    return {};
}

std::function<json(const json &)> Component::GetFunc(const std::string &name)
{
    if (!m_CommandDispatcher->HasHandler(name))
    {
        throw Atom::Utils::Exception::InvalidArgument_Error("Function not found");
    }
    return m_CommandDispatcher->GetHandler(name);
}

json Component::createSuccessResponse(const std::string &command, const json &value)
{
    json res;
    res["command"] = command;
    res["value"] = value;
    res["status"] = "ok";
    res["code"] = 200;
#if __cplusplus >= 202003L
    res["message"] = std::format("{} operated on success", command);
#else
    res["message"] = fmt::format("{} operated on success", command);
#endif
    return res;
}

json Component::createErrorResponse(const std::string &command, const json &error, const std::string &message = "")
{
    json res;
    res["command"] = command;
    res["status"] = "error";
    res["code"] = 500;
#if __cplusplus >= 202003L
    res["message"] = std::format("{} operated on failure, message: {}", command, message);
#else
    res["message"] = std::format("{} operated on failure, message: {}", command, message);
#endif
    if (!error.empty())
    {
        res["error"] = error;
    }
    else
    {
        res["error"] = "Unknown Error";
    }
    return res;
}

json Component::createWarningResponse(const std::string &command, const json &warning, const std::string &message = "")
{
    json res;
    res["command"] = command;
    res["status"] = "warning";
    res["code"] = 400;
#if __cplusplus >= 202003L
    res["message"] = std::format("{} operated on warning, message: {}", command, message);
#else
    res["message"] = std::format("{} operated on warning, message: {}", command, message);
#endif
    if (!warning.empty())
    {
        res["warning"] = warning;
    }
    else
    {
        res["warning"] = "Unknown Warning";
    }
    return res;
}