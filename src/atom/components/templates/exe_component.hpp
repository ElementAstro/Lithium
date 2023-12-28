/*
 * exe_plugin.hpp
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

#pragma once

#include "atom/components/component.hpp"

#include "atom/system/process.hpp"

class ExecutablePlugin : public C
{
public:
    ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager);

    void RunSystemCommand(const json &m_parmas);

    void RunSystemCommandOutput(const json &m_parmas);

    void RunScript(const json &m_parmas);

    void RunScriptOutput(const json &m_parmas);
private:
    std::shared_ptr<Lithium::Process::ProcessManager> m_ProcessManager;
};
