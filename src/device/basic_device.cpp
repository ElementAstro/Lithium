/*
 * basic_device.cpp
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
 
Description: Basic Device Definitions
 
**************************************************/

#include <iostream>
#include <vector>
#include <string>

#include "basic_device.hpp"

namespace OpenAPT
{
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
} // namespace OpenAPT

/*
int main() {
    DeviceManager manager;
    manager.addDevice(DeviceType::Camera, "Camera1");
    manager.addDevice(DeviceType::Camera, "Camera1");
    manager.addDevice(DeviceType::Camera, "Camera2");
    manager.addDevice(DeviceType::Telescope, "Telescope1");
    manager.addDevice(DeviceType::Focuser, "EC1");
    manager.addDevice(DeviceType::FilterWheel, "FW1");
    manager.addDevice(DeviceType::Solver, "Solver1");
    manager.addDevice(DeviceType::Guider, "Guider1");

    auto cameraList = manager.getDeviceList(DeviceType::Camera);
    std::cout << "相机列表: ";
    for (auto& name : cameraList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    auto telescopeList = manager.getDeviceList(DeviceType::Telescope);
    std::cout << "望远镜列表: ";
    for (auto& name : telescopeList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    spdlog::info("{}",manager.findDeviceByName("Camera2")->getName());

    auto device1 = manager.findDeviceByName("Camera1");
    if (device1 != nullptr) {
        spdlog::info("{}",device1->connect());
        // 找到了设备
        // ...
    } else {
        spdlog::error("找不到设备 Camera1");
    }

    auto deviceaa = manager.findDeviceByName("Camera1aaa");
    if (deviceaa != nullptr) {
        spdlog::info("{}",deviceaa->connect());
        // 找到了设备
        // ...
    } else {
        spdlog::error("找不到设备 Camera1");
    }

    //manager.removeDevice(DeviceType::Camera,"Camera1");
    manager.removeDevicesByName("Camera1");
    auto device = manager.findDeviceByName("Camera1");
    if (device != nullptr) {
        spdlog::info("{}",device->connect());
        // 找到了设备
        // ...
    } else {
        spdlog::error("找不到设备 Camera1");
    }

    return 0;
}
*/

