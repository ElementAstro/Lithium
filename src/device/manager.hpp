#ifndef LITHIUM_DEVICE_MANAGER_HPP
#define LITHIUM_DEVICE_MANAGER_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "device/template/camera.hpp"
#include "template/device.hpp"

namespace lithium {

class ComponentManager;

class DeviceManager {
public:
    static auto createShared() -> std::shared_ptr<DeviceManager>;

    template <typename DeviceType>
    std::shared_ptr<DeviceType> addDevice(const std::string& name) {
        auto device = std::make_shared<DeviceType>(name);
        devicesByUUID_[device->getUUID()] = device;
        devicesByName_[name] = device;
        devicesByType_[device->getType()].push_back(device);
        return device;
    }

    auto addDeviceFromComponent(const std::string& device_type,
                                const std::string& device_name,
                                const std::string& component,
                                const std::string& entry) -> bool;

    std::shared_ptr<AtomDriver> getDeviceByUUID(const std::string& uuid) const;
    std::shared_ptr<AtomDriver> getDeviceByName(const std::string& name) const;
    std::vector<std::shared_ptr<AtomDriver>> getDevicesByType(
        const std::string& type) const;

    auto removeDeviceByUUID(const std::string& uuid) -> bool;
    auto removeDeviceByName(const std::string& name) -> bool;

    void listDevices() const;

    auto updateDeviceName(const std::string& uuid,
                          const std::string& newName) -> bool;
    auto updateDeviceStatus(const std::string& uuid,
                            const std::string& newStatus) -> bool;

    auto getDeviceCount() const -> size_t;
    auto saveToFile(const std::string& filename) const -> bool;
    auto loadFromFile(const std::string& filename) -> bool;

    auto getCameraByName(const std::string& name) const
        -> std::shared_ptr<AtomCamera>;

    // New functionality
    using DeviceFilter =
        std::function<bool(const std::shared_ptr<AtomDriver>&)>;
    std::vector<std::shared_ptr<AtomDriver>> findDevices(
        const DeviceFilter& filter) const;

    void setDeviceUpdateCallback(
        const std::string& uuid,
        std::function<void(const std::shared_ptr<AtomDriver>&)> callback);
    void removeDeviceUpdateCallback(const std::string& uuid);

    auto getDeviceUsageStatistics(const std::string& uuid) const
        -> std::pair<std::chrono::seconds, int>;
    void resetDeviceUsageStatistics(const std::string& uuid);

    auto getLastErrorForDevice(const std::string& uuid) const -> std::string;
    void clearLastErrorForDevice(const std::string& uuid);

    void enableDeviceLogging(const std::string& uuid, bool enable);
    auto getDeviceLog(const std::string& uuid) const
        -> std::vector<std::string>;

    auto createDeviceGroup(const std::string& groupName,
                           const std::vector<std::string>& deviceUUIDs) -> bool;
    auto removeDeviceGroup(const std::string& groupName) -> bool;
    auto getDeviceGroup(const std::string& groupName) const
        -> std::vector<std::shared_ptr<AtomDriver>>;

    void performBulkOperation(
        const std::vector<std::string>& deviceUUIDs,
        const std::function<void(std::shared_ptr<AtomDriver>&)>& operation);

private:
    std::unordered_map<std::string, std::shared_ptr<AtomDriver>> devicesByUUID_;
    std::unordered_map<std::string, std::shared_ptr<AtomDriver>> devicesByName_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<AtomDriver>>>
        devicesByType_;
    std::weak_ptr<ComponentManager> componentManager_;
    std::shared_ptr<AtomDriver> main_camera_;

    // New member variables
    std::unordered_map<std::string,
                       std::function<void(const std::shared_ptr<AtomDriver>&)>>
        updateCallbacks_;
    std::unordered_map<std::string,
                       std::pair<std::chrono::steady_clock::time_point, int>>
        deviceUsageStats_;
    std::unordered_map<std::string, std::string> lastDeviceErrors_;
    std::unordered_map<std::string, std::vector<std::string>> deviceLogs_;
    std::unordered_map<std::string, std::vector<std::string>> deviceGroups_;
};

}  // namespace lithium

#endif
