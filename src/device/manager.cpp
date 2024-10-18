#include "manager.hpp"

#include <any>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "addon/manager.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "device/template/camera.hpp"
#include "device/template/device.hpp"

using json = nlohmann::json;

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
    return false;
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

auto DeviceManager::getCameraByName(const std::string& name) const
    -> std::shared_ptr<AtomCamera> {
    auto it = devicesByName_.find(name);
    if (it != devicesByName_.end()) {
        return std::dynamic_pointer_cast<AtomCamera>(it->second);
    }
    return nullptr;
}

std::vector<std::shared_ptr<AtomDriver>> DeviceManager::findDevices(
    const DeviceFilter& filter) const {
    std::vector<std::shared_ptr<AtomDriver>> result;
    for (const auto& pair : devicesByUUID_) {
        if (filter(pair.second)) {
            result.push_back(pair.second);
        }
    }
    return result;
}

void DeviceManager::setDeviceUpdateCallback(
    const std::string& uuid,
    std::function<void(const std::shared_ptr<AtomDriver>&)> callback) {
    updateCallbacks_[uuid] = std::move(callback);
}

void DeviceManager::removeDeviceUpdateCallback(const std::string& uuid) {
    updateCallbacks_.erase(uuid);
}

auto DeviceManager::getDeviceUsageStatistics(const std::string& uuid) const
    -> std::pair<std::chrono::seconds, int> {
    auto it = deviceUsageStats_.find(uuid);
    if (it != deviceUsageStats_.end()) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - it->second.first);
        return {duration, it->second.second};
    }
    return {std::chrono::seconds(0), 0};
}

void DeviceManager::resetDeviceUsageStatistics(const std::string& uuid) {
    deviceUsageStats_[uuid] = {std::chrono::steady_clock::now(), 0};
}

auto DeviceManager::getLastErrorForDevice(const std::string& uuid) const
    -> std::string {
    auto it = lastDeviceErrors_.find(uuid);
    return (it != lastDeviceErrors_.end()) ? it->second : "";
}

void DeviceManager::clearLastErrorForDevice(const std::string& uuid) {
    lastDeviceErrors_.erase(uuid);
}

void DeviceManager::enableDeviceLogging(const std::string& uuid, bool enable) {
    if (enable) {
        deviceLogs_[uuid] = {};
    } else {
        deviceLogs_.erase(uuid);
    }
}

auto DeviceManager::getDeviceLog(const std::string& uuid) const
    -> std::vector<std::string> {
    auto it = deviceLogs_.find(uuid);
    return (it != deviceLogs_.end()) ? it->second : std::vector<std::string>();
}

auto DeviceManager::createDeviceGroup(
    const std::string& groupName,
    const std::vector<std::string>& deviceUUIDs) -> bool {
    if (deviceGroups_.find(groupName) != deviceGroups_.end()) {
        return false;  // Group already exists
    }
    deviceGroups_[groupName] = deviceUUIDs;
    return true;
}

auto DeviceManager::removeDeviceGroup(const std::string& groupName) -> bool {
    return deviceGroups_.erase(groupName) > 0;
}

auto DeviceManager::getDeviceGroup(const std::string& groupName) const
    -> std::vector<std::shared_ptr<AtomDriver>> {
    std::vector<std::shared_ptr<AtomDriver>> result;
    auto it = deviceGroups_.find(groupName);
    if (it != deviceGroups_.end()) {
        for (const auto& uuid : it->second) {
            auto device = getDeviceByUUID(uuid);
            if (device) {
                result.push_back(device);
            }
        }
    }
    return result;
}

void DeviceManager::performBulkOperation(
    const std::vector<std::string>& deviceUUIDs,
    const std::function<void(std::shared_ptr<AtomDriver>&)>& operation) {
    for (const auto& uuid : deviceUUIDs) {
        auto device = getDeviceByUUID(uuid);
        if (device) {
            operation(device);
        }
    }
}

auto DeviceManager::loadFromFile(const std::string& filename) -> bool {
    std::ifstream file(filename);
    if (!file.is_open()) {
        LOG_F(ERROR, "Failed to open file: {}", filename);
        return false;
    }

    try {
        nlohmann::json json;
        file >> json;

        for (const auto& deviceJson : json) {
            // Implement device creation from JSON
            // This is a placeholder and needs to be implemented based on your
            // specific device structure
            std::string uuid = deviceJson["uuid"];
            std::string name = deviceJson["name"];
            std::string type = deviceJson["type"];

            // Create device based on type
            std::shared_ptr<AtomDriver> device;
            if (type == "camera") {
                device = std::make_shared<AtomCamera>(name);
            } else {
                // Add other device types as needed
                LOG_F(WARNING, "Unknown device type: {}", type);
                continue;
            }

            // Set device properties
            // device->setUUID(uuid);
            // Set other properties as needed

            // Add device to manager
            devicesByUUID_[uuid] = device;
            devicesByName_[name] = device;
            devicesByType_[type].push_back(device);
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error parsing JSON: {}", e.what());
        return false;
    }

    return true;
}

// Update the existing saveToFile method
auto DeviceManager::saveToFile(const std::string& filename) const -> bool {
    nlohmann::json json;
    for (const auto& pair : devicesByUUID_) {
        nlohmann::json deviceJson;
        deviceJson["uuid"] = pair.second->getUUID();
        deviceJson["name"] = pair.second->getName();
        deviceJson["type"] = pair.second->getType();
        // Add other device properties as needed
        json.push_back(deviceJson);
    }

    std::ofstream file(filename);
    if (!file.is_open()) {
        LOG_F(ERROR, "Failed to open file for writing: {}", filename);
        return false;
    }

    file << json.dump(4);
    return true;
}

}  // namespace lithium
