/*
 * plugin.cpp
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

Date: 2023-8-6

Description: Basic Plugin Definition

**************************************************/

#include "plugin.hpp"

Plugin::Plugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : path_(path), version_(version), author_(author), description_(description)
{
    m_CommandDispatcher = std::make_unique<CommandDispatcher<void, json>>();
    m_VariableRegistry = std::make_unique<VariableRegistry>();

    SETVAR_STR("name", path);
    SETVAR_STR("version", version);
    SETVAR_STR("author", author);
    SETVAR_STR("description", description);
    SETVAR_STR("license", path);
}

Plugin::~Plugin()
{
    // Just for safety
    m_CommandDispatcher->RemoveAll();
    m_VariableRegistry->RemoveAll();
    m_CommandDispatcher.reset();
    m_VariableRegistry.reset();
}

std::string Plugin::GetPath() const
{
    return path_;
}

std::string Plugin::GetVersion() const
{
    return version_;
}

std::string Plugin::GetAuthor() const
{
    return author_;
}

std::string Plugin::GetDescription() const
{
    return description_;
}

bool Plugin::RunFunc(const std::string &name, const json &params)
{
    if (!m_CommandDispatcher->HasHandler(name))
    {
        return false;
    }
    m_CommandDispatcher->Dispatch(name, params);
    return true;
}

bool Plugin::RunFunc(const std::vector<std::string> &name, const std::vector<json> &params)
{
    if (name.empty())
        return false;
    if (name.size() != params.size())
        return false;
    return true;
}

json Plugin::GetFuncInfo(const std::string &name)
{
    if (m_CommandDispatcher->HasHandler(name))
    {
        return {
            {"name", name}, {"description", m_CommandDispatcher->GetFunctionDescription(name)}};
    }
    else
    {
        return json();
    }
}
json Plugin::GetPluginInfo() const
{
    return {{"author", author_}, {"version", version_}, {"description", description_}, {"license", path_}};
}