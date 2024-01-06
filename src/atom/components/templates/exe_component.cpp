/*
 * exe_plugin.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-7-13

Description: Executable Plugin

**************************************************/

#include "exe_component.hpp"
#include "component_marco.hpp"

#include "atom/log/loguru.hpp"
#include "atom/utils/random.hpp"

#include <fstream>
#include <sstream>

ExecutableComponent::ExecutableComponent()
    : Component()
{

    RegisterFunc("run_system_command", &ExecutableComponent::RunSystemCommand, this);
    RegisterFunc("run_system_command_with_output", &ExecutableComponent::RunSystemCommandOutput, this);
    RegisterFunc("run_script", &ExecutableComponent::RunScript, this);
    RegisterFunc("run_script_with_output", &ExecutableComponent::RunScriptOutput, this);
}

void ExecutableComponent::RunSystemCommand(const Args &m_params)
{
    GET_COMPONENT_ARG(command,std::string);
    GET_COMPONENT_ARG(identifier,std::string);
    DLOG_F(INFO, "Running command: {}", command);
    if (m_ProcessManager)
    {
        if (!m_ProcessManager->createProcess(command, identifier.empty() ? Atom::Utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", command);
        }
        else
        {
            LOG_F(ERROR, "Started {} successfully", command);
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}

void ExecutableComponent::RunSystemCommandOutput(const Args &m_params)
{
    GET_COMPONENT_ARG(command,std::string);
    GET_COMPONENT_ARG(identifier,std::string);
    if (m_ProcessManager)
    {
        DLOG_F(INFO, "Running command: {}", command);
        if (m_ProcessManager->createProcess(command, identifier.empty() ? Atom::Utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Started {} successfully", command);
        }
        else
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", command);
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}

void ExecutableComponent::RunScript(const Args &m_params)
{
    GET_COMPONENT_ARG(script,std::string);
    GET_COMPONENT_ARG(identifier,std::string);
    if (m_ProcessManager)
    {
        DLOG_F(INFO, "Running script: {}", script);
        if (m_ProcessManager->createProcess(script, identifier.empty() ? Atom::Utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Started {} successfully", script);
        }
        else
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", script);
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}

void ExecutableComponent::RunScriptOutput(const Args &m_params)
{
    GET_COMPONENT_ARG(script,std::string);
    GET_COMPONENT_ARG(identifier,std::string);
    if (m_ProcessManager)
    {
        DLOG_F(INFO, "Running script: {}", script);
        if (m_ProcessManager->createProcess(script, identifier.empty() ? Atom::Utils::generateRandomString(10) : identifier))
        {
            LOG_F(ERROR, "Started {} successfully", script);
        }
        else
        {
            LOG_F(ERROR, "Failed to run executable plugin : {}", script);
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}
