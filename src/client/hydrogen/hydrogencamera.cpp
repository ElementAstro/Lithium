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

Description: Hydorgen Camera

**************************************************/

#include "hydrogencamera.hpp"

#include "loguru/loguru.hpp"

void HydrogenCamera::newDevice(HYDROGEN::BaseDevice *dp)
{
    if (strcmp(dp->getDeviceName(), device_name.c_str()) == 0)
    {
        camera_device = dp;
    }
}

void HydrogenCamera::newSwitch(ISwitchVectorProperty *svp)
{
    std::string name = svp->name;

    DLOG_F(INFO, "{} Received Switch: {}", _name, name);

    if (name == "CONNECTION")
    {
        if (auto connectswitch = IUFindSwitch(svp, "CONNECT"); connectswitch->s == ISS_ON)
        {
            setProperty("connect", true);
            device_info["connect"] = true;
            DLOG_F(INFO, "{} is connected", _name);
        }
        else
        {
            if (is_ready)
            {
                ClearStatus();
                device_info["connect"] = false;
                DLOG_F(INFO, "{} is disconnected", _name);
            }
        }
    }
    else if (name == "DEBUG")
    {
        if (auto debugswitch = IUFindSwitch(svp, "ENABLE"); debugswitch->s == ISS_ON)
        {
            setProperty("debug", true);
            device_info["debug"] = true;
            DLOG_F(INFO, "DEBUG mode of {} is enabled", _name);
        }
        else
        {
            setProperty("debug", false);
            device_info["debug"] = false;
            DLOG_F(INFO, "DEBUG mode of {} is disabled", _name);
        }
    }
    else if (name == "CCD_FRAME_TYPE")
    {
        std::string type;
        if (auto lightswitch = IUFindSwitch(svp, "FRAME_LIGHT"); lightswitch->s == ISS_ON)
            type = "Light";
        else if (auto darkswitch = IUFindSwitch(svp, "FRAME_DARK"); darkswitch->s == ISS_ON)
            type = "Dark";
        else if (auto flatswitch = IUFindSwitch(svp, "FRAME_FLAT"); flatswitch->s == ISS_ON)
            type = "Flat";
        else if (auto biasswitch = IUFindSwitch(svp, "FRAME_BIAS"); biasswitch->s == ISS_ON)
            type = "Bias";
        setProperty("frame_type", type);
        device_info["frame_type"] = type;
        DLOG_F(INFO, "Current frame type of {} is {}", _name, device_info["frame_type"].dump());
    }
    else if (name == "CCD_TRANSFER_FORMAT")
    {
        std::string format;
        if (auto fitsswitch = IUFindSwitch(svp, "FORMAT_FITS"); fitsswitch->s == ISS_ON)
            format = "Fits";
        else if (auto natswitch = IUFindSwitch(svp, "FORMAT_NATIVE"); natswitch->s == ISS_ON)
            format = "Raw";
        else if (auto xisfswitch = IUFindSwitch(svp, "FORMAT_XISF"); xisfswitch->s == ISS_ON)
            format = "Xisf";
        setProperty("frame_format", foramt);
        device_info["frame_format"] = format;
        DLOG_F(INFO, "Current frame format of {} is {}", _name, device_info["frame_format"].dump());
    }
    else if (name == "CCD_ABORT_EXPOSURE")
    {
        if (auto abortswitch = IUFindSwitch(svp, "ABORT_EXPOSURE"); abortswitch->s == ISS_ON)
        {
            setProperty("is_exposure", false);
            device_info["is_exposure"] = false;
            DLOG_F(INFO, "{} is stopped", _name);
            is_exposuring = false;
        }
    }
    else if (name == "UPLOAD_MODE")
    {
        std::string mode;

        if (auto clientswitch = IUFindSwitch(svp, "UPLOAD_CLIENT"); clientswitch->s == ISS_ON)
            mode = "Client";
        else if (auto localswitch = IUFindSwitch(svp, "UPLOAD_LOCAL"); localswitch->s == ISS_ON)
            mode = "Local";
        else if (auto bothswitch = IUFindSwitch(svp, "UPLOAD_BOTH"); bothswitch->s == ISS_ON)
            mode = "Both";

        device_info["network"]["mode"] = mode;
        DLOG_F(INFO, "Current upload mode of {} is {}", _name, device_info["network"]["mode"].dump());
    }
    else if (name == "CCD_FAST_TOGGLE")
    {
        bool mode;

        if (auto enabledswitch = IUFindSwitch(svp, "LITHIUM_ENABLED"); enabledswitch->s == ISS_ON)
            mode = true;
        else if (auto disabledswitch = IUFindSwitch(svp, "LITHIUM_DISABLED"); disabledswitch->s == ISS_ON)
            mode = false;

        device_info["frame"]["fast_read"] = mode;
        DLOG_F(INFO, "Current readout mode of {} is {}", _name, device_info["frame"]["fast_read"].dump());
    }
    else if (name == "CCD_VIDEO_STREAM")
    {
        if (auto onswitch = IUFindSwitch(svp, "STREAM_ON"); onswitch->s == ISS_ON)
        {
            device_info["video"]["is_video"] = true;
            is_video = true;
            DLOG_F(INFO, "{} start video capture", _name);
        }
        else if (auto offswitch = IUFindSwitch(svp, "STREAM_OFF"); offswitch->s == ISS_ON)
        {
            device_info["video"]["is_video"] = false;
            is_video = false;
            DLOG_F(INFO, "{} stop video capture", _name);
        }
    }
    else if (name == "FLIP")
    {
    }
}

