#include "collection.hpp"

#include <algorithm>
#include <filesystem>

#include <tinyxml2.h>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace fs = std::filesystem;

auto INDIDriverCollection::parseDrivers(const std::string& path) -> bool {
    if (!fs::exists(path) || !fs::is_directory(path)) {
        LOG_F(ERROR, "INDI driver path {} does not exist", path);
        return false;
    }
    for (const auto& entry : fs::directory_iterator(path)) {
        const auto& fname = entry.path().filename().string();
        if (fname.ends_with(".xml") && fname.find("_sk") == std::string::npos) {
            files_.push_back(entry.path().string());
        }
    }

    for (const auto& fname : files_) {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(fname.c_str()) != tinyxml2::XML_SUCCESS) {
            LOG_F(ERROR, "Error loading file {}", fname);
            continue;
        }

        auto* root = doc.FirstChildElement("root");
        for (auto* group = root->FirstChildElement("devGroup");
             group != nullptr; group = group->NextSiblingElement("devGroup")) {
            const auto& family = group->Attribute("group");
            for (auto* device = group->FirstChildElement("device");
                 device != nullptr;
                 device = device->NextSiblingElement("device")) {
                const auto& label = device->Attribute("label");
                const auto& skel = device->Attribute("skel");
                const auto& name =
                    device->FirstChildElement("driver")->Attribute("name");
                const auto& binary =
                    device->FirstChildElement("driver")->GetText();
                const auto& version =
                    device->FirstChildElement("version")->GetText();

                drivers_.push_back(std::make_shared<INDIDeviceContainer>(
                    name, label, version, binary, family, skel));
            }
        }
    }

    std::sort(drivers_.begin(), drivers_.end(),
              [](const auto& a, const auto& b) { return a->label < b->label; });
    return true;
}

auto INDIDriverCollection::parseCustomDrivers(const json& drivers) -> bool {
    for (const auto& custom : drivers) {
        const auto& name = custom["name"].get<std::string>();
        const auto& label = custom["label"].get<std::string>();
        const auto& version = custom["version"].get<std::string>();
        const auto& binary = custom["exec"].get<std::string>();
        const auto& family = custom["family"].get<std::string>();
        drivers_.push_back(std::make_shared<INDIDeviceContainer>(
            name, label, version, binary, family, "", true));
    }
    return true;
}

void INDIDriverCollection::clearCustomDrivers() {
    drivers_.erase(
        std::remove_if(drivers_.begin(), drivers_.end(),
                       [](const auto& driver) { return driver->custom; }),
        drivers_.end());
}

auto INDIDriverCollection::getByLabel(const std::string& label)
    -> std::shared_ptr<INDIDeviceContainer> {
    for (const auto& driver : drivers_) {
        if (driver->label == label) {
            return driver;
        }
    }
    LOG_F(ERROR, "INDI device {} not found", label);
    return nullptr;
}

auto INDIDriverCollection::getByName(const std::string& name)
    -> std::shared_ptr<INDIDeviceContainer> {
    for (const auto& driver : drivers_) {
        if (driver->name == name) {
            return driver;
        }
    }
    LOG_F(ERROR, "INDI device {} not found", name);
    return nullptr;
}

auto INDIDriverCollection::getByBinary(const std::string& binary)
    -> std::shared_ptr<INDIDeviceContainer> {
    for (const auto& driver : drivers_) {
        if (driver->binary == binary) {
            return driver;
        }
    }
    LOG_F(ERROR, "INDI device {} not found", binary);
    return nullptr;
}

auto INDIDriverCollection::getFamilies()
    -> std::unordered_map<std::string, std::vector<std::string>> {
    std::unordered_map<std::string, std::vector<std::string>> families;
    for (const auto& driver : drivers_) {
        families[driver->family].push_back(driver->label);
        DLOG_F(INFO, "Family {} contains devices {}", driver->family,
               driver->label);
    }
    if (families.empty()) {
        LOG_F(ERROR, "No families found");
    }
    return families;
}
