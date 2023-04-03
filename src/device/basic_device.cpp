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

    class Device 
    {
        public:

            explicit Device();
            virtual ~Device();

            virtual bool Connect();
            virtual bool Disconnect();
            virtual bool Reconnect();
            virtual bool Scanning();

            virtual bool GetSettings();
            virtual bool SaveSettings();
            virtual bool GetParameter();
            virtual bool SetParameter();

            virtual std::string GetName();
            virtual bool SetName();
            virtual int GetId();
            virtual bool SetId();
        
        public:

            int             id;
            std::string     name;
            std::string     description;

            std::string     config_path;

            std::string     hostname;
            int             port;

            bool            is_connected;
            
            std::vector<std::string> available_devices;
    };

    class Camera : public Device
    {

        public:

            explicit Camera();
            virtual ~Camera();

            virtual bool StartExposure();
            virtual bool StopExposure();
            virtual bool WaitForExposure();
            virtual bool ReadAfterExposure();

            virtual bool StartVideoCapturing();
            virtual bool StopVideoCapturing();
            virtual bool ReadStreamFromCamera();

            virtual bool IsCameraConnected();
            virtual bool IsCameraExposuring();
            virtual bool IsCameraVideo();
            virtual bool IsCameraCooling();

            virtual bool CanCameraGain();
            virtual int CurrentCameraGain();
            virtual int MaxCameraGain();
            virtual int MinCameraGain();

            virtual bool CanCameraOffset();
            virtual int CurrentCameraOffset();
            virtual int MaxCameraOffset();
            virtual int MinCameraOffset();

            virtual bool CanCameraBinningt();
            virtual int CurrentCameraBinningt();
            virtual int MaxCameraBinningt();
            virtual int MinCameraBinningt();

            virtual bool IsCameraHasShutter();
            virtual bool IsCameraShutterClosed();

            virtual bool IsCameraSubframe();
            virtual bool CanCameraSubframe();

            virtual int GetReadDelay();

            virtual bool CanCameraCooling();
            virtual double CurrentTemperature();
            virtual double CurrentPower();

            virtual CameraFrame GetCameraFrame();

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

