/*
 * component.cpp
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

Description: Basic Component Definition

**************************************************/

#include "component.hpp"

#include "component_info.hpp"
#include "atom/utils/exception.hpp"

Component::Component()
{
    // Just for safety, initialize the members
    m_CommandDispatcher = std::make_unique<CommandDispatcher<void, json>>();
    m_VariableRegistry = std::make_unique<VariableRegistry>();

    // Load the package info from package.json
    // Other name is also supported but not recommended

    m_PackageInfo = std::make_shared<PackageInfo>("package.json");

    // Should we catch the exception here?
    // The package info should be loaded successfully, so it should not be empty
    m_PackageInfo->loadPackageJson();

    // This is a little bit hacky, but it works
    m_VariableRegistry->RegisterVariable<std::string>("version", "the version of the component");
    m_VariableRegistry->SetVariable("version", m_PackageInfo->getVersion());
    m_VariableRegistry->RegisterVariable<std::string>("author", "the author of the component");
    m_VariableRegistry->SetVariable("author", m_PackageInfo->getPackageJson().at("author").get<std::string>());
    m_VariableRegistry->RegisterVariable<std::string>("description", "the description of the component");
    m_VariableRegistry->SetVariable("description", m_PackageInfo->getPackageJson().at("description").get<std::string>());
    m_VariableRegistry->RegisterVariable<std::string>("repository", "the repository of the component");
    m_VariableRegistry->SetVariable("repository", m_PackageInfo->getPackageJson().at("repository").value("url", ""));
    m_VariableRegistry->RegisterVariable<std::string>("homepage", "the homepage of the component");
    m_VariableRegistry->SetVariable("homepage", m_PackageInfo->getPackageJson().at("homepage").value("url", ""));
    
}

Component::~Component()
{
    // Just for safety
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
        return {
            {"name", name}, {"description", m_CommandDispatcher->GetFunctionDescription(name)}};
    }
    else
    {
        return json();
    }
}
json Component::GetComponentInfo() const
{
    if (m_PackageInfo->isLoaded())
    {
        return m_PackageInfo->getPackageJson();
    }
    return {};
}

std::string Component::GetName() const
{
    return m_PackageInfo->getName();
}

std::string Component::GetVersion() const
{
    return m_PackageInfo->getVersion();
}