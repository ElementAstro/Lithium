#ifndef LITHIUM_DEVICE_BASIC_HPP
#define LITHIUM_DEVICE_BASIC_HPP

#include <memory>
#include <string>
#include <vector>

#include "atom/macro.hpp"
#include "atom/type/json.hpp"

class AtomDriver;

namespace lithium::device {

struct Device {
    std::string label;
    std::string manufacturer;
    std::string driverName;
    std::string version;
} ATOM_ALIGNAS(128);

struct DevGroup {
    std::string groupName;
    std::vector<Device> devices;
} ATOM_ALIGNAS(64);

struct DriversList {
    std::vector<DevGroup> devGroups;
    int selectedGroup =
        -1;  // Fixed typo: changed 'selectedGrounp' to 'selectedGroup'
} ATOM_ALIGNAS(32);

struct SystemDevice {
    std::string description;
    int deviceIndiGroup;
    std::string deviceIndiName;
    std::string driverIndiName;
    std::string driverForm;
    std::shared_ptr<AtomDriver> driver;
    bool isConnect;
} ATOM_ALIGNAS(128);

struct SystemDeviceList {
    std::vector<SystemDevice> systemDevices;
    int currentDeviceCode = -1;
} ATOM_ALIGNAS(32);

}  // namespace lithium::device

// to_json and from_json functions for Device
inline void to_json(nlohmann::json& jsonObj,
                    const lithium::device::Device& device) {
    jsonObj = nlohmann::json{{"label", device.label},
                             {"manufacturer", device.manufacturer},
                             {"driverName", device.driverName},
                             {"version", device.version}};
}

inline void from_json(const nlohmann::json& jsonObj,
                      lithium::device::Device& device) {
    jsonObj.at("label").get_to(device.label);
    jsonObj.at("manufacturer").get_to(device.manufacturer);
    jsonObj.at("driverName").get_to(device.driverName);
    jsonObj.at("version").get_to(device.version);
}

inline void to_json(nlohmann::json& jsonArray,
                    const std::vector<lithium::device::Device>& vec) {
    jsonArray = nlohmann::json::array();
    for (const auto& device : vec) {
        jsonArray.push_back({{"label", device.label},
                             {"manufacturer", device.manufacturer},
                             {"driverName", device.driverName},
                             {"version", device.version}});
    }
}

inline void from_json(const nlohmann::json& jsonArray,
                      std::vector<lithium::device::Device>& vec) {
    for (const auto& jsonObj : jsonArray) {
        lithium::device::Device device;
        jsonObj.at("label").get_to(device.label);
        jsonObj.at("manufacturer").get_to(device.manufacturer);
        jsonObj.at("driverName").get_to(device.driverName);
        jsonObj.at("version").get_to(device.version);
        vec.push_back(device);
    }
}

// to_json and from_json functions for DevGroup
inline void to_json(nlohmann::json& jsonObj,
                    const lithium::device::DevGroup& group) {
    jsonObj = nlohmann::json{{"group", group.groupName}};
    to_json(jsonObj["devices"], group.devices);
}

inline void from_json(const nlohmann::json& jsonObj,
                      lithium::device::DevGroup& group) {
    jsonObj.at("group").get_to(group.groupName);
    from_json(jsonObj.at("devices"), group.devices);
}

inline void to_json(nlohmann::json& jsonArray,
                    const std::vector<lithium::device::DevGroup>& vec) {
    jsonArray = nlohmann::json::array();
    for (const auto& group : vec) {
        jsonArray.push_back({{"group", group.groupName}});
        to_json(jsonArray.back()["devices"], group.devices);
    }
}

inline void from_json(const nlohmann::json& jsonArray,
                      std::vector<lithium::device::DevGroup>& vec) {
    for (const auto& jsonObj : jsonArray) {
        lithium::device::DevGroup group;
        jsonObj.at("group").get_to(group.groupName);
        from_json(jsonObj.at("devices"), group.devices);
        vec.push_back(group);
    }
}

// to_json and from_json functions for DriversList
inline void to_json(nlohmann::json& jsonObj,
                    const lithium::device::DriversList& driversList) {
    to_json(jsonObj["devGroups"], driversList.devGroups);
    jsonObj["selectedGroup"] = driversList.selectedGroup;
    // Fixed typo
}

inline void from_json(const nlohmann::json& jsonObj,
                      lithium::device::DriversList& driversList) {
    from_json(jsonObj.at("devGroups"), driversList.devGroups);
    jsonObj.at("selectedGroup").get_to(driversList.selectedGroup);
}

#endif
