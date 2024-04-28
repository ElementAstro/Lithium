/*
 * exe_plugin.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Executable Plugin

**************************************************/

#include "exe_component.hpp"
#include "atom/components/macro.hpp"

#include "atom/log/loguru.hpp"
#include "atom/utils/random.hpp"

#include <fstream>
#include <sstream>

ExecutableComponent::ExecutableComponent(const std::string &name)
    : Component(name)
{
    registerFunc("run_system_command", &ExecutableComponent::RunSystemCommand, this);
    registerFunc("run_system_command_with_output", &ExecutableComponent::RunSystemCommandOutput, this);
    registerFunc("run_script", &ExecutableComponent::RunScript, this);
    registerFunc("run_script_with_output", &ExecutableComponent::RunScriptOutput, this);
}

json ExecutableComponent::RunSystemCommand(const json &params)
{
    CHECK_PARAM("command");
    CHECK_PARAM("identifier");
    std::string command = params["command"].get<std::string>();
    std::string identifier = params["identifier"].get<std::string>();
    DLOG_F(INFO, "Running command: {}", command);
    if (m_ProcessManager)
    {
        if (!m_ProcessManager->createProcess(command, identifier.empty() ? atom::utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", command);
            return createErrorResponse(__func__, json(), std::format("Failed to run executable plugin : {}", command));
        }
        else
        {
            LOG_F(ERROR, "Started {} successfully", command);
            return createSuccessResponse(__func__, json());
        }
    }
    LOG_F(ERROR, "Process manager is not initialized");
    return createErrorResponse(__func__, json(), "Process manager is not initialized");
}

json ExecutableComponent::RunSystemCommandOutput(const json &params)
{
    CHECK_PARAM("command");
    CHECK_PARAM("identifier");
    std::string command = params["command"].get<std::string>();
    std::string identifier = params["identifier"].get<std::string>();
    if (m_ProcessManager)
    {
        DLOG_F(INFO, "Running command: {}", command);
        if (m_ProcessManager->createProcess(command, identifier.empty() ? atom::utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Started {} successfully", command);
            return createSuccessResponse(__func__, json());
        }
        else
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", command);
            return createErrorResponse(__func__, json(), std::format("Failed to run executable plugin : {}", command));
        }
    }
    LOG_F(ERROR, "Process manager is not initialized");
    return createErrorResponse(__func__, json(), "Process manager is not initialized");
}

json ExecutableComponent::RunScript(const json &params)
{
    CHECK_PARAM("script");
    CHECK_PARAM("identifier");
    std::string script = params["script"].get<std::string>();
    std::string identifier = params["identifier"].get<std::string>();
    if (m_ProcessManager)
    {
        DLOG_F(INFO, "Running script: {}", script);
        if (m_ProcessManager->createProcess(script, identifier.empty() ? atom::utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Started {} successfully", script);
            return createSuccessResponse(__func__, json());
        }
        else
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", script);
            return createErrorResponse(__func__, json(), std::format("Failed to run executable plugin : {}", script));
        }
    }
    LOG_F(ERROR, "Process manager is not initialized");
    return createErrorResponse(__func__, json(), "Process manager is not initialized");
}

json ExecutableComponent::RunScriptOutput(const json &params)
{
    CHECK_PARAM("script");
    CHECK_PARAM("identifier");
    std::string script = params["script"].get<std::string>();
    std::string identifier = params["identifier"].get<std::string>();
    if (m_ProcessManager)
    {
        DLOG_F(INFO, "Running script: {}", script);
        if (m_ProcessManager->createProcess(script, identifier.empty() ? atom::utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Started {} successfully", script);
            return createSuccessResponse(__func__, json());
        }
        else
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", script);
            return createErrorResponse(__func__, json(), std::format("Failed to run executable plugin : {}", script));
        }
    }
    LOG_F(ERROR, "Process manager is not initialized");
    return createErrorResponse(__func__, json(), "Process manager is not initialized");
}
