/*
 * component_info.cpp
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

Description: Basic Plugin Infomation

**************************************************/

#include "component_info.hpp"

#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/exception.hpp"

ComponentInfo::ComponentInfo(const std::string &filename) : filename_(filename) {}

ComponentInfo::~ComponentInfo()
{
    if (need_save_)
    {
        savePackageJson();
    }
}

void ComponentInfo::loadPackageJson()
{
    std::ifstream file(filename_);
    if (file.is_open())
    {
        json j;
        file >> j;
        file.close();
        package_ = j;
        is_loaded_ = true;
        DLOG_F(INFO, "Loaded {}", filename_);
    }
    else
    {
        LOG_F(ERROR, "Failed to open file {}", filename_);
        throw Atom::Utils::Exception::FileNotReadable_Error("package.json file not readable");
    }
}

void ComponentInfo::savePackageJson()
{
    std::ofstream file(filename_);
    if (file.is_open())
    {
        json j = package_;
        file << j.dump(4);
        file.close();
        LOG_F(INFO, "Saved {}", filename_);
    }
    else
    {
        LOG_F(ERROR, "Failed to open file {}", filename_);
    }
}

bool ComponentInfo::isLoaded() const
{
    return is_loaded_;
}

json ComponentInfo::getPackageJson() const
{
    return package_;
}

_ComponentInfo ComponentInfo::toStruct() const
{    
    _ComponentInfo result;
    result.m_name = package_["name"].get<std::string>();
    result.m_version = package_["version"].get<std::string>();
    result.m_type = package_["type"].get<std::string>();
    result.description = package_["description"].get<std::string>();
    result.author = package_["author"].get<std::string>();
    result.license = package_["license"].get<std::string>();

    result.m_repository_url = package_["repository"].value("url", "");
    result.m_repository_type = package_["repository"].value("type", "");
    result.homepage = package_["homepage"].value("url", "");
    result.bugs = package_["bugs"].value("url", "");
    result.keywords = package_["keywords"].get<std::vector<std::string>>();

    result.scripts["dev"] = package_["scripts"].value("dev", "");
    result.scripts["build"] = package_["scripts"].value("build", "");
    result.scripts["start"] = package_["scripts"].value("start", "");
    result.scripts["lint"] = package_["scripts"].value("lint", "");

    for (const auto &dep : package_["dependencies"].items())
    {
        result.dependencies.regular[dep.key()] = dep.value().get<std::string>();
    }

    for (const auto &main : package_["main"].items())
    {
        if (main.contains("func"))
        {
            _ComponentMain m;
            m.m_component_name = main.key();
            m.m_func_name = main["func"].get<std::string>();
            m.m_component_type = main["type"].get<std::string>();
            result.main[main.key()] = m;
        }
    }
    return result;
}
