/*
 * manager.cpp
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
 
Description: Device Manager
 
**************************************************/

#include "manager.hpp"
#include "basic_device.hpp"

#include <spdlog/spdlog.h>
#include "nlohmann/json.hpp"

namespace OpenAPT {

    class Device {
        public:
            Device(const std::string& name) : _name(name) {}
            virtual ~Device() {}

            const std::string& getName() const {
                return _name;
            }

            virtual bool connect() = 0;
            virtual bool disconnect() = 0;
            virtual bool reconnect() = 0;
            virtual bool scan() = 0;

            virtual DeviceStatus getStatus() const = 0;
            const nlohmann::json& getDeviceInfo() const {
                return _deviceInfo;
            }

            std::string getId() const {
                return "device_id";
            }

        protected:
            std::string _name;
            nlohmann::json _deviceInfo;
    };

    class Camera : public Device {
        public:
            Camera(const std::string& name) : Device(name) {}

            bool connect() override {
                // 连接相机
                return true;
            }

            bool disconnect() override {
                // 断开相机连接
                return true;
            }

            bool reconnect() override {
                // 重连相机
                return true;
            }

            bool scan() override {
                // 扫描是否有已有相机
                return true;
            }

            DeviceStatus getStatus() const override {
                // 获取相机状态
                return DeviceStatus::Connected;
            }

            bool setExposureTime(double time) {
                // 设置相机曝光时间
                return true;
            }

            bool setGain(double gain) {
                // 设置相机增益
                return true;
            }

            bool setResolution(int width, int height) {
                // 设置相机分辨率
                return true;
            }
    };

    class Telescope : public Device {
        public:
            Telescope(const std::string& name) : Device(name) {}

            bool connect() override {
                // 连接望远镜
                return true;
            }

            bool disconnect() override {
                // 断开望远镜连接
                return true;
            }

            bool reconnect() override {
                // 重连望远镜
                return true;
            }

            bool scan() override {
                // 扫描是否有已有望远镜
                return true;
            }

            DeviceStatus getStatus() const override {
                // 获取望远镜状态
                return DeviceStatus::Disconnected;
            }

            bool moveTo(double ra, double dec) {
                // 控制望远镜走到指定位置
                return true;
            }

            bool setFocus(int focusPosition) {
                // 调整望远镜的焦距
                return true;
            }

            bool setTracking(bool enabled) {
                // 开启或关闭望远镜的跟踪模式
                return true;
            }
    };

    class Focuser : public Device {
    public:
        Focuser(const std::string& name) : Device(name) {}

        bool connect() override {
            // 连接电调
            return true;
        }

        bool disconnect() override {
            // 断开电调连接
            return true;
        }

        bool reconnect() override {
            // 重连电调
            return true;
        }

        bool scan() override {
            // 扫描是否有已有电调
            return true;
        }

        DeviceStatus getStatus() const override {
            // 获取电调状态
            return DeviceStatus::Connected;
        }

        bool setSpeed(double speed) {
            // 设置电调速度
            return true;
        }

        bool setDirection(bool forward) {
            // 设置电调运动方向
            return true;
        }

        bool setRotation(double angle) {
            // 控制电调旋转
            return true;
        }
    };

    class FilterWheel : public Device {
        public:
            FilterWheel(const std::string& name) : Device(name) {}

            bool connect() override {
                // 连接滤镜轮
                return true;
            }

            bool disconnect() override {
                // 断开滤镜轮连接
                return true;
            }

            bool reconnect() override {
                // 重连滤镜轮
                return true;
            }

            bool scan() override {
                // 扫描是否有已有滤镜轮
                return true;
            }

            DeviceStatus getStatus() const override {
                // 获取滤镜轮状态
                return DeviceStatus::Disconnected;
            }

            bool setFilter(int filterIndex) {
                // 改变滤镜轮当前滤镜
                return true;
            }
    };

    class Solver : public Device {
        public:
            Solver(const std::string& name) : Device(name) {}

            bool connect() override {
                // 连接解析器
                return true;
            }

            bool disconnect() override {
                // 断开解析器连接
                return true;
            }

            bool reconnect() override {
                // 重连解析器
                return true;
            }

            bool scan() override {
                // 扫描是否有已有解析器
                return true;
            }

            DeviceStatus getStatus() const override {
                // 获取解析器状态
                return DeviceStatus::Connected;
            }

            bool parseData(const std::string& data) {
                // 解析数据并返回结果
                return true;
            }
    };

