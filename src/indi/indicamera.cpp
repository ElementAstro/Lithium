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

namespace OpenAPT {
    
    void INDICamera::newDevice(INDI::BaseDevice *dp)
    {
        if (strcmp(dp->getDeviceName(), device_name.c_str()) == 0)
        {
            camera_device = dp;
        }
    }

    void INDICamera::newSwitch(ISwitchVectorProperty *svp)
    {
        spdlog::debug("INDI Camera Received Switch: {} = {}", svp->name, svp->sp->s);

        if (strcmp(svp->name, "CONNECTION") == 0)
        {
            ISwitch *connectswitch = IUFindSwitch(svp, "CONNECT");
            if (connectswitch->s == ISS_ON)
            {
                is_connected = true;
                spdlog::info("INDI Camera is connected");
            }
            else
            {
                if (is_ready)
                {
                    ClearStatus();
                    spdlog::info("INDICamera is disconnected");
                }
            }
        }
    }

    void INDICamera::newMessage(INDI::BaseDevice *dp, int messageID)
    {
        spdlog::debug("INDI Camera Received message: {}", dp->messageQueue(messageID));
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
        spdlog::debug("INDI Camera Received Number: {} = {} state = {}", nvp->name, os.str().c_str(), StateStr(nvp->s));

        if (nvp == ccdinfo_prop)
        {
            pixel = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE")->value;
            pixel_x = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE_X")->value;
            pixel_y = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE_Y")->value;
            max_frame_x = IUFindNumber(ccdinfo_prop, "CCD_MAX_X")->value;
            max_frame_y = IUFindNumber(ccdinfo_prop, "CCD_MAX_Y")->value;
            pixel_depth = IUFindNumber(ccdinfo_prop, "CCD_BITSPERPIXEL")->value;
        }
        else if (nvp == binning_prop)
        {
            binning_x = IUFindNumber(binning_prop, "HOR_BIN")->value;
            binning_y = IUFindNumber(binning_prop, "VER_BIN")->value;
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
        spdlog::debug("INDI Camera Received Text: {} = {}\n", tvp->name, tvp->tp->text);
    }

    void INDICamera::newBLOB(IBLOB *bp)
    {
        // we go here every time a new blob is available
        // this is normally the image from the camera

        spdlog::debug("INDI Camera Received BLOB {} len = {} size = {}\n", bp->name, bp->bloblen, bp->size);

        if (expose_prop)
        {
            if (bp->name == indi_blob_name.c_str())
            {
                //updateLastFrame(bp);
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

        spdlog::debug("INDI Camera Property: {}", property->getName());

        if (Proptype == INDI_BLOB)
        {
            spdlog::debug("INDI Camera Found BLOB property for {} {}\n", property->getDeviceName(), PropName);

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
            spdlog::debug("INDI Camera Found CCD_EXPOSURE for {} {}\n", property->getDeviceName(), PropName);
            expose_prop = property->getNumber();
        }
        else if (PropName == indi_camera_cmd + "FRAME" && Proptype == INDI_NUMBER)
        {
            spdlog::debug("INDI Camera Found CCD_FRAME for {} {}\n", property->getDeviceName(), PropName);
            frame_prop = property->getNumber();
            indi_frame_x = IUFindNumber(frame_prop, "X");
            indi_frame_y = IUFindNumber(frame_prop, "Y");
            indi_frame_width = IUFindNumber(frame_prop, "WIDTH");
            indi_frame_height = IUFindNumber(frame_prop, "HEIGHT");
            newNumber(frame_prop);
        }
        else if (PropName == indi_camera_cmd + "FRAME_TYPE" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("INDI Camera Found CCD_FRAME_TYPE for {} {}\n", property->getDeviceName(), PropName);
            frame_type_prop = property->getSwitch();
        }
        else if (PropName == indi_camera_cmd + "BINNING" && Proptype == INDI_NUMBER)
        {
            spdlog::debug("INDI Camera Found CCD_BINNING for {} {}\n", property->getDeviceName(), PropName);
            binning_prop = property->getNumber();
            indi_binning_x = IUFindNumber(binning_prop, "HOR_BIN");
            indi_binning_y = IUFindNumber(binning_prop, "VER_BIN");
            newNumber(binning_prop);
        }
        else if (PropName == indi_camera_cmd + "CFA" && Proptype == INDI_TEXT)
        {
            spdlog::debug("INDI Camera Found CCD_CFA for {} {}\n", property->getDeviceName(), PropName);
            ITextVectorProperty *cfa_prop = property->getText();
            IText *cfa_type = IUFindText(cfa_prop, "CFA_TYPE");
            if (cfa_type && cfa_type->text && *cfa_type->text)
            {
                spdlog::debug("INDI Camera CFA_TYPE is {}\n", cfa_type->text);
                is_color = true;
            }
        }
        else if (PropName == indi_camera_cmd + "VIDEO_STREAM" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("INDI Camera Found Video {} {}\n", property->getDeviceName(), PropName);
            video_prop = property->getSwitch();
        }
        else if (PropName == "VIDEO_STREAM" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("INDI Camera Found Video {} {}\n", property->getDeviceName(), PropName);
            video_prop = property->getSwitch();
        }
        else if (PropName == "DEVICE_PORT" && Proptype == INDI_TEXT)
        {
            spdlog::debug("INDI Camera Found device port for {} \n", property->getDeviceName());
            camera_port = property->getText();
        }
        else if (PropName == "CONNECTION" && Proptype == INDI_SWITCH)
        {
            spdlog::debug("INDI Camera Found CONNECTION for {} {}\n", property->getDeviceName(), PropName);
            connection_prop = property->getSwitch();
            ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
            is_connected = (connectswitch->s == ISS_ON);
        }
        else if (PropName == "DRIVER_INFO" && Proptype == INDI_TEXT)
        {
        }
        else if (PropName == indi_camera_cmd + "INFO" && Proptype == INDI_NUMBER)
        {
            ccdinfo_prop = property->getNumber();
            newNumber(ccdinfo_prop);
        }
    }

    void INDICamera::IndiServerConnected()
    {
        spdlog::debug("INDI Camera connection succeeded");
        is_connected = true;
    }

    void INDICamera::IndiServerDisconnected(int exit_code)
    {
        spdlog::debug("INDI Camera: serverDisconnected");
        // after disconnection we reset the connection status and the properties pointers
        ClearStatus();
        // in case the connection lost we must reset the client socket
        if (exit_code == -1)
            spdlog::debug("INDI server disconnected");
    }

    void INDICamera::removeDevice(INDI::BaseDevice *dp)
    {
        ClearStatus();
        spdlog::info("INDI camera disconnected");
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

    INDICamera::INDICamera(const std::string& name) : Camera(name)
    {

    }

    INDICamera::~INDICamera()
    {

    }

    bool INDICamera::connect(std::string name)
    {
        spdlog::debug("Trying to connect to {}", name);
        setServer(hostname.c_str(),port);
        // Receive messages only for our camera.
        watchDevice(name.c_str());
        // Connect to server.
        if (connectServer())
        {
            spdlog::debug("INDI Camera: connectServer done ready = {}\n", is_ready);
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

    bool INDICamera::getROIFrame()
    {
        return true;
    }

    bool INDICamera::setROIFrame(int start_x, int start_y, int frame_x, int frame_y)
    {
        return true;
    }
}

#endif