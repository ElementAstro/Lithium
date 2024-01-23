/*
 * hydrogen_driver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Hydrogen Web Driver

**************************************************/

#include "hydrogen_driver.hpp"
#include "../utils/utils.hpp"

#include <regex>
#include <fstream>
#include <filesystem>
#include <stdexcept>

#include "atom/type/tinyxml2.h"

#define LOGURU_USE_FMTLIB 1
#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

HydrogenDeviceContainer::HydrogenDeviceContainer(const std::string &name, const std::string &label, const std::string &version,
                                         const std::string &binary, const std::string &family,
                                         const std::string &skeleton, bool custom)
    : name(name), label(label), version(version), binary(binary),
      family(family), skeleton(skeleton), custom(custom) {}


bool HydrogenDriverCollection::parseDrivers(const std::string &path)
{
    if (!fs::exists(path) || !fs::is_directory(path))
    {
        LOG_F(ERROR, "Hydrogen driver path {} does not exist", path);
        return false;
    }
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
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(fname.c_str()) != tinyxml2::XML_SUCCESS)
        {
            LOG_F(ERROR, "Error loading file {}", fname);
            continue;
        }

        tinyxml2::XMLElement *root = doc.FirstChildElement("root");
        for (tinyxml2::XMLElement *group = root->FirstChildElement("devGroup"); group; group = group->NextSiblingElement("devGroup"))
        {
            const std::string &family = group->Attribute("group");
            for (tinyxml2::XMLElement *device = group->FirstChildElement("device"); device; device = device->NextSiblingElement("device"))
            {
                const std::string &label = device->Attribute("label");
                const std::string &skel = device->Attribute("skel");
                const std::string &name = device->FirstChildElement("driver")->Attribute("name");
                const std::string &binary = device->FirstChildElement("driver")->GetText();
                const std::string &version = device->FirstChildElement("version")->GetText();

                drivers.push_back(std::make_shared<HydrogenDeviceContainer>(name, label, version, binary, family, skel));
            }
        }
    }

    // Sort drivers by label
    std::sort(drivers.begin(), drivers.end(), [](const std::shared_ptr<HydrogenDeviceContainer> a, const std::shared_ptr<HydrogenDeviceContainer> b)
              { return a->label < b->label; });
}

bool HydrogenDriverCollection::parseCustomDrivers(const json &drivers)
{
    for (const auto &custom : drivers)
    {
        const std::string &name = custom["name"].get<std::string>();
        const std::string &label = custom["label"].get<std::string>();
        const std::string &version = custom["version"].get<std::string>();
        const std::string &binary = custom["exec"].get<std::string>();
        const std::string &family = custom["family"].get<std::string>();
        this->drivers.push_back(std::make_shared<HydrogenDeviceContainer>(name, label, version, binary, family, "", true));
    }
}

void HydrogenDriverCollection::clearCustomDrivers()
{
    drivers.erase(std::remove_if(drivers.begin(), drivers.end(), [](const std::shared_ptr<HydrogenDeviceContainer> driver)
                                 { return driver->custom == true; }),
                  drivers.end());
}

std::shared_ptr<HydrogenDeviceContainer> HydrogenDriverCollection::getByLabel(const std::string &label)
{
    for (auto driver : drivers)
    {
        if (driver->label == label)
        {
            return driver;
        }
    }
    LOG_F(ERROR, "Hydrogen device {} not found", label);
    return nullptr;
}

std::shared_ptr<HydrogenDeviceContainer> HydrogenDriverCollection::getByName(const std::string &name)
{
    for (auto driver : drivers)
    {
        if (driver->name == name)
        {
            return driver;
        }
    }
    LOG_F(ERROR, "Hydrogen device {} not found", name);
    return nullptr;
}

std::shared_ptr<HydrogenDeviceContainer> HydrogenDriverCollection::getByBinary(const std::string &binary)
{
    for (auto driver : drivers)
    {
        if (driver->binary == binary)
        {
            return driver;
        }
    }
    LOG_F(ERROR, "Hydrogen device {} not found", binary);
    return nullptr;
}

#if ENABLE_FASTHASH
emhash8::HashMap<std::string, std::vector<std::string>> HydrogenDriverCollection::getFamilies()
#else
std::unordered_map<std::string, std::vector<std::string>> HydrogenDriverCollection::getFamilies()
#endif
{
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::vector<std::string>> families;
#else
    std::unordered_map<std::string, std::vector<std::string>> families;
#endif
    for (const auto driver : drivers)
    {
        families[driver->family].push_back(driver->label);
        DLOG_F(INFO, "Family {} contains devices {}", driver->family, driver->label);
    }
    if (families.empty())
    {
        LOG_F(ERROR, "No families found");
    }
    return families;
}