void HydrogenCamera::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
{
    DLOG_F(INFO, "{} Received message: {}", _name, dp->messageQueue(messageID));
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

void HydrogenCamera::newNumber(INumberVectorProperty *nvp)
{
    using namespace std::string_literals;
    const std::string name = nvp->name;
    if (name == "CCD_EXPOSURE")
    {
        const double exposure = nvp->np->value;
        device_info["exposure"]["current"] = exposure;
        DLOG_F(INFO, "Current CCD_EXPOSURE for {} is {}", _name, exposure);
    }
    else if (name == "CCD_INFO")
    {
        pixel = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE")->value;
        pixel_x = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE_X")->value;
        pixel_y = IUFindNumber(ccdinfo_prop, "CCD_PIXEL_SIZE_Y")->value;
        max_frame_x = IUFindNumber(ccdinfo_prop, "CCD_MAX_X")->value;
        max_frame_y = IUFindNumber(ccdinfo_prop, "CCD_MAX_Y")->value;
        pixel_depth = IUFindNumber(ccdinfo_prop, "CCD_BITSPERPIXEL")->value;

        device_info["frame"] = {
            {"pixel_x", pixel_x},
            {"pixel_y", pixel_y},
            {"pixel_depth", pixel_depth},
            {"max_frame_x", max_frame_x},
            {"max_frame_y", max_frame_y}};
        DLOG_F(INFO, "{} pixel {} pixel_x {} pixel_y {} max_frame_x {} max_frame_y {} pixel_depth {}", _name, pixel, pixel_x, pixel_y, max_frame_x, max_frame_y, pixel_depth);
    }
    else if (name == "BINNING")
    {
        indi_binning_x = IUFindNumber(nvp, "HOR_BIN");
        indi_binning_y = IUFindNumber(nvp, "VER_BIN");
        device_info["exposure"] = {
            {"binning_x", indi_binning_x->value},
            {"binning_y", indi_binning_y->value}};
        DLOG_F(INFO, "Current binning_x and y of {} are {} {}", _name, indi_binning_x->value, indi_binning_y->value);
    }
    else if (name == "FRAME")
    {
        indi_frame_x = IUFindNumber(nvp, "X");
        indi_frame_y = IUFindNumber(nvp, "Y");
        indi_frame_width = IUFindNumber(nvp, "WIDTH");
        indi_frame_height = IUFindNumber(nvp, "HEIGHT");

        device_info["frame"] = {
            {"x", indi_frame_x->value},
            {"y", indi_frame_y->value},
            {"width", indi_frame_width->value},
            {"height", indi_frame_height->value}};
        DLOG_F(INFO, "Current frame of {} are {} {} {} {}", _name, indi_frame_width->value, indi_frame_y->value, indi_frame_width->value, indi_frame_height->value);
    }
    else if (name == "CCD_TEMPERATURE")
    {
        current_temperature = IUFindNumber(nvp, "CCD_TEMPERATURE_VALUE")->value;
        device_info["temperature"]["current"] = current_temperature;
        DLOG_F(INFO, "Current temperature of {} is {}", _name, current_temperature);
    }
    else if (name == "CCD_GAIN")
    {
        gain = IUFindNumber(nvp, "GAIN")->value;
        device_info["exposure"]["gain"] = gain;
        DLOG_F(INFO, "Current camera gain of {} is {}", _name, gain);
    }
    else if (name == "CCD_OFFSET")
    {
        offset = IUFindNumber(nvp, "OFFSET")->value;
        device_info["exposure"]["offset"] = offset;
        DLOG_F(INFO, "Current camera offset of {} is {}", _name, offset);
    }
    else if (name == "POLLING_PERIOD")
    {
        device_info["network"]["period"] = IUFindNumber(nvp, "PERIOD_MS")->value;
        DLOG_F(INFO, "Current period of {} is {}", _name, device_info["network"]["period"].dump());
    }
    else if (name == "LIMITS")
    {
        device_info["limits"]["maxbuffer"] = IUFindNumber(nvp, "LIMITS_BUFFER_MAX")->value;
        DLOG_F(INFO, "Current max buffer of {} is {}", _name, device_info["limits"]["maxbuffer"].dump());
        device_info["limits"]["maxfps"] = IUFindNumber(nvp, "LIMITS_PREVIEW_FPS")->value;
        DLOG_F(INFO, "Current max fps of {} is {}", _name, device_info["limits"]["maxfps"].dump());
    }
    else if (name == "STREAM_DELAY")
    {
        device_info["video"]["delay"] = IUFindNumber(nvp, "STREAM_DELAY_TIME")->value;
        DLOG_F(INFO, "Current stream delay of {} is {}", _name, device_info["video"]["delay"].dump());
    }
    else if (name == "STREAMING_EXPOSURE")
    {
        device_info["video"]["exposure"] = IUFindNumber(nvp, "STREAMING_EXPOSURE_VALUE")->value;
        DLOG_F(INFO, "Current streaming exposure of {} is {}", _name, device_info["video"]["exposure"].dump());
        device_info["video"]["division"] = IUFindNumber(nvp, "STREAMING_DIVISOR_VALUE")->value;
        DLOG_F(INFO, "Current streaming division of {} is {}", _name, device_info["video"]["division"].dump());
    }
    else if (name == "FPS")
    {
        device_info["video"]["fps"] = IUFindNumber(nvp, "EST_FPS")->value;
        DLOG_F(INFO, "Current fps of {} is {}", _name, device_info["video"]["fps"].dump());
        device_info["video"]["avgfps"] = IUFindNumber(nvp, "AVG_FPS")->value;
        DLOG_F(INFO, "Current average fps of {} is {}", _name, device_info["video"]["avgfps"].dump());
    }
}

void HydrogenCamera::newText(ITextVectorProperty *tvp)
{
    DLOG_F(INFO, "{} Received Text: {} = {}", _name, tvp->name, tvp->tp->text);
}

void HydrogenCamera::newBLOB(IBLOB *bp)
{
    // we go here every time a new blob is available
    // this is normally the image from the camera

    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", _name, bp->name, bp->bloblen, bp->size);

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

void HydrogenCamera::newProperty(HYDROGEN::Property *property)
{
    std::string PropName(property->getName());
    LITHIUM_PROPERTY_TYPE Proptype = property->getType();

    // DLOG_F(INFO,"{} Property: {}", _name, property->getName());

    if (Proptype == LITHIUM_BLOB)
    {

        if (PropName == indi_blob_name.c_str())
        {
            has_blob = 1;
            // set option to receive blob and messages for the selected CCD
            setBLOBMode(B_ALSO, device_name.c_str(), indi_blob_name.c_str());

#ifdef LITHIUM_SHARED_BLOB_SUPPORT
            // Allow faster mode provided we don't modify the blob content or free/realloc it
            enableDirectBlobAccess(device_name.c_str(), indi_blob_name.c_str());
#endif
        }
    }
    else if (PropName == indi_camera_cmd + "EXPOSURE" && Proptype == LITHIUM_NUMBER)
    {
        expose_prop = property->getNumber();
        newNumber(expose_prop);
    }
    else if (PropName == indi_camera_cmd + "FRAME" && Proptype == LITHIUM_NUMBER)
    {
        frame_prop = property->getNumber();
        newNumber(frame_prop);
    }
    else if (PropName == indi_camera_cmd + "FRAME_TYPE" && Proptype == LITHIUM_SWITCH)
    {
        frame_type_prop = property->getSwitch();
        newSwitch(frame_type_prop);
    }
    else if (PropName == indi_camera_cmd + "BINNING" && Proptype == LITHIUM_NUMBER)
    {
        binning_prop = property->getNumber();
        newNumber(binning_prop);
    }
    else if (PropName == indi_camera_cmd + "CFA" && Proptype == LITHIUM_TEXT)
    {
        ITextVectorProperty *cfa_prop = property->getText();
        IText *cfa_type = IUFindText(cfa_prop, "CFA_TYPE");
        if (cfa_type && cfa_type->text && *cfa_type->text)
        {
            DLOG_F(INFO, "{} CFA_TYPE is {}", _name, cfa_type->text);
            is_color = true;
        }
    }
    else if (PropName == indi_camera_cmd + "VIDEO_STREAM" && Proptype == LITHIUM_SWITCH)
    {
        video_prop = property->getSwitch();
        newSwitch(video_prop);
    }
    else if (PropName == "STREAM_DELAY" && Proptype == LITHIUM_NUMBER)
    {
        video_delay_prop = property->getNumber();
        newNumber(video_delay_prop);
    }
    else if (PropName == "STREAMING_EXPOSURE" && Proptype == LITHIUM_NUMBER)
    {
        video_exposure_prop = property->getNumber();
        newNumber(video_exposure_prop);
    }
    else if (PropName == "FPS" && Proptype == LITHIUM_NUMBER)
    {
        video_fps_prop = property->getNumber();
        newNumber(video_fps_prop);
    }
    else if (PropName == "DEVICE_PORT" && Proptype == LITHIUM_TEXT)
    {
        camera_port = property->getText();
        device_info["network"]["port"] = camera_port->tp->text;
        DLOG_F(INFO, "Current device port of {} is {}", _name, camera_port->tp->text);
    }
    else if (PropName == "CONNECTION" && Proptype == LITHIUM_SWITCH)
    {
        connection_prop = property->getSwitch();
        ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
        is_connected = (connectswitch->s == ISS_ON);
        if (!is_connected)
        {
            connection_prop->sp->s = ISS_ON;
            sendNewSwitch(connection_prop);
        }
        DLOG_F(INFO, "{} Connected {}", _name, is_connected);
    }
    else if (PropName == "DRIVER_INFO" && Proptype == LITHIUM_TEXT)
    {
        device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
        indi_camera_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
        indi_camera_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
        indi_camera_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
        device_info["driver"]["name"] = device_name;
        device_info["driver"]["exec"] = indi_camera_exec;
        device_info["driver"]["version"] = indi_camera_version;
        device_info["driver"]["interfaces"] = indi_camera_interface;
        DLOG_F(INFO, "Camera Name : {} connected exec {}", _name, device_name, indi_camera_exec);
    }
    else if (PropName == indi_camera_cmd + "INFO" && Proptype == LITHIUM_NUMBER)
    {
        ccdinfo_prop = property->getNumber();
        newNumber(ccdinfo_prop);
    }
    else if (PropName == "DEBUG" && Proptype == LITHIUM_SWITCH)
    {
        debug_prop = property->getSwitch();
        newSwitch(debug_prop);
    }
    else if (PropName == "POLLING_PERIOD" && Proptype == LITHIUM_NUMBER)
    {
        polling_prop = property->getNumber();
        newNumber(polling_prop);
    }
    else if (PropName == "ACTIVE_DEVICES" && Proptype == LITHIUM_TEXT)
    {
        active_device_prop = property->getText();
        newText(active_device_prop);
    }
    else if (PropName == "CCD_COMPRESSION" && Proptype == LITHIUM_SWITCH)
    {
        compression_prop = property->getSwitch();
        newSwitch(compression_prop);
    }
    else if (PropName == "UPLOAD_MODE" && Proptype == LITHIUM_SWITCH)
    {
        image_upload_mode_prop = property->getSwitch();
        newSwitch(image_upload_mode_prop);
    }
    else if (PropName == "CCD_FAST_TOGGLE" && Proptype == LITHIUM_SWITCH)
    {
        fast_read_out_prop = property->getSwitch();
        newSwitch(fast_read_out_prop);
    }
    else if (PropName == "LIMITS" && Proptype == LITHIUM_NUMBER)
    {
        camera_limit_prop = property->getNumber();
        newNumber(camera_limit_prop);
    }
    // The following properties are for ASI Camera
    else if (PropName == "FLIP" && Proptype == LITHIUM_SWITCH)
    {
        asi_image_flip_prop = property->getSwitch();
        newSwitch(asi_image_flip_prop);
    }
    else if (PropName == "CCD_CONTROLS" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "CCD_CONTROLS_MODE" && Proptype == LITHIUM_SWITCH)
    {
    }
    // The following properties are for Toup Camera
    else if (PropName == "TC_FAN_CONTROL" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "TC_FAN_Speed" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "TC_AUTO_WB" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "TC_HEAT_CONTROL" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "TC_HCG_CONTROL" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "TC_HGC_SET" && Proptype == LITHIUM_NUMBER)
    {
    }
    else if (PropName == "TC_LOW_NOISE_CONTROL" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "SIMULATION" && Proptype == LITHIUM_SWITCH)
    {
        toupcam_simulation_prop = property->getSwitch();
        newSwitch(toupcam_simulation_prop);
    }
    else if (PropName == "CCD_LEVEL_RANGE" && Proptype == LITHIUM_NUMBER)
    {
    }
    else if (PropName == "CCD_BINNING_MODE" && Proptype == LITHIUM_SWITCH)
    {
    }
    else if (PropName == "CCD_BLACK_BALANCE" && Proptype == LITHIUM_NUMBER)
    {
    }
    else if (PropName == "Firmware" && Proptype == LITHIUM_NUMBER)
    {
    }
}

void HydrogenCamera::IndiServerConnected()
{
    DLOG_F(INFO, "{} connection succeeded", _name);
    is_connected = true;
}

void HydrogenCamera::IndiServerDisconnected(int exit_code)
{
    DLOG_F(INFO, "{}: serverDisconnected", _name);
    // after disconnection we reset the connection status and the properties pointers
    ClearStatus();
    // in case the connection lost we must reset the client socket
    if (exit_code == -1)
        DLOG_F(INFO, "{} : Hydorgen server disconnected", _name);
}

void HydrogenCamera::removeDevice(HYDROGEN::BaseDevice *dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", _name);
}

void HydrogenCamera::ClearStatus()
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
    debug_prop = nullptr;
    polling_prop = nullptr;
    active_device_prop = nullptr;
    compression_prop = nullptr;
    image_upload_mode_prop = nullptr;
    fast_read_out_prop = nullptr;
    camera_limit_prop = nullptr;

    toupcam_fan_control_prop = nullptr;
    toupcam_heat_control_prop = nullptr;
    toupcam_hcg_control_prop = nullptr;
    toupcam_low_noise_control_prop = nullptr;
    toupcam_simulation_prop = nullptr;
    toupcam_binning_mode_prop = nullptr;

    asi_image_flip_prop = nullptr;
    asi_image_flip_hor_prop = nullptr;
    asi_image_flip_ver_prop = nullptr;
    asi_controls_prop = nullptr;
    asi_controls_mode_prop = nullptr;
}

HydrogenCamera::HydrogenCamera(const std::string &name) : Camera(name)
{
}

HydrogenCamera::~HydrogenCamera()
{
}

bool HydrogenCamera::connect(std::string name)
{
    DLOG_F(INFO, "Trying to connect to {}", name);
    setServer(hostname.c_str(), port);
    // Receive messages only for our camera.
    watchDevice(name.c_str());
    // Connect to server.
    if (connectServer())
    {
        DLOG_F(INFO, "{}: connectServer done ready = {}", _name, is_ready);
        connectDevice(name.c_str());
        return !is_ready;
    }
    return false;
}

bool HydrogenCamera::disconnect()
{
    return true;
}

bool HydrogenCamera::reconnect()
{
    return true;
}

bool HydrogenCamera::scanForAvailableDevices()
{
    return true;
}

bool HydrogenCamera::getParameter(const std::string &paramName)
{
    if (paramName.empty())
    {
        // spdlog::error("HydrogenCamera::getParameter : Parameter name is required");
        return false;
    }
    return false;
}

bool HydrogenCamera::setParameter(const std::string &paramName, const std::string &paramValue)
{
    if (paramName.empty() || paramValue.empty())
    {
        // spdlog::error("HydrogenCamera::setParameter : Parameter name and value are required");
        return false;
        ;
    }
    return false;
}

bool HydrogenCamera::startExposure(int duration_ms)
{
    return true;
}

bool HydrogenCamera::stopExposure()
{
    return true;
}

bool HydrogenCamera::waitForExposureComplete()
{
    return true;
}

// bool readImage(Image& image);

bool HydrogenCamera::startLiveView()
{
    return true;
}

bool HydrogenCamera::stopLiveView()
{
    return true;
}
// bool readLiveView(Image& image);

bool HydrogenCamera::setCoolingOn(bool on)
{
    return true;
}

bool HydrogenCamera::setTemperature(double temperature)
{
    return true;
}

double HydrogenCamera::getTemperature()
{
    return true;
}

bool HydrogenCamera::setShutterOpen(bool open)
{
    return true;
}

bool HydrogenCamera::setBinning(int binning)
{
    return true;
}

bool HydrogenCamera::setGain(int gain)
{
    return true;
}

bool HydrogenCamera::setOffset(int offset)
{
    return true;
}

bool HydrogenCamera::setROIFrame(int start_x, int start_y, int frame_x, int frame_y)
{
    return true;
}

std::shared_ptr<Lithium::SimpleTask> HydrogenCamera::getSimpleTask(const std::string &task_name, const nlohmann::json &params)
{
    // 定义任务名称与逻辑之间的映射关系
    std::map<std::string, std::function<void(const nlohmann::json &)>> task_map = {
        {"Connect", [this](const nlohmann::json &tparams)
         {
             if (tparams["name"].empty())
             {
                 // spdlog::error("No camera name specified");
                 return;
             }
             if (!this->connect(tparams["name"]))
             {
                 // spdlog::error("Failed to connect to camera {}", _name);
                 return;
             }
         }},
        {"Disconnect", [this](const nlohmann::json &tparams)
         {
             if (!this->is_connected && !this->device_info["connected"].empty())
             {
                 // spdlog::warn("Camera is not connected, please do not execute disconnect command");
                 return;
             }
             if (!this->disconnect())
             {
                 // spdlog::error("Failed to disconnect from camera {}", _name);
             }
         }},
        {"Reconnect", [this](const nlohmann::json &tparams)
         {
             if (!this->is_connected && !this->device_info["connected"].empty())
             {
                 // spdlog::warn("Camera is not connected, please do not execute reconnect command");
                 return;
             }
             if (!this->reconnect())
             {
                 // spdlog::error("Failed to reconnect from camera {}", _name);
             }
         }},

        {"Scanning", [this](const nlohmann::json &tparams)
         {
             if (!this->scanForAvailableDevices())
             {
                 // spdlog::error("Failed to scan for available devices from camera {}", _name);
             }
         }},
        {"GetParameter", [this](const nlohmann::json &tparams)
         {
             if (!this->getParameter(tparams["name"].get<std::string>()))
             {
                 // spdlog::error("Failed to get parameter from camera {}", _name);
             }
         }},
        {"SetParameter", [this](const nlohmann::json &tparams)
         {
             if (!this->setParameter(tparams["name"].get<std::string>(), tparams["value"].get<std::string>()))
             {
                 // spdlog::error("Failed to set parameter to camera {}", _name);
             }
         }},
        {"SingleShot", [this](const nlohmann::json &tparams)
         {
             DLOG_F(INFO, "{} SingleShot task is called", this->_name);
         }},

        {"AbortShot", [this](const nlohmann::json &tparams)
         {
             DLOG_F(INFO, "{} AbortShot task is called", this->_name);
         }},
        {"StartLiveView", [this](const nlohmann::json &tparams)
         {
             this->startLiveView();
         }},
        {"StopLiveView", [this](const nlohmann::json &tparams)
         {
             this->stopLiveView();
         }},
        {"Cooling", [this](const nlohmann::json &tparams)
         {
             if (!this->can_cooling)
             {
                 // spdlog::error("Can't cool while cooling is unsupported");
                 return;
             }
             if (!this->setCoolingOn(tparams["enable"].get<bool>()))
             {
                 // spdlog::error("Failed to change the mode of cooling");
             }
         }},
        {"GetTemperature", [this](const nlohmann::json &tparams)
         {
             this->getTemperature();
         }},
        {"SetTemperature", [this](const nlohmann::json &tparams)
         {
             if (!this->can_cooling)
             {
                 // spdlog::error("Can't set temperature while cooling is unsupported");
                 return;
             }
         }},
        {"SetGain", [this](const nlohmann::json &tparams)
         {
             DLOG_F(INFO, "{} SetGain task is called", this->_name);
         }},
        {"SetOffset", [this](const nlohmann::json &tparams)
         {
             DLOG_F(INFO, "{} SetOffset task is called", this->_name);
         }},
        {"SetBinning", [this](const nlohmann::json &tparams)
         {
             if (!this->can_binning)
             {
                 // spdlog::error("Can't bin while binning is unsupported");
                 return;
             }
             if (!this->setBinning(tparams["binning"].get<int>()))
             {
                 // spdlog::error("Failed to change the mode of binning");
             }
         }},
        {"SetROIFrame", [this](const nlohmann::json &tparams)
         {
             if (!this->setROIFrame(tparams["start_x"].get<int>(), tparams["start_y"].get<int>(), tparams["frame_x"].get<int>(), tparams["frame_y"].get<int>()))
             {
                 // spdlog::error("Failed to change the mode of ROI");
             }
         }}};

    auto it = task_map.find(task_name);
    if (it != task_map.end())
    {
        return std::shared_ptr<Lithium::SimpleTask>(new Lithium::SimpleTask(it->second, {params}));
    }
    // spdlog::error("Unknown type of the {} task : {}", _name, task_name);
    return nullptr;
}

std::shared_ptr<Lithium::ConditionalTask> HydrogenCamera::getCondtionalTask(const std::string &task_name, const nlohmann::json &params)
{
    return nullptr;
}

std::shared_ptr<Lithium::LoopTask> HydrogenCamera::getLoopTask(const std::string &task_name, const nlohmann::json &params)
{
    return nullptr;
}