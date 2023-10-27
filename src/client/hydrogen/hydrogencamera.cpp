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

HydrogenCamera::HydrogenCamera(const std::string &name) : Camera(name)
{
}

HydrogenCamera::~HydrogenCamera()
{
}

bool HydrogenCamera::connect(const nlohmann::json &params)
{
    std::string name = params["name"];
    std::string hostname = params["host"];
    int port = params["port"];
    DLOG_F(INFO, "Trying to connect to {}", name);
    setServer(hostname.c_str(), port);
    // Receive messages only for our camera.
    watchDevice(name.c_str());
    // Connect to server.
    if (connectServer())
    {
        DLOG_F(INFO, "{}: connectServer done ready", getDeviceName());
        connectDevice(name.c_str());
        return !is_ready.load();
    }
    return false;
}

bool HydrogenCamera::disconnect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool HydrogenCamera::reconnect(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::isConnected()
{
    return true;
}

bool HydrogenCamera::startExposure(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::abortExposure(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::getExposureStatus(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::getExposureResult(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::saveExposureResult(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::startVideo(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::stopVideo(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::getVideoStatus(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::getVideoResult(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::saveVideoResult(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::startCooling(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::stopCooling(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::isCoolingAvailable()
{
    return true;
}

bool HydrogenCamera::getTemperature(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::getCoolingPower(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::setTemperature(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::setCoolingPower(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::getGain(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::setGain(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::isGainAvailable()
{
    return true;
}

bool HydrogenCamera::getOffset(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::setOffset(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::isOffsetAvailable()
{
    return true;
}

bool HydrogenCamera::getISO(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::setISO(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::isISOAvailable()
{
    return true;
}

bool HydrogenCamera::getFrame(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::setFrame(const nlohmann::json &params)
{
    return true;
}

bool HydrogenCamera::isFrameSettingAvailable()
{
    return true;
}

void HydrogenCamera::newDevice(HYDROGEN::BaseDevice *dp)
{
    if (strcmp(dp->getDeviceName(), getDeviceName().c_str()) == 0)
    {
        camera_device = dp;
    }
}

void HydrogenCamera::newSwitch(ISwitchVectorProperty *svp)
{
    std::string name = svp->name;

    DLOG_F(INFO, "{} Received Switch: {}", getDeviceName(), name);

    if (name == "CONNECTION")
    {
        connection_prop.reset(svp);
        if (auto connectswitch = IUFindSwitch(svp, "CONNECT"); connectswitch->s == ISS_ON)
        {
            setProperty("connect", true);
            is_connected.store(true);
            DLOG_F(INFO, "{} is connected", getDeviceName());
        }
        else
        {
            if (is_ready.load())
            {
                ClearStatus();
                setProperty("connect", false);
                is_connected.store(true);
                DLOG_F(INFO, "{} is disconnected", getDeviceName());
            }
        }
    }
    else if (name == "DEBUG")
    {
        debug_prop.reset(svp);
        if (auto debugswitch = IUFindSwitch(svp, "ENABLE"); debugswitch->s == ISS_ON)
        {
            setProperty("debug", true);
            is_debug.store(true);
            DLOG_F(INFO, "DEBUG mode of {} is enabled", getDeviceName());
        }
        else
        {
            setProperty("debug", false);
            is_debug.store(false);
            DLOG_F(INFO, "DEBUG mode of {} is disabled", getDeviceName());
        }
    }
    else if (name == "CCD_FRAME_TYPE")
    {
        frame_type_prop.reset(svp);
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
        frame.frame_type = type;
        DLOG_F(INFO, "Current frame type of {} is {}", getDeviceName(), frame.frame_type);
    }
    else if (name == "CCD_TRANSFER_FORMAT")
    {
        frame_format_prop.reset(svp);
        std::string format;
        if (auto fitsswitch = IUFindSwitch(svp, "FORMAT_FITS"); fitsswitch->s == ISS_ON)
            format = "Fits";
        else if (auto natswitch = IUFindSwitch(svp, "FORMAT_NATIVE"); natswitch->s == ISS_ON)
            format = "Raw";
        else if (auto xisfswitch = IUFindSwitch(svp, "FORMAT_XISF"); xisfswitch->s == ISS_ON)
            format = "Xisf";
        setProperty("frame_format", format);
        frame.frame_format = format;
        DLOG_F(INFO, "Current frame format of {} is {}", getDeviceName(), frame.frame_format);
    }
    else if (name == "CCD_ABORT_EXPOSURE")
    {
        abort_exposure_prop.reset(svp);
        if (auto abortswitch = IUFindSwitch(svp, "ABORT_EXPOSURE"); abortswitch->s == ISS_ON)
        {
            setProperty("is_exposure", false);
            is_exposure.store(false);
            DLOG_F(INFO, "{} is stopped", getDeviceName());
        }
    }
    else if (name == "UPLOAD_MODE")
    {
        image_upload_mode_prop.reset(svp);
        std::string mode;
        if (auto clientswitch = IUFindSwitch(svp, "UPLOAD_CLIENT"); clientswitch->s == ISS_ON)
            mode = "Client";
        else if (auto localswitch = IUFindSwitch(svp, "UPLOAD_LOCAL"); localswitch->s == ISS_ON)
            mode = "Local";
        else if (auto bothswitch = IUFindSwitch(svp, "UPLOAD_BOTH"); bothswitch->s == ISS_ON)
            mode = "Both";
        frame.upload_mode = mode;
        DLOG_F(INFO, "Current upload mode of {} is {}", getDeviceName(), frame.upload_mode);
    }
    else if (name == "CCD_FAST_TOGGLE")
    {
        fast_read_out_prop.reset(svp);
        if (auto enabledswitch = IUFindSwitch(svp, "HYDROGEN_ENABLED"); enabledswitch->s == ISS_ON)
        {
            setProperty("is_fastread", true);
            frame.is_fastread.store(true);
            DLOG_F(INFO, "Current fast readout mode of {} is enabled", getDeviceName());
        }
        else if (auto disabledswitch = IUFindSwitch(svp, "HYDROGEN_DISABLED"); disabledswitch->s == ISS_ON)
        {
            setProperty("is_fastread", false);
            frame.is_fastread.store(false);
            DLOG_F(INFO, "Current fast readout mode of {} is disabled", getDeviceName());
        }
    }
    else if (name == "CCD_VIDEO_STREAM")
    {
        video_prop.reset(svp);
        if (auto onswitch = IUFindSwitch(svp, "STREAM_ON"); onswitch->s == ISS_ON)
        {
            setProperty("is_video", true);
            is_video.store(true);
            DLOG_F(INFO, "{} start video capture", getDeviceName());
        }
        else if (auto offswitch = IUFindSwitch(svp, "STREAM_OFF"); offswitch->s == ISS_ON)
        {
            setProperty("is_video", false);
            is_video.store(false);
            DLOG_F(INFO, "{} stop video capture", getDeviceName());
        }
    }
    else if (name == "FLIP")
    {
    }
    else if (name == "CCD_COMPRESSION")
    {
    }

    else if (name == "CCD_CONTROLS")
    {
    }
    else if (name == "CCD_CONTROLS_MODE")
    {
    }
    // The following properties are for Toup Camera
    else if (name == "TC_FAN_CONTROL")
    {
    }
    else if (name == "TC_FAN_Speed")
    {
    }
    else if (name == "TC_AUTO_WB")
    {
    }
    else if (name == "TC_HEAT_CONTROL")
    {
    }
    else if (name == "TC_HCG_CONTROL")
    {
    }
    else if (name == "TC_LOW_NOISE_CONTROL")
    {
    }
    /*
    else if (PropName == "CCD_BINNING_MODE" && Proptype == HYDROGEN_SWITCH)
    {
    }
    */
}

void HydrogenCamera::newMessage(HYDROGEN::BaseDevice *dp, int messageID)
{
    DLOG_F(INFO, "{} Received message: {}", getDeviceName(), dp->messageQueue(messageID));
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
        current_exposure.store(exposure);
        DLOG_F(INFO, "Current CCD_EXPOSURE for {} is {}", getDeviceName(), exposure);
    }
    else if (name == "CCD_INFO")
    {
        ccdinfo_prop.reset(nvp);
        frame.pixel.store(IUFindNumber(nvp, "CCD_PIXEL_SIZE")->value);
        frame.pixel_x.store(IUFindNumber(nvp, "CCD_PIXEL_SIZE_X")->value);
        frame.pixel_y.store(IUFindNumber(nvp, "CCD_PIXEL_SIZE_Y")->value);
        frame.max_frame_x.store(IUFindNumber(nvp, "CCD_MAX_X")->value);
        frame.max_frame_y.store(IUFindNumber(nvp, "CCD_MAX_Y")->value);
        frame.pixel_depth.store(IUFindNumber(nvp, "CCD_BITSPERPIXEL")->value);

        DLOG_F(INFO, "{} pixel {} pixel_x {} pixel_y {} max_frame_x {} max_frame_y {} pixel_depth {}",
               getDeviceName(), frame.pixel.load(), frame.pixel_x.load(), frame.pixel_y.load(), frame.max_frame_x.load(), frame.max_frame_y.load(), frame.pixel_depth.load());
    }
    else if (name == "CCD_BINNING")
    {
        indi_binning_x.reset(IUFindNumber(nvp, "HOR_BIN"));
        indi_binning_y.reset(IUFindNumber(nvp, "VER_BIN"));
        frame.binning_x.store(indi_binning_x->value);
        frame.binning_y.store(indi_binning_y->value);
        DLOG_F(INFO, "Current binning_x and y of {} are {} {}", getDeviceName(), indi_binning_x->value, indi_binning_y->value);
    }
    else if (name == "CCD_FRAME")
    {
        indi_frame_x.reset(IUFindNumber(nvp, "X"));
        indi_frame_y.reset(IUFindNumber(nvp, "Y"));
        indi_frame_width.reset(IUFindNumber(nvp, "WIDTH"));
        indi_frame_height.reset(IUFindNumber(nvp, "HEIGHT"));

        frame.frame_x.store(indi_frame_x->value);
        frame.frame_y.store(indi_frame_y->value);
        frame.frame_height.store(indi_frame_height->value);
        frame.frame_width.store(indi_frame_width->value);

        DLOG_F(INFO, "Current frame of {} are {} {} {} {}", getDeviceName(), indi_frame_width->value, indi_frame_y->value, indi_frame_width->value, indi_frame_height->value);
    }
    else if (name == "CCD_TEMPERATURE")
    {
        camera_temperature_prop.reset(nvp);
        current_temperature.store(IUFindNumber(nvp, "CCD_TEMPERATURE_VALUE")->value);
        DLOG_F(INFO, "Current temperature of {} is {}", getDeviceName(), current_temperature.load());
    }
    else if (name == "CCD_GAIN")
    {
        gain_prop.reset(nvp);
        current_gain.store(IUFindNumber(nvp, "GAIN")->value);
        setProperty("gain", current_gain.load());
        DLOG_F(INFO, "Current camera gain of {} is {}", getDeviceName(), current_gain.load());
    }
    else if (name == "CCD_OFFSET")
    {
        offset_prop.reset(nvp);
        current_offset.store(IUFindNumber(nvp, "OFFSET")->value);
        setProperty("offset", current_offset.load());
        DLOG_F(INFO, "Current camera offset of {} is {}", getDeviceName(), current_offset.load());
    }
    /*
     else if (name == "POLLING_PERIOD")
    {
        device_info["network"]["period"] = IUFindNumber(nvp, "PERIOD_MS")->value;
        DLOG_F(INFO, "Current period of {} is {}", getDeviceName(), device_info["network"]["period"].dump());
    }
    else if (name == "LIMITS")
    {
        device_info["limits"]["maxbuffer"] = IUFindNumber(nvp, "LIMITS_BUFFER_MAX")->value;
        DLOG_F(INFO, "Current max buffer of {} is {}", getDeviceName(), device_info["limits"]["maxbuffer"].dump());
        device_info["limits"]["maxfps"] = IUFindNumber(nvp, "LIMITS_PREVIEW_FPS")->value;
        DLOG_F(INFO, "Current max fps of {} is {}", getDeviceName(), device_info["limits"]["maxfps"].dump());
    }
    else if (name == "STREAM_DELAY")
    {
        device_info["video"]["delay"] = IUFindNumber(nvp, "STREAM_DELAY_TIME")->value;
        DLOG_F(INFO, "Current stream delay of {} is {}", getDeviceName(), device_info["video"]["delay"].dump());
    }
    else if (name == "STREAMING_EXPOSURE")
    {
        device_info["video"]["exposure"] = IUFindNumber(nvp, "STREAMING_EXPOSURE_VALUE")->value;
        DLOG_F(INFO, "Current streaming exposure of {} is {}", getDeviceName(), device_info["video"]["exposure"].dump());
        device_info["video"]["division"] = IUFindNumber(nvp, "STREAMING_DIVISOR_VALUE")->value;
        DLOG_F(INFO, "Current streaming division of {} is {}", getDeviceName(), device_info["video"]["division"].dump());
    }
    else if (name == "FPS")
    {
        device_info["video"]["fps"] = IUFindNumber(nvp, "EST_FPS")->value;
        DLOG_F(INFO, "Current fps of {} is {}", getDeviceName(), device_info["video"]["fps"].dump());
        device_info["video"]["avgfps"] = IUFindNumber(nvp, "AVG_FPS")->value;
        DLOG_F(INFO, "Current average fps of {} is {}", getDeviceName(), device_info["video"]["avgfps"].dump());
    }
    else if (PropName == "TC_HGC_SET" && Proptype == HYDROGEN_NUMBER)
    {
    }
    else if (PropName == "CCD_LEVEL_RANGE" && Proptype == HYDROGEN_NUMBER)
    {
    }

    else if (PropName == "CCD_BLACK_BALANCE" && Proptype == HYDROGEN_NUMBER)
    {
    }
    else if (PropName == "Firmware" && Proptype == HYDROGEN_NUMBER)
    {
    }
    */
}

void HydrogenCamera::newText(ITextVectorProperty *tvp)
{
    const std::string name = tvp->name;
    DLOG_F(INFO, "{} Received Text: {} = {}", getDeviceName(), name, tvp->tp->text);

    if (name == indi_camera_cmd + "CFA")
    {
        cfa_prop.reset(tvp);
        cfa_type_prop.reset(IUFindText(tvp, "CFA_TYPE"));
        if (cfa_type_prop && cfa_type_prop->text && *cfa_type_prop->text)
        {
            DLOG_F(INFO, "{} CFA_TYPE is {}", getDeviceName(), cfa_type_prop->text);
            is_color = true;
            setProperty("is_color", true);
        }
        else
        {
            setProperty("is_color", false);
        }
    }
    else if (name == "DEVICE_PORT")
    {
        camera_prop.reset(tvp);
        indi_camera_port = tvp->tp->text;
        setProperty("port", indi_camera_port);
        DLOG_F(INFO, "Current device port of {} is {}", getDeviceName(), camera_prop->tp->text);
    }
    else if (name == "DRIVER_INFO")
    {
        indi_camera_exec = IUFindText(tvp, "DRIVER_EXEC")->text;
        indi_camera_version = IUFindText(tvp, "DRIVER_VERSION")->text;
        indi_camera_interface = IUFindText(tvp, "DRIVER_INTERFACE")->text;
        DLOG_F(INFO, "Camera Name : {} connected exec {}", getDeviceName(), getDeviceName(), indi_camera_exec);
    }
    else if (name == "ACTIVE_DEVICES")
    {
        active_device_prop.reset(tvp);
    }
}

void HydrogenCamera::newBLOB(IBLOB *bp)
{
    // we go here every time a new blob is available
    // this is normally the image from the camera

    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", getDeviceName(), bp->name, bp->bloblen, bp->size);

    if (exposure_prop)
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
    HYDROGEN_PROPERTY_TYPE Proptype = property->getType();

    // DLOG_F(INFO,"{} Property: {}", getDeviceName(), property->getName());

    if (Proptype == HYDROGEN_BLOB)
    {

        if (PropName == indi_blob_name.c_str())
        {
            has_blob = 1;
            // set option to receive blob and messages for the selected CCD
            setBLOBMode(B_ALSO, getDeviceName().c_str(), indi_blob_name.c_str());

#ifdef HYDROGEN_SHARED_BLOB_SUPPORT
            // Allow faster mode provided we don't modify the blob content or free/realloc it
            enableDirectBlobAccess(getDeviceName().c_str(), indi_blob_name.c_str());
#endif
        }
    }
    else if (Proptype == HYDROGEN_NUMBER)
    {
        newNumber(property->getNumber());
    }
    else if (Proptype == HYDROGEN_SWITCH)
    {
        newSwitch(property->getSwitch());
    }
    else if (Proptype == HYDROGEN_TEXT)
    {
        newText(property->getText());
    }
}

void HydrogenCamera::IndiServerConnected()
{
    DLOG_F(INFO, "{} connection succeeded", getDeviceName());
    is_connected = true;
}

void HydrogenCamera::IndiServerDisconnected(int exit_code)
{
    DLOG_F(INFO, "{}: serverDisconnected", getDeviceName());
    // after disconnection we reset the connection status and the properties pointers
    ClearStatus();
    // in case the connection lost we must reset the client socket
    if (exit_code == -1)
        DLOG_F(INFO, "{} : Hydorgen server disconnected", getDeviceName());
}

void HydrogenCamera::removeDevice(HYDROGEN::BaseDevice *dp)
{
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", getDeviceName());
}

void HydrogenCamera::ClearStatus()
{
    connection_prop = nullptr;
    exposure_prop = nullptr;
    frame_prop = nullptr;
    frame_type_prop = nullptr;
    ccdinfo_prop = nullptr;
    binning_prop = nullptr;
    video_prop = nullptr;
    camera_prop = nullptr;
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