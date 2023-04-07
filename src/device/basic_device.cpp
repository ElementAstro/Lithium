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

struct CameraFrame {
    int X = 0;
    int Y = 0;
    double PixelX = 0;
    double PixelY = 0;
};

namespace OpenAPT
{

    class Device {
        public:
            explicit Device();
            virtual ~Device();

            virtual bool connect();
            virtual bool disconnect();
            virtual bool reconnect();
            virtual bool scanForAvailableDevices();

            virtual bool getSettings();
            virtual bool saveSettings();
            virtual bool getParameter(const std::string& paramName, std::string& paramValue);
            virtual bool setParameter(const std::string& paramName, const std::string& paramValue);

            virtual std::string getName();
            virtual bool setName(const std::string& name);
            virtual int getId();
            virtual bool setId(int id);

        public:
            int             id;
            std::string     name;
            std::string     description;

            std::string     configPath;

            std::string     hostname;
            int             port;

            bool            isConnected;    
    };


    class Camera : public Device
    {

        public:

            explicit Camera();
            virtual ~Camera();

            virtual bool startExposure(int duration_ms);
            virtual bool stopExposure();
            virtual bool waitForExposureComplete();
            //virtual bool readImage(Image& image);

            virtual bool startLiveView();
            virtual bool stopLiveView();
            //virtual bool readLiveView(Image& image);

            virtual bool isCoolingAvailable() const;
            virtual bool isCoolingOn() const;
            virtual bool setCoolingOn(bool on);
            virtual bool setTemperature(double temperature);
            virtual double getTemperature() const;

            virtual bool isShutterAvailable() const;
            virtual bool isShutterOpen() const;
            virtual bool setShutterOpen(bool open);

            virtual bool isSubframeSupported() const;
            virtual bool isSubframeEnabled() const;
            virtual bool setSubframeEnabled(bool enabled);
            //virtual bool setSubframe(const ImageRect& rect);

            virtual bool isBinningSupported(int binning) const;
            virtual int getMaxBinning() const;
            virtual int getBinning() const;
            virtual bool setBinning(int binning);

            virtual bool isGainSupported(int gain) const;
            virtual int getMaxGain() const;
            virtual int getGain() const;
            virtual bool setGain(int gain);

            virtual bool isOffsetSupported(int offset) const;
            virtual int getMaxOffset() const;
            virtual int getOffset() const;
            virtual bool setOffset(int offset);    

        public:

            static const double UnknownPixelSize;

            bool            is_exposuring;
            bool            is_video;

            bool            can_gain;
            int             gain;
            int             max_gain;
            int             min_gain;

            bool            can_offset;
            int             offset;
            int             max_offset;
            int             min_offset;

            bool            has_shutter;
            bool            is_shutter_closed;          

            bool            has_subframe;
            bool            is_subframe;        

            bool            can_binning;
            int             binning;            
            int             max_binning;
            int             min_binning;

            int             read_delay;

            bool            can_cooling;
            bool            is_cooling;
            double          current_temperature;
            double          current_power;

            double          pixel_x;
            double          pixel_y;
            int             frame_x;
            int             frame_y;

            

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

