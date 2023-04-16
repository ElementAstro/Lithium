/*
 * indicamera.cpp
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

#define INDI_CAMERA

#ifdef INDI_CAMERA

#include "indicamera.hpp"

#include <spdlog/spdlog.h>

namespace OpenAPT
{

    void INDICamera::newDevice(INDI::BaseDevice *dp)
    {
        if (strcmp(dp->getDeviceName(), device_name.c_str()) == 0)
        {
            camera_device = dp;
        }
    }

    void INDICamera::newSwitch(ISwitchVectorProperty *svp)
    {
        spdlog::debug("{} Received Switch: {} = {}", _name, svp->name, svp->sp->s);

        if (strcmp(svp->name, "CONNECTION") == 0)
        {
            ISwitch *connectswitch = IUFindSwitch(svp, "CONNECT");
            if (connectswitch->s == ISS_ON)
            {
                is_connected = true;
                spdlog::info("{} is connected", _name);
            }
            else
            {
                if (is_ready)
                {
                    ClearStatus();
                    spdlog::info("{} is disconnected", _name);
                }
            }
        }
    }

    void INDICamera::newMessage(INDI::BaseDevice *dp, int messageID)
    {
        spdlog::debug("{} Received message: {}", _name, dp->messageQueue(messageID));
    }

    inline static const char *StateStr(IPState st)
    {
        switch (st)
        {
        default:
        case IPS_IDLE:
            return "Idle";
        case IPS_OK:
            return "Ok";
        case IPS_BUSY:
            return "Busy";
        case IPS_ALERT:
            return "Alert";
        }
    }

    void INDICamera::newNumber(INumberVectorProperty *nvp)
    {
        if (strcmp(nvp->name, "CCD_EXPOSURE") == 0)
        {
            static double s_lastval;
            if (nvp->np->value > 0.0 && fabs(nvp->np->value - s_lastval) < 0.5)
                return;
            s_lastval = nvp->np->value;
        }
        std::ostringstream os;
        for (int i = 0; i < nvp->nnp; i++)
        {
            if (i)
                os << ',';
            os << nvp->np[i].name << ':' << nvp->np[i].value;
        }
        spdlog::debug("{} Received Number: {} = {} state = {}", _name, nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == ccdinfo_prop)
        {
            pixel = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE")->value;
            pixel_x = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE_X")->value;
            pixel_y = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE_Y")->value;
            max_frame_x = IUFindNumber(ccdinfo_prop, "CCD_MAX_X")->value;
            max_frame_y = IUFindNumber(ccdinfo_prop, "CCD_MAX_Y")->value;
            pixel_depth = IUFindNumber(ccdinfo_prop, "CCD_BITSPERPIXEL")->value;
            spdlog::debug("{} pixel {} pixel_x {} pixel_y {} max_frame_x {} max_frame_y {} pixel_depth {}", _name, pixel, pixel_x, pixel_y, max_frame_x, max_frame_y, pixel_depth);
        }
        else if (nvp == binning_prop)
        {
            binning_x = IUFindNumber(binning_prop, "HOR_BIN")->value;
            binning_y = IUFindNumber(binning_prop, "VER_BIN")->value;
            spdlog::debug("{} binning_x {} binning_y {}", _name, binning_x, binning_y);
        }
        else if (nvp == frame_prop)
        {
            frame_x = IUFindNumber(frame_prop, "HEIGHT")->value;
            frame_y = IUFindNumber(frame_prop, "WIDTH")->value;
            start_x = IUFindNumber(frame_prop, "X")->value;
            start_y = IUFindNumber(frame_prop, "Y")->value;
        }
        else if (nvp == temperature_prop)
        {
            current_temperature = IUFindNumber(temperature_prop, "CCD_TEMPERATURE_VALUE")->value;
        }
        else if (nvp == gain_prop)
        {
            gain = IUFindNumber(gain_prop, "GAIN")->value;
        }
        else if (nvp == offset_prop)
        {
            offset = IUFindNumber(offset_prop, "OFFSET")->value;
        }
    }

    void INDICamera::newText(ITextVectorProperty *tvp)
    {
        spdlog::debug("{} Received Text: {} = {}", _name, tvp->name, tvp->tp->text);
    }

    void INDICamera::newBLOB(IBLOB *bp)
    {
        // we go here every time a new blob is available
        // this is normally the image from the camera

        spdlog::debug("{} Received BLOB {} len = {} size = {}", _name, bp->name, bp->bloblen, bp->size);

        if (expose_prop)
        {
            if (bp->name == indi_blob_name.c_str())
            {
                // updateLastFrame(bp);
            }
        }
        else if (video_prop)
        {
        }
    }

    void INDICamera::newProperty(INDI::Property *property)
    {
        std::string PropName(property->getName());
        INDI_PROPERTY_TYPE Proptype = property->getType();

        spdlog::debug("{} Property: {}", _name, property->getName());

        if (Proptype == INDI_BLOB)
        {
            spdlog::debug("{} Found BLOB property for {} {}", _name, property->getDeviceName(), PropName);

            if (PropName == indi_blob_name.c_str())
            {
                has_blob = 1;
                // set option to receive blob and messages for the selected CCD
                setBLOBMode(B_ALSO, device_name.c_str(), indi_blob_name.c_str());

#ifdef INDI_SHARED_BLOB_SUPPORT
                // Allow faster mode provided we don't modify the blob content or free/realloc it
                enableDirectBlobAccess(device_name.c_str(), indi_blob_name.c_str());
#endif
            }
        }
        else if (PropName == indi_camera_cmd + "EXPOSURE" && Proptype == INDI_NUMBER)
        {
            spdlog::debug("{} Found CCD_EXPOSURE for {} {}", _name, property->getDeviceName(), PropName);
            expose_prop = property->getNumber();
        }
        else if (PropName == indi_camera_cmd + "FRAME" && Proptype == INDI_NUMBER)
        {
            spdlog::debug("{} Found CCD_FRAME for {} {}", _name, property->getDeviceName(), PropName);
            frame_prop = property->getNumber();
            indi_frame_x = IUFindNumber(frame_prop, "X");
            indi_frame_y = IUFindNumber(frame_prop, "Y");
            indi_frame_width = IUFindNumber(frame_prop, "WIDTH");
            indi_frame_height = IUFindNumber(frame_prop, "HEIGHT");
            newNumber(frame_prop);
        }
        else if (PropName == indi_camera_cmd + "FRAME_TYPE" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("{} Found CCD_FRAME_TYPE for {} {}", _name, property->getDeviceName(), PropName);
            frame_type_prop = property->getSwitch();
        }
        else if (PropName == indi_camera_cmd + "BINNING" && Proptype == INDI_NUMBER)
        {
            spdlog::debug("{} Found CCD_BINNING for {} {}", _name, property->getDeviceName(), PropName);
            binning_prop = property->getNumber();
            indi_binning_x = IUFindNumber(binning_prop, "HOR_BIN");
            indi_binning_y = IUFindNumber(binning_prop, "VER_BIN");
            newNumber(binning_prop);
        }
        else if (PropName == indi_camera_cmd + "CFA" && Proptype == INDI_TEXT)
        {
            spdlog::debug("{} Found CCD_CFA for {} {}", _name, property->getDeviceName(), PropName);
            ITextVectorProperty *cfa_prop = property->getText();
            IText *cfa_type = IUFindText(cfa_prop, "CFA_TYPE");
            if (cfa_type && cfa_type->text && *cfa_type->text)
            {
                spdlog::debug("{} CFA_TYPE is {}", _name, cfa_type->text);
                is_color = true;
            }
        }
        else if (PropName == indi_camera_cmd + "VIDEO_STREAM" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("{} Found Video {} {}", _name, property->getDeviceName(), PropName);
            video_prop = property->getSwitch();
        }
        else if (PropName == "VIDEO_STREAM" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("{} Found Video {} {}", _name, property->getDeviceName(), PropName);
            video_prop = property->getSwitch();
        }
        else if (PropName == "DEVICE_PORT" && Proptype == INDI_TEXT)
        {
            spdlog::debug("{} Found device port for {} ", _name, property->getDeviceName());
            camera_port = property->getText();
        }
        else if (PropName == "CONNECTION" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("{} Found CONNECTION for {} {}", _name, property->getDeviceName(), PropName);
            connection_prop = property->getSwitch();
            ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
            is_connected = (connectswitch->s == ISS_ON);
            if (!is_connected)
            {
                connection_prop->sp->s = ISS_ON;
                sendNewSwitch(connection_prop);
            }
            spdlog::debug("{} Connected {}", _name, is_connected);
        }
        else if (PropName == "DRIVER_INFO" && Proptype == INDI_TEXT)
        {
            device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
            indi_camera_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
            spdlog::debug("Camera Name : {} connected exec {}", _name, device_name, indi_camera_exec);
        }
        else if (PropName == indi_camera_cmd + "INFO" && Proptype == INDI_NUMBER)
        {
            ccdinfo_prop = property->getNumber();
            newNumber(ccdinfo_prop);
        }
    }

    void INDICamera::IndiServerConnected()
    {
        spdlog::debug("{} connection succeeded", _name);
        is_connected = true;
    }

    void INDICamera::IndiServerDisconnected(int exit_code)
    {
        spdlog::debug("{}: serverDisconnected", _name);
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            spdlog::debug("{} : INDI server disconnected", _name);
    }

    void INDICamera::removeDevice(INDI::BaseDevice *dp)
    {
        ClearStatus();
        spdlog::info("{} disconnected", _name);
    }

    void INDICamera::ClearStatus()
    {
        connection_prop = nullptr;
        expose_prop = nullptr;
        frame_prop = nullptr;
        frame_type_prop = nullptr;
        ccdinfo_prop = nullptr;
        binning_prop = nullptr;
        video_prop = nullptr;
        camera_port = nullptr;
        camera_device = nullptr;
    }

    INDICamera::INDICamera(const std::string &name) : Camera(name)
    {
    }

    INDICamera::~INDICamera()
    {
    }

    bool INDICamera::connect(std::string name)
    {
        spdlog::debug("Trying to connect to {}", name);
        setServer(hostname.c_str(), port);
        // Receive messages only for our camera.
        watchDevice(name.c_str());
        // Connect to server.
        if (connectServer())
        {
            spdlog::debug("{}: connectServer done ready = {}", _name, is_ready);
            connectDevice(name.c_str());
            return !is_ready;
        }
        return false;
    }

    bool INDICamera::disconnect()
    {
        return true;
    }

    bool INDICamera::reconnect()
    {
        return true;
    }

    bool INDICamera::scanForAvailableDevices()
    {
        return true;
    }

    bool INDICamera::startExposure(int duration_ms)
    {
        return true;
    }

    bool INDICamera::stopExposure()
    {
        return true;
    }

    bool INDICamera::waitForExposureComplete()
    {
        return true;
    }

    // bool readImage(Image& image);

    bool INDICamera::startLiveView()
    {
        return true;
    }

    bool INDICamera::stopLiveView()
    {
        return true;
    }
    // bool readLiveView(Image& image);

    bool INDICamera::setCoolingOn(bool on)
    {
        return true;
    }

    bool INDICamera::setTemperature(double temperature)
    {
        return true;
    }

    double INDICamera::getTemperature()
    {
        return true;
    }

    bool INDICamera::setShutterOpen(bool open)
    {
        return true;
    }

    bool INDICamera::setBinning(int binning)
    {
        return true;
    }

    bool INDICamera::setGain(int gain)
    {
        return true;
    }

    bool INDICamera::setOffset(int offset)
    {
        return true;
    }

    bool INDICamera::setROIFrame(int start_x, int start_y, int frame_x, int frame_y)
    {
        return true;
    }

    std::shared_ptr<OpenAPT::SimpleTask> INDICamera::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
    {
        if (task_name == "SingleShot")
        {
            spdlog::debug("SingleShot task with parameters : {}", params.dump());
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &tparams)
                {
                    spdlog::debug("{} SingleShot task is called", this->_name);
                },
                {params}));
        }
        else if (task_name == "GetGain")
        {
            return std::shared_ptr<OpenAPT::SimpleTask>(new OpenAPT::SimpleTask(
                [this](const nlohmann::json &tparams)
                {
                    spdlog::debug("{} SingleShot task is called", this->_name);
                },
                {params}));
        }

        spdlog::error("Unknown type of the {} task : {}", _name, task_name);
        return nullptr;
    }

    std::shared_ptr<OpenAPT::ConditionalTask> INDICamera::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
    {
        return nullptr;
    }

    std::shared_ptr<OpenAPT::LoopTask> INDICamera::getLoopTask(const std::string &task_name, const nlohmann::json &params)
    {
        return nullptr;
    }
}

#endif