    class Guider : public Device {
        public:
            Guider(const std::string& name) : Device(name) {}

            bool connect() override {
                // 连接导星器
                return true;
            }

            bool disconnect() override {
                // 断开导星器连接
                return true;
            }

            bool reconnect() override {
                // 重连导星器
                return true;
            }

            bool scan() override {
                // 扫描是否有已有导星器
                return true;
            }

            DeviceStatus getStatus() const override {
                // 获取导星器状态
                return DeviceStatus::Connected;
            }

            bool setGuideMode(bool autoGuide) {
                // 设置导星模式
                return true;
            }

            bool calibrate() {
                // 导星器校准
                return true;
            }

            bool startGuiding() {
                // 开始导星
                return true;
            }

            bool stopGuiding() {
                // 停止导星
                return true;
            }
    };

    // Constructor
    DeviceManager::DeviceManager() {
        for (auto& devices : m_devices) {
            devices.emplace_back();
        }
    }

    // Destructor
    DeviceManager::~DeviceManager() {
        for (auto& devices : m_devices) {
            for (auto& device : devices) {
                if (device) {
                    device->disconnect();
                }
            }
        }
    }

    // Returns a list of device names of the specified type
    std::vector<std::string> DeviceManager::getDeviceList(DeviceType type) {
        std::vector<std::string> deviceList;
        auto& devices = m_devices[static_cast<int>(type)];
        for (const auto& device : devices) {
            if (device) {
                deviceList.emplace_back(device->getName());
            }
        }
        return deviceList;
    }

    // Adds a new device with the given name to the specified device type
    bool DeviceManager::addDevice(DeviceType type, const std::string& name) {
        assert(type >= DeviceType::Camera && type <= DeviceType::Guider && "Invalid device type");

        // Check if a device with the same name already exists
        if (findDeviceByName(name)) {
            spdlog::warn("A device with name {} already exists, please choose a different name", name);
            return false;
        }

        std::string newName = name;
        int index = 1;
        while (findDevice(type, newName) != -1) {
            newName = fmt::format("{}-{}", name, index++);
        }
        switch (type) {
            case DeviceType::Camera:
                m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Camera>(newName));
                break;
            case DeviceType::Telescope:
                m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
                break;
            case DeviceType::Focuser:
                m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
                break;
            case DeviceType::FilterWheel:
                m_devices[static_cast<int>(type)].emplace_back(std::make_shared<FilterWheel>(newName));
                break;
            case DeviceType::Solver:
                m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
                break;
            case DeviceType::Guider:
                m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
                break;
            default:
                spdlog::error("Invalid device type");
                return false;
        }
        return true;
    }

    // Removes a device with the given name from the specified device type
    bool DeviceManager::removeDevice(DeviceType type, const std::string& name) {
        auto& devices = m_devices[static_cast<int>(type)];
        for (auto it = devices.begin(); it != devices.end(); ++it) {
            if (*it && (*it)->getName() == name) {
                (*it)->disconnect();
                devices.erase(it);
                return true;
            }
        }
        spdlog::warn("Could not find device {} of type {}", name, static_cast<int>(type));
        return false;
    }

    // Removes all devices with the given name from all device types
    void DeviceManager::removeDevicesByName(const std::string& name) {
        for (auto& devices : m_devices) {
            devices.erase(std::remove_if(devices.begin(), devices.end(),
                        [&](std::shared_ptr<Device> device) { return device && device->getName() == name; }),
                        devices.end());
        }
    }

    // Returns a shared pointer to the device with the given name and type
    std::shared_ptr<Device> DeviceManager::getDevice(DeviceType type, const std::string& name) {
        int index = findDevice(type, name);
        if (index != -1) {
            return m_devices[static_cast<int>(type)][index];
        } else {
            spdlog::warn("Could not find device {} of type {}", name, static_cast<int>(type));
            return nullptr;
        }
    }

    // Returns the index of the device with the given name and type, or -1 if not found
    int DeviceManager::findDevice(DeviceType type, const std::string& name) {
        auto& devices = m_devices[static_cast<int>(type)];
        for (size_t i = 0; i < devices.size(); ++i) {
            if (devices[i] && devices[i]->getName() == name) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    // Returns a shared pointer to the device with the given name, or nullptr if not found
    std::shared_ptr<Device> DeviceManager::findDeviceByName(const std::string& name) const {
        for (const auto& devices : m_devices) {
            for (const auto& device : devices) {
                if (device && device->getName() == name) {
                    return device;
                }
            }
        }
        return nullptr;
    }
    
}
