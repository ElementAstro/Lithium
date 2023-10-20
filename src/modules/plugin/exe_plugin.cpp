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

#include "loguru/loguru.hpp"

#include <fstream>
#include <sstream>

ExecutablePlugin::ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager)
    : Plugin(path, version, author, description)
{
    m_ProcessManager = processManager;
}

void ExecutablePlugin::Execute(const std::vector<std::string> &args) const
{
    std::ostringstream oss;
    oss << GetPath();
    for (const std::string &arg : args)
    {
        oss << " " << arg;
    }
    std::string command = oss.str();
    DLOG_F(INFO, "Running command: %s", command.c_str());
    if (m_ProcessManager)
    {
        if (!m_ProcessManager->createProcess(command, GetPath()))
        {
            LOG_F(ERROR, "Failed to run executable plugin : %s", command.c_str());
        }
        else
        {
            LOG_F(ERROR, "Started %s successfully", command.c_str());
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}
