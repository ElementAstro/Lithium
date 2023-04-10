/*
 * indicamera.hpp
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

Date: 2023-4-9

Description: INDI Camera

**************************************************/

#pragma once


#include "api/indiclient.hpp"
#include "device/basic_device.hpp"
#include "task/camera_task.hpp"

#include <libindi/basedevice.h>
#include <libindi/indiproperty.h>

#include <string>

#include <spdlog/spdlog.h>

namespace OpenAPT {
    class INDICamera : public Camera, public OpenAptIndiClient
    {
        // INDI Parameters
    private:
        ISwitchVectorProperty *connection_prop;
        INumberVectorProperty *expose_prop;
        INumberVectorProperty *frame_prop;
        INumberVectorProperty *temperature_prop;
        INumberVectorProperty *gain_prop;
        INumberVectorProperty *offset_prop;
        INumber *indi_frame_x;
        INumber *indi_frame_y;
        INumber *indi_frame_width;
        INumber *indi_frame_height;
        ISwitchVectorProperty *frame_type_prop;
        INumberVectorProperty *ccdinfo_prop;
        INumberVectorProperty *binning_prop;
        INumber *indi_binning_x;
        INumber *indi_binning_y;
        ISwitchVectorProperty *video_prop;
        ITextVectorProperty *camera_port;
        INDI::BaseDevice *camera_device;

        bool is_ready;
        bool has_blob;

        std::string indi_camera_cmd;
        std::string indi_blob_name;

    public:

        INDICamera(const std::string& name);
        ~INDICamera();

        bool connect(std::string name) override;
        bool disconnect() override;
        bool reconnect() override;
        bool scanForAvailableDevices() override;

        bool startExposure(int duration_ms) override;
        bool stopExposure() override;
        bool waitForExposureComplete() override;
        // bool readImage(Image& image);

        bool startLiveView() override;
        bool stopLiveView() override;
        // bool readLiveView(Image& image);

        bool setCoolingOn(bool on) override;
        bool setTemperature(double temperature) override;
        double getTemperature();

        bool setShutterOpen(bool open) override;

        // bool setSubframe(const ImageRect& rect);

        bool setBinning(int binning) override;

        bool setGain(int gain) override;

        bool setOffset(int offset) override;

        bool getROIFrame() override;
        bool setROIFrame(int start_x, int start_y, int frame_x, int frame_y) override;

        std::shared_ptr<OpenAPT::SingleShotTask> SingleShotTask() {
            std::shared_ptr<OpenAPT::SingleShotTask> SingleShotT(new OpenAPT::SingleShotTask(
                [](const nlohmann::json &params) 
                { 
                    std::cout << "Single shot task is running." << std::endl;
                    std::cout << "The parameters are: " << params.dump() << std::endl;
                }, 
                {{"threshold", 10}}
            ));
            return SingleShotT;
        }

    protected:
        void ClearStatus();

        // INDI Client API
    protected:
        void newDevice(INDI::BaseDevice *dp) override;
        void removeDevice(INDI::BaseDevice *dp) override;
        void newProperty(INDI::Property *property) override;
        void removeProperty(INDI::Property *property) override {}
        void newBLOB(IBLOB *bp) override;
        void newSwitch(ISwitchVectorProperty *svp) override;
        void newNumber(INumberVectorProperty *nvp) override;
        void newMessage(INDI::BaseDevice *dp, int messageID) override;
        void newText(ITextVectorProperty *tvp) override;
        void newLight(ILightVectorProperty *lvp) override {}
        void IndiServerConnected() override;
        void IndiServerDisconnected(int exit_code) override;
    };
}