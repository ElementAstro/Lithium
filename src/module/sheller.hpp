/*
 * sheller.hpp
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

Date: 2023-4-9

Description: Shell Manager

**************************************************/

#ifndef SHELLER_HPP
#define SHELLER_HPP

#include <iostream>
#include <vector>
#include <filesystem>
#include <sstream>

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

namespace OpenAPT
{
    enum class ScriptType
    {
        Sh,
        Ps
    };

    class ScriptManager
    {
    public:
        ScriptManager(const std::string &path);
        bool runScript(const std::string &scriptName, bool async = false) const;

    private:
        std::vector<std::string> m_files;
        json m_scriptsJson;
        std::string m_path = "scripts";

        std::vector<std::string> getScriptFiles() const;
        std::string readScriptFromFile(const std::string &path) const;
        bool validateScript(const std::string &script, ScriptType scriptType) const;
        ScriptType getScriptType(const std::string &path) const;
        json getScriptsJson(const std::vector<std::string> &files) const;
        std::string buildCommand(const std::string &scriptPath) const;
        std::string executeCommand(const std::string &command) const;
    };
}

#endif // SHELLER_HPP
