/*
 * component.cpp
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

Date: 2023-8-6

Description: Basic Component Definition

**************************************************/

#include "component.hpp"

#include "atom/utils/exception.hpp"

Component::Component()
{
    // Just for safety, initialize the members
    m_CommandDispatcher = std::make_unique<CommandDispatcher<void, Args>>();
    m_VariableRegistry = std::make_unique<VariableRegistry>();

    m_ComponentInfo = std::make_shared<INIFile>();
    m_ComponentConfig = std::make_shared<INIFile>();
}

Component::~Component()
{
    // Save the config file
    if (!m_ConfigPath.empty())
        m_ComponentConfig->save(m_ConfigPath);
    // Save the info file
    if (!m_InfoPath.empty())
        m_ComponentInfo->save(m_InfoPath);
    // Just for safety
    m_CommandDispatcher->RemoveAll();
    m_VariableRegistry->RemoveAll();
    m_CommandDispatcher.reset();
    m_VariableRegistry.reset();
    m_ComponentInfo.reset();
    m_ComponentConfig.reset();
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

bool Component::LoadComponentInfo(const std::string &path)
{
    try
    {
        m_ComponentInfo->load(path);
    }
    catch(const Atom::Utils::Exception::FileNotReadable_Error& e)
    {
        return false;
    }
    m_InfoPath = path;
    return true;
}

std::string Component::getJsonInfo() const
{
    return m_ComponentInfo->toJson();
}

std::string Component::getXmlInfo() const
{
    return m_ComponentInfo->toXml();
}

bool Component::LoadComponentConfig(const std::string &path)
{
    try
    {
        m_ComponentConfig->load(path);
    }
    catch(const Atom::Utils::Exception::FileNotReadable_Error& e)
    {
        return false;
    }
    m_ConfigPath = path;
    return true;
}

std::string Component::getJsonConfig() const
{
    return m_ComponentConfig->toJson();
}

std::string Component::getXmlConfig() const
{
    return m_ComponentConfig->toXml();
}

std::string Component::GetVariableInfo(const std::string &name) const
{
    if (!m_VariableRegistry->HasVariable(name))
    {
        return "";
    }
    return m_VariableRegistry->GetDescription(name);
}

bool Component::RunFunc(const std::string &name, const Args &params)
{
    if (!m_CommandDispatcher->HasHandler(name))
    {
        return false;
    }
    m_CommandDispatcher->Dispatch(name, params);
    return true;
}

Args Component::GetFuncInfo(const std::string &name)
{
    if (m_CommandDispatcher->HasHandler(name))
    {
        Args args;
        args = {
            {"name", name},
            {"description", m_CommandDispatcher->GetFunctionDescription(name)}};
        return args;
    }
    return {};
}

std::function<void(const Args &)> Component::GetFunc(const std::string &name)
{
    if (!m_CommandDispatcher->HasHandler(name))
    {
        throw Atom::Utils::Exception::InvalidArgument_Error("Function not found");
    }
    return m_CommandDispatcher->GetHandler(name);
}
