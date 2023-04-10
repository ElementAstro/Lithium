/*
 * basic_device.hpp
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

#pragma once

#include <string>

#include "task/camera_task.hpp"

namespace OpenAPT {

    enum class DeviceType {
        Camera,
        Telescope,
        Focuser,
        FilterWheel,
        Solver,
        Guider,
        NumDeviceTypes
    };

    static constexpr int DeviceTypeCount = 6;

    enum class DeviceStatus {
        Unconnected,
        Connected,
        Disconnected
    };

    struct CameraFrame {
        int X = 0;
        int Y = 0;
        double PixelX = 0;
        double PixelY = 0;
    };

    class Device {
        public:
    
            explicit Device(const std::string& name);
            virtual ~Device();

            virtual bool connect(std::string name) {}
            virtual bool disconnect() {}
            virtual bool reconnect() {}
            virtual bool scanForAvailableDevices() {}

            virtual bool getSettings() {}
            virtual bool saveSettings() {}
            virtual bool getParameter(const std::string& paramName, std::string& paramValue) {}
            virtual bool setParameter(const std::string& paramName, const std::string& paramValue) {}

            virtual std::string getName() const {
                return _name;
            }
            virtual bool setName(const std::string& name) {
                _name = name;
            }
            std::string getId() const {
                return device_name;
            }
            virtual bool setId(int id) {
                id = id;
            }

            virtual bool runSimpleTask(std::string task) {
                return true;
            }

        public:
            std::string _name;
            int             _id;
            std::string     device_name;
            std::string     description;

            std::string     configPath;

            std::string     hostname = "127.0.0.1";
            int             port = 7624;

            bool            is_connected;    
    };


    class Camera : public Device
    {

        public:

            Camera(const std::string& name);
            ~Camera();

            virtual bool startExposure(int duration_ms) {}
            virtual bool stopExposure() {}
            virtual bool waitForExposureComplete() {}
            //virtual bool readImage(Image& image);

            virtual bool startLiveView() {}
            virtual bool stopLiveView() {}
            //virtual bool readLiveView(Image& image);

            bool isCoolingAvailable() { return can_cooling; }
            bool isCoolingOn() { return is_cooling; }
            virtual bool setCoolingOn(bool on) {}
            virtual bool setTemperature(double temperature) {}
            virtual double getTemperature() {}

            bool isShutterAvailable() { return has_shutter; }
            bool isShutterOpen() { return is_shutter_closed; }
            virtual bool setShutterOpen(bool open) {}

            bool isSubframeEnabled() { return is_subframe; }
            bool setSubframeEnabled(bool enabled) {}
            //virtual bool setSubframe(const ImageRect& rect);

            bool isBinningSupported(int binning) { return can_binning; }
            int getMaxBinning() { return max_binning; }
            int getBinningX() { return binning_x; }
            int getBinningY() { return binning_y; }
            virtual bool setBinning(int binning) {}

            bool isGainSupported(int gain) { return can_gain; }
            int getMaxGain() { return max_gain; }
            int getGain() { return gain; }
            virtual bool setGain(int gain) {}

            bool isOffsetSupported(int offset) { return can_offset; }
            int getMaxOffset() { return max_offset; }
            int getOffset() { return offset; }
            virtual bool setOffset(int offset) {}

            virtual bool getROIFrame() {}
            virtual bool setROIFrame(int start_x, int start_y, int frame_x, int frame_y) {}

            std::shared_ptr<OpenAPT::SingleShotTask> SingleShotTask() {}

        public:

            static const double UnknownPixelSize;

            bool            is_connected;
            bool            is_exposuring;
            bool            is_video;
            bool            is_color;

            bool            can_gain;
            int             gain;
            int             max_gain;

            bool            can_offset;
            int             offset;
            int             max_offset;

            bool            has_shutter;
            bool            is_shutter_closed;          

            bool            has_subframe;
            bool            is_subframe;        

            bool            can_binning;
            int             binning_x;
            int             binning_y;           
            int             max_binning;
            int             min_binning;

            int             read_delay;

            bool            can_cooling;
            bool            is_cooling;
            double          current_temperature;
            double          current_power;

            double          pixel;
            double          pixel_x;
            double          pixel_y;
            int             pixel_depth;
            int             frame_x;
            int             frame_y;
            int             max_frame_x;
            int             max_frame_y;
            int             start_x;
            int             start_y;

    };

    class Telescope : public Device
    {
        public:

            Telescope(const std::string& name);
            ~Telescope();
    };
} // namespace OpenAPT
