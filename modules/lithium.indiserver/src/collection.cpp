#include "collection.hpp"

#include <algorithm>
#include <filesystem>

#include <tinyxml2.h>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace fs = std::filesystem;

// 收集XML文件
auto INDIDriverCollection::collectXMLFiles(const std::string& path) -> bool {
    LOG_F(INFO, "Collecting XML files from path: {}", path);
    if (!fs::exists(path) || !fs::is_directory(path)) {
        LOG_F(ERROR, "INDI driver path {} does not exist", path);
        return false;
    }

    files_.clear();
    for (const auto& entry : fs::directory_iterator(path)) {
        const auto& fname = entry.path().filename().string();
        if (fname.ends_with(".xml") && fname.find("_sk") == std::string::npos) {
            LOG_F(INFO, "Found XML file: {}", fname);
            files_.push_back(entry.path().string());
        }
    }
    bool result = !files_.empty();
    LOG_F(INFO, "Collected {} XML files", files_.size());
    return result;
}

// 解析单个设备
auto INDIDriverCollection::parseDevice(tinyxml2::XMLElement* device,
                                       const char* family)
    -> std::shared_ptr<INDIDeviceContainer> {
    if (!device || !family) {
        LOG_F(ERROR, "Null device or family pointer");
        return nullptr;
    }
    try {
        const char* label = device->Attribute("label");
        if (!label) {
            LOG_F(ERROR, "Device missing required 'label' attribute");
            return nullptr;
        }

        // 处理可选的skel属性
        const char* skel = device->Attribute("skel");
        std::string skelPath;
        if (skel) {
            skelPath = skel;
        }

        auto* driverElement = device->FirstChildElement("driver");
        if (!driverElement) {
            LOG_F(ERROR, "Device '{}' missing driver element", label);
            return nullptr;
        }

        const char* name = driverElement->Attribute("name");
        if (!name) {
            LOG_F(ERROR, "Driver missing required 'name' attribute");
            return nullptr;
        }

        const char* binary = driverElement->GetText();
        if (!binary) {
            LOG_F(ERROR, "Driver missing binary path");
            return nullptr;
        }

        // 获取版本，提供默认值
        const char* version = "0.0";
        auto* versionElement = device->FirstChildElement("version");
        if (versionElement && versionElement->GetText()) {
            version = versionElement->GetText();
        }

        LOG_F(INFO,
              "Parsed device: label={}, name={}, version={}, binary={}, "
              "family={}, skelPath={}",
              label, name, version, binary, family, skelPath);

        return std::make_shared<INDIDeviceContainer>(name, label, version,
                                                     binary, family, skelPath);

    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error parsing device: {}", e.what());
        return nullptr;
    }
}

auto INDIDriverCollection::parseDrivers(const std::string& path) -> bool {
    LOG_F(INFO, "Parsing drivers from path: {}", path);
    if (!collectXMLFiles(path)) {
        LOG_F(INFO, "No XML files found in directory {}", path);
        THROW_FILE_NOT_FOUND("No XML files found in directory: ", path);
    }

    drivers_.clear();

    for (const auto& fname : files_) {
        LOG_F(INFO, "Loading XML file: {}", fname);
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(fname.c_str()) != tinyxml2::XML_SUCCESS) {
            LOG_F(ERROR, "Error loading file {}: {}", fname, doc.ErrorStr());
            continue;
        }

        auto* root = doc.FirstChildElement("root");
        if (!root) {
            LOG_F(ERROR, "Missing root element in {}", fname);
            continue;
        }

        for (auto* group = root->FirstChildElement("devGroup");
             group != nullptr; group = group->NextSiblingElement("devGroup")) {
            const char* family = group->Attribute("group");
            if (!family) {
                LOG_F(ERROR, "Device group missing 'group' attribute in {}",
                      fname);
                continue;
            }

            for (auto* device = group->FirstChildElement("device");
                 device != nullptr;
                 device = device->NextSiblingElement("device")) {
                if (auto driver = parseDevice(device, family)) {
                    drivers_.push_back(driver);
                }
            }
        }
    }

    // 按label排序
    std::sort(drivers_.begin(), drivers_.end(),
              [](const auto& a, const auto& b) { return a->label < b->label; });

    LOG_F(INFO, "Parsed {} drivers", drivers_.size());
    return !drivers_.empty();
}

auto INDIDriverCollection::parseCustomDrivers(const json& drivers) -> bool {
    LOG_F(INFO, "Parsing custom drivers");
    for (const auto& custom : drivers) {
        const auto& name = custom["name"].get<std::string>();
        const auto& label = custom["label"].get<std::string>();
        const auto& version = custom["version"].get<std::string>();
        const auto& binary = custom["exec"].get<std::string>();
        const auto& family = custom["family"].get<std::string>();
        LOG_F(INFO,
              "Parsed custom driver: name={}, label={}, version={}, binary={}, "
              "family={}",
              name, label, version, binary, family);
        drivers_.push_back(std::make_shared<INDIDeviceContainer>(
            name, label, version, binary, family, "", true));
    }
    return true;
}

void INDIDriverCollection::clearCustomDrivers() {
    LOG_F(INFO, "Clearing custom drivers");
    drivers_.erase(
        std::remove_if(drivers_.begin(), drivers_.end(),
                       [](const auto& driver) { return driver->custom; }),
        drivers_.end());
}

auto INDIDriverCollection::getByLabel(const std::string& label)
    -> std::shared_ptr<INDIDeviceContainer> {
    LOG_F(INFO, "Getting driver by label: {}", label);
    for (const auto& driver : drivers_) {
        if (driver->label == label) {
            LOG_F(INFO, "Found driver with label: {}", label);
            return driver;
        }
    }
    DLOG_F(INFO, "INDI device {} not found", label);
    return nullptr;
}

auto INDIDriverCollection::getByName(const std::string& name)
    -> std::shared_ptr<INDIDeviceContainer> {
    LOG_F(INFO, "Getting driver by name: {}", name);
    for (const auto& driver : drivers_) {
        if (driver->name == name) {
            LOG_F(INFO, "Found driver with name: {}", name);
            return driver;
        }
    }
    LOG_F(ERROR, "INDI device {} not found", name);
    return nullptr;
}

auto INDIDriverCollection::getByBinary(const std::string& binary)
    -> std::shared_ptr<INDIDeviceContainer> {
    LOG_F(INFO, "Getting driver by binary: {}", binary);
    for (const auto& driver : drivers_) {
        if (driver->binary == binary) {
            LOG_F(INFO, "Found driver with binary: {}", binary);
            return driver;
        }
    }
    LOG_F(ERROR, "INDI device {} not found", binary);
    return nullptr;
}

auto INDIDriverCollection::getFamilies()
    -> std::unordered_map<std::string, std::vector<std::string>> {
    LOG_F(INFO, "Getting all families");
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