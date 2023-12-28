/*
 * component_info.cpp
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

Description: Basic Plugin Infomation

**************************************************/

#include "component_info.hpp"

#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/exception.hpp"

PackageInfo::PackageInfo(const std::string &filename) : filename_(filename) {}

PackageInfo::~PackageInfo()
{
    if (need_save_)
    {
        savePackageJson();
    }
}

void PackageInfo::loadPackageJson()
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

void PackageInfo::savePackageJson()
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

bool PackageInfo::isLoaded() const
{
    return is_loaded_;
}

json PackageInfo::getPackageJson() const
{
    return package_;
}

std::string PackageInfo::getName() const
{
    return package_.at("name").get<std::string>();
}

std::string PackageInfo::getVersion() const
{
    return package_.at("version").get<std::string>();
}

bool PackageInfo::isPrivate() const
{
    return package_.at("private").get<bool>();
}

void PackageInfo::setName(const std::string &name)
{
    package_["name"] = name;
}

void PackageInfo::setVersion(const std::string &version)
{
    package_["version"] = version;
}

void PackageInfo::setIsPrivate(bool isPrivate)
{
    package_["private"] = isPrivate;
}

void PackageInfo::setMain(const std::string &main)
{
    package_["main"] = main;
}

void PackageInfo::setBin(const std::string &bin)
{
    package_["bin"] = bin;
}

void PackageInfo::setMan(const std::string &man)
{
    package_["man"] = man;
}

_PackageJson PackageInfo::toStruct() const
{
    _PackageJson result;
    result.component.name = getName();
    result.component.version = getVersion();
    result.component.isPrivate = isPrivate();
    result.component.main = package_["main"].get<std::string>();
    result.component.bin = package_["bin"].get<std::string>();
    result.component.man = package_["man"].get<std::string>();

    if (package_["types"].is_null())
    {
        result.component.types = ComponentType::NONE;
    }
    else
    {
        result.component.types = toComponentType(package_["types"].get<int>());
    }
    result.component.repository = package_["repository"].value("url", "");
    result.component.homepage = package_["homepage"].value("url", "");
    result.component.bugs = package_["bugs"].value("url", "");
    result.component.keywords = package_["keywords"].get<std::vector<std::string>>();
    result.component.description = package_["description"].get<std::string>();
    result.component.author = package_["author"].get<std::string>();
    result.component.license = package_["license"].get<std::string>();

    result.scripts.dev = package_["scripts"].value("dev", "");
    result.scripts.build = package_["scripts"].value("build", "");
    result.scripts.start = package_["scripts"].value("start", "");
    result.scripts.lint = package_["scripts"].value("lint", "");

    for (const auto &dep : package_["dependencies"].items())
    {
        result.dependencies.regular[dep.key()] = dep.value().get<std::string>();
    }

    for (const auto &devDep : package_["devDependencies"].items())
    {
        result.devDependencies.regular[devDep.key()] = devDep.value().get<std::string>();
    }

    return result;
}
