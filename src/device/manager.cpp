#include "manager.hpp"

#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "addon/manager.hpp"
#include "device/template/camera.hpp"

#include "atom/log/loguru.hpp"
#include "device/template/device.hpp"

namespace lithium {
auto DeviceManager::createShared() -> std::shared_ptr<DeviceManager> {
    return std::make_shared<DeviceManager>();
}

auto DeviceManager::addDeviceFromComponent(const std::string& device_type,
                                           const std::string& device_name,
                                           const std::string& component,
                                           const std::string& entry) -> bool {
    if (componentManager_.expired()) {
        LOG_F(ERROR, "Component manager expired");
        return false;
    }
    auto component_ = componentManager_.lock();
    if (!component_->hasComponent(component)) {
        LOG_F(ERROR, "Component {} not found", component);
        return false;
    }
    auto componentPtr = component_->getComponent(component).value();
    if (componentPtr.expired()) {
        LOG_F(ERROR, "Component {} expired", component);
        return false;
    }
    try {
        if (device_type == "camera") {
            auto driver = std::dynamic_pointer_cast<AtomDriver>(
                std::any_cast<std::shared_ptr<AtomCamera>>(
                    componentPtr.lock()->dispatch("create_instance",
                                                  device_name)));
            devicesByName_[device_name] = driver;
            devicesByUUID_[driver->getUUID()] = driver;
            devicesByType_[device_type].push_back(driver);
            return true;
        }
    } catch (const std::bad_cast& e) {
        LOG_F(ERROR, "Failed to cast component {} to {}", component,
              device_type);
    }
}

std::shared_ptr<AtomDriver> DeviceManager::getDeviceByUUID(
    const std::string& uuid) const {
    auto it = devicesByUUID_.find(uuid);
    if (it != devicesByUUID_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<AtomDriver> DeviceManager::getDeviceByName(
    const std::string& name) const {
    auto it = devicesByName_.find(name);
    if (it != devicesByName_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<AtomDriver>> DeviceManager::getDevicesByType(
    const std::string& type) const {
    auto it = devicesByType_.find(type);
    if (it != devicesByType_.end()) {
        return it->second;
    }
    return {};
}

bool DeviceManager::removeDeviceByUUID(const std::string& uuid) {
    auto it = devicesByUUID_.find(uuid);
    if (it != devicesByUUID_.end()) {
        devicesByName_.erase(it->second->getName());
        auto& typeList = devicesByType_[it->second->getType()];
        typeList.erase(
            std::remove(typeList.begin(), typeList.end(), it->second),
            typeList.end());
        devicesByUUID_.erase(it);
        return true;
    }
    return false;
}

bool DeviceManager::removeDeviceByName(const std::string& name) {
    auto it = devicesByName_.find(name);
    if (it != devicesByName_.end()) {
        devicesByUUID_.erase(it->second->getUUID());
        auto& typeList = devicesByType_[it->second->getType()];
        typeList.erase(
            std::remove(typeList.begin(), typeList.end(), it->second),
            typeList.end());
        devicesByName_.erase(it);
        return true;
    }
    return false;
}

void DeviceManager::listDevices() const {
    std::cout << "Devices list:" << std::endl;
    for (const auto& pair : devicesByUUID_) {
        std::cout << "UUID: " << pair.first
                  << ", Name: " << pair.second->getName()
                  << ", Type: " << pair.second->getType() << std::endl;
    }
}

bool DeviceManager::updateDeviceName(const std::string& uuid,
                                     const std::string& newName) {
    auto device = getDeviceByUUID(uuid);
    if (device) {
        devicesByName_.erase(device->getName());
        device->setName(newName);
        devicesByName_[newName] = device;
        return true;
    }
    return false;
}

size_t DeviceManager::getDeviceCount() const { return devicesByUUID_.size(); }

bool DeviceManager::saveToFile(const std::string& filename) const {
    nlohmann::json json;
    for (const auto& pair : devicesByUUID_) {
        // json.push_back(pair.second->toJson());
    }
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << json.dump(4);
    return true;
}

auto DeviceManager::getCameraByName(const std::string& name) const
    -> std::shared_ptr<AtomCamera> {
    auto it = devicesByName_.find(name);
    if (it != devicesByName_.end()) {
        return std::dynamic_pointer_cast<AtomCamera>(it->second);
    }
    return nullptr;
}
}  // namespace lithium
