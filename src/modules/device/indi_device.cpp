/*
 * idriver.cpp
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

Date: 2023-3-29

Description: INDI Web Driver

**************************************************/

#include "indi_device.hpp"
#include "device_utils.hpp"

#include <regex>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include <pugixml/pugixml.hpp>

#define LOGURU_USE_FMTLIB 1
#include "loguru/loguru.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

INDIDeviceContainer::INDIDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                                         const std::string &binary, const std::string &family,
                                         const std::string &skeleton, bool custom)
    : name(name), label(label), version(version), binary(binary),
      family(family), skeleton(skeleton), custom(custom) {}

INDIDriverCollection::INDIDriverCollection(const std::string &path) : path(path)
{
    parseDrivers();
}

void INDIDriverCollection::parseDrivers()
{
    for (const auto &entry : fs::directory_iterator(path))
    {
        const std::string &fname = entry.path().filename().string();
        if (fname.ends_with(".xml") && fname.find("_sk") == std::string::npos)
        {
            files.push_back(entry.path().string());
        }
    }

    for (const std::string &fname : files)
    {
        pugi::xml_document doc;
        if (!doc.load_file(fname.c_str()))
        {
            DLOG_F(ERROR, "Error loading file {}", fname);
            continue;
        }

        pugi::xml_node root = doc.child("root");
        for (pugi::xml_node group = root.child("devGroup"); group; group = group.next_sibling("devGroup"))
        {
            const std::string &family = group.attribute("group").as_string();
            for (pugi::xml_node device = group.child("device"); device; device = device.next_sibling("device"))
            {
                const std::string &label = device.attribute("label").as_string();
                const std::string &skel = device.attribute("skel").as_string();
                const std::string &name = device.child("driver").attribute("name").as_string();
                const std::string &binary = device.child("driver").text().as_string();
                const std::string &version = device.child("version").text().as_string("0.0");

                drivers.push_back(std::make_shared<INDIDeviceContainer>(name, label, version, binary, family, skel));
            }
        }
    }

    // Sort drivers by label
    std::sort(drivers.begin(), drivers.end(), [](const std::shared_ptr<INDIDeviceContainer> a, const std::shared_ptr<INDIDeviceContainer> b)
              { return a->label < b->label; });
}

void INDIDriverCollection::parseCustomDrivers(const json &drivers)
{
    for (const auto &custom : drivers)
    {
        const std::string &name = custom["name"].get<std::string>();
        const std::string &label = custom["label"].get<std::string>();
        const std::string &version = custom["version"].get<std::string>();
        const std::string &binary = custom["exec"].get<std::string>();
        const std::string &family = custom["family"].get<std::string>();
        this->drivers.push_back(std::make_shared<INDIDeviceContainer>(name, label, version, binary, family, "", true));
    }
}

void INDIDriverCollection::clearCustomDrivers()
{
    drivers.erase(std::remove_if(drivers.begin(), drivers.end(), [](const std::shared_ptr<INDIDeviceContainer> driver)
                                 { return driver->custom == true; }),
                  drivers.end());
}

std::shared_ptr<INDIDeviceContainer> INDIDriverCollection::getByLabel(const std::string &label)
{
    for (auto driver : drivers)
    {
        if (driver->label == label)
        {
            return driver;
        }
    }
    return nullptr;
}

std::shared_ptr<INDIDeviceContainer> INDIDriverCollection::getByName(const std::string &name)
{
    for (auto driver : drivers)
    {
        if (driver->name == name)
        {
            return driver;
        }
    }
    return nullptr;
}

std::shared_ptr<INDIDeviceContainer> INDIDriverCollection::getByBinary(const std::string &binary)
{
    for (auto driver : drivers)
    {
        if (driver->binary == binary)
        {
            return driver;
        }
    }
    return nullptr;
}

std::map<std::string, std::vector<std::string>> INDIDriverCollection::getFamilies()
{
    std::map<std::string, std::vector<std::string>> families;
    for (const auto driver : drivers)
    {
        families[driver->family].push_back(driver->label);
    }
    return families;
}
