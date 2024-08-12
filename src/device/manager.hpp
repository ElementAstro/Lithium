#ifndef LITHIUM_DEVICE_MANAGER_HPP
#define LITHIUM_DEVICE_MANAGER_HPP

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

    auto getCameraByName(const std::string& name) const
        -> std::shared_ptr<AtomCamera>;

private:
    std::unordered_map<std::string, std::shared_ptr<AtomDriver>> devicesByUUID_;
    std::unordered_map<std::string, std::shared_ptr<AtomDriver>> devicesByName_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<AtomDriver>>>
        devicesByType_;

    std::weak_ptr<ComponentManager> componentManager_;

    std::shared_ptr<AtomDriver> main_camera_;
};

}  // namespace lithium

#endif
