/*
 * exe_plugin.cpp
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

Date: 2023-7-13

Description: Executable Plugin

**************************************************/

#include "exe_plugin.hpp"

#include "atom/log/loguru.hpp"

#include <fstream>
#include <sstream>

ExecutablePlugin::ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager)
    : Plugin(path, version, author, description)
{
    m_ProcessManager = processManager;

    RegisterFunc("run_system_command", &ExecutablePlugin::RunSystemCommand, this);
    RegisterFunc("run_system_command_with_output", &ExecutablePlugin::RunSystemCommandOutput, this);
    RegisterFunc("run_script", &ExecutablePlugin::RunScript, this);
    RegisterFunc("run_script_with_output", &ExecutablePlugin::RunScriptOutput, this);
}

void ExecutablePlugin::RunSystemCommand(const json &m_params)
{
    std::string command = m_params["command"].get<std::string>();
    DLOG_F(INFO, "Running command: {}", command);
    if (m_ProcessManager)
    {
        if (!m_ProcessManager->createProcess(command, GetPath()))
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

void ExecutablePlugin::RunSystemCommandOutput(const json &m_params)
{
    if (m_ProcessManager)
    {
        std::string command = m_params["command"].get<std::string>();
        DLOG_F(INFO, "Running command: {}", command);
        if (m_ProcessManager->createProcess(command, GetPath()))
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

void ExecutablePlugin::RunScript(const json &m_params)
{
    if (m_ProcessManager)
    {
        std::string script = m_params["script"].get<std::string>();
        DLOG_F(INFO, "Running script: {}", script);
        if (m_ProcessManager->createProcess(script, GetPath()))
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

void ExecutablePlugin::RunScriptOutput(const json &m_params)
{
    if (m_ProcessManager)
    {
        std::string script = m_params["script"].get<std::string>();
        DLOG_F(INFO, "Running script: {}", script);
        if (m_ProcessManager->createProcess(script, GetPath()))
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
