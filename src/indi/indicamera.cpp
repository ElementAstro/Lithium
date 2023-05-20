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
        auto [name, s] = std::tuple{svp->name, IUFindSwitch(svp, "CONNECT")->s};

        spdlog::debug("{} Received Switch: {}", _name, name);

        if (name == "CONNECTION")
        {
            if (s == ISS_ON)
            {
                is_connected = true;
                camera_info["connected"] = true;
                spdlog::info("{} is connected", _name);
            }
            else
            {
                if (is_ready)
                {
                    ClearStatus();
                    camera_info["connected"] = false;
                    spdlog::info("{} is disconnected", _name);
                }
            }
        }
        else if (name == "DEBUG")
        {
            if (auto [enable_name, enable_s] = std::tuple{"ENABLE", IUFindSwitch(svp, "ENABLE")->s}; enable_s == ISS_ON)
            {
                is_debug = camera_info["debug"] = true;
                spdlog::info("DEBUG mode of {} is enabled", _name);
            }
            else
            {
                is_debug = camera_info["debug"] = false;
                spdlog::info("DEBUG mode of {} is disabled", _name);
            }
        }
        else if (name == "CCD_FRAME_TYPE")
        {
            std::string_view type;

            if (auto [frame_light_name, frame_light_s] = std::tuple{"FRAME_LIGHT", IUFindSwitch(svp, "FRAME_LIGHT")->s}; frame_light_s == ISS_ON)
                type = "Light";
            else if (auto [frame_dark_name, frame_dark_s] = std::tuple{"FRAME_DARK", IUFindSwitch(svp, "FRAME_DARK")->s}; frame_dark_s == ISS_ON)
                type = "Dark";
            else if (auto [frame_flat_name, frame_flat_s] = std::tuple{"FRAME_FLAT", IUFindSwitch(svp, "FRAME_FLAT")->s}; frame_flat_s == ISS_ON)
                type = "Flat";
            else if (auto [frame_bias_name, frame_bias_s] = std::tuple{"FRAME_BIAS", IUFindSwitch(svp, "FRAME_BIAS")->s}; frame_bias_s == ISS_ON)
                type = "Bias";

            camera_info["frame"]["type"] = type;
            spdlog::debug("Current frame type of {} is {}", _name, camera_info["frame"]["type"].dump());
        }
        else if (name == "CCD_TRANSFER_FORMAT")
        {
            std::string_view format;

            if (auto [format_fits_name, format_fits_s] = std::tuple{"FORMAT_FITS", IUFindSwitch(svp, "FORMAT_FITS")->s}; format_fits_s == ISS_ON)
                format = "Fits";
            else if (auto [format_native_name, format_native_s] = std::tuple{"FORMAT_NATIVE", IUFindSwitch(svp, "FORMAT_NATIVE")->s}; format_native_s == ISS_ON)
                format = "Raw";
            else if (auto [format_xisf_name, format_xisf_s] = std::tuple{"FORMAT_XISF", IUFindSwitch(svp, "FORMAT_XISF")->s}; format_xisf_s == ISS_ON)
                format = "Xisf";

            camera_info["frame"]["format"] = format;
            spdlog::debug("Current frame format of {} is {}", _name, camera_info["frame"]["format"].dump());
        }
        else if (name == "CCD_ABORT_EXPOSURE")
        {
            if (auto [abort_name, abort_s] = std::tuple{"ABORT_EXPOSURE", IUFindSwitch(svp, "ABORT_EXPOSURE")->s}; abort_s == ISS_ON)
            {
                camera_info["exposure"]["abort"] = true;
                spdlog::debug("{} is stopped", _name);
                is_exposuring = false;
            }
        }
        else if (name == "UPLOAD_MODE")
        {
            std::string_view mode;

            if (auto [clientswitch_name, clientswitch_s] = std::tuple{"UPLOAD_CLIENT", IUFindSwitch(svp, "UPLOAD_CLIENT")->s}; clientswitch_s == ISS_ON)
                mode = "Client";
            else if (auto [localswitch_name, localswitch_s] = std::tuple{"UPLOAD_LOCAL", IUFindSwitch(svp, "UPLOAD_LOCAL")->s}; localswitch_s == ISS_ON)
                mode = "Local";
            else if (auto [bothswitch_name, bothswitch_s] = std::tuple{"UPLOAD_BOTH", IUFindSwitch(svp, "UPLOAD_BOTH")->s}; bothswitch_s == ISS_ON)
                mode = "Both";

            camera_info["network"]["mode"] = mode;
            spdlog::debug("Current upload mode of {} is {}", _name, camera_info["network"]["mode"].dump());
        }
        else if (name == "CCD_FAST_TOGGLE")
        {
            bool mode;

            if (auto [indiswitch_name, indiswitch_s] = std::tuple{"INDI_ENABLED", IUFindSwitch(svp, "INDI_ENABLED")->s}; indiswitch_s == ISS_ON)
                mode = true;
            else if (auto [indiswitch_name, indiswitch_s] = std::tuple{"INDI_DISABLED", IUFindSwitch(svp, "INDI_DISABLED")->s}; indiswitch_s == ISS_ON)
                mode = false;

            camera_info["frame"]["fast_read"] = mode;
            spdlog::debug("Current readout mode of {} is {}", _name, camera_info["frame"]["fast_read"].dump());
        }
        else if (name == "CCD_VIDEO_STREAM")
        {
            if (auto [stream_on_name, stream_on_s] = std::tuple{"STREAM_ON", IUFindSwitch(svp, "STREAM_ON")->s}; stream_on_s == ISS_ON)
            {
                camera_info["video"]["is_video"] = true;
                is_video = true;
                spdlog::debug("{} start video capture", _name);
            }
            else if (auto [stream_off_name, stream_off_s] = std::tuple{"STREAM_OFF", IUFindSwitch(svp, "STREAM_OFF")->s}; stream_off_s == ISS_ON)
            {
                camera_info["video"]["is_video"] = false;
                is_video = false;
                spdlog::debug("{} stop video capture", _name);
            }
        }
        else if (name == "FLIP")
        {
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
        using namespace std::string_literals;
        const std::string name = nvp->name;

        auto find_number = [](INumberVectorProperty *prop, const std::string &n) -> std::optional<double>
        {
            if (auto num = IUFindNumber(prop, n.c_str()))
            {
                return num->value;
            }
            return std::nullopt;
        };

        if (name == "CCD_EXPOSURE")
        {
            if (auto exposure = find_number(nvp, "CCD_EXPOSURE"))
            {
                camera_info["exposure"]["current"] = *exposure;
                spdlog::debug("Current CCD_EXPOSURE for {} is {}", _name, *exposure);
            }
        }
        else if (name == "CCD_INFO")
        {
            const auto pixel = find_number(nvp, "CCD_PIXEL_SIZE");
            const auto pixel_x = find_number(nvp, "CCD_PIXEL_SIZE_X");
            const auto pixel_y = find_number(nvp, "CCD_PIXEL_SIZE_Y");
            const auto max_frame_x = find_number(nvp, "CCD_MAX_X");
            const auto max_frame_y = find_number(nvp, "CCD_MAX_Y");
            const auto pixel_depth = find_number(nvp, "CCD_BITSPERPIXEL");

            if (pixel && pixel_x && pixel_y && max_frame_x && max_frame_y && pixel_depth)
            {
                camera_info["frame"] = {
                    {"pixel_x", *pixel_x},
                    {"pixel_y", *pixel_y},
                    {"pixel_depth", *pixel_depth},
                    {"max_frame_x", *max_frame_x},
                    {"max_frame_y", *max_frame_y}};
                spdlog::debug("{} pixel {} pixel_x {} pixel_y {} max_frame_x {} max_frame_y {} pixel_depth {}", _name, *pixel, *pixel_x, *pixel_y, *max_frame_x, *max_frame_y, *pixel_depth);
            }
        }
        else if (name == "BINNING")
        {
            if (auto binning_x = find_number(nvp, "HOR_BIN"))
            {
                if (auto binning_y = find_number(nvp, "VER_BIN"))
                {
                    camera_info["exposure"] = {
                        {"binning_x", *binning_x},
                        {"binning_y", *binning_y}};
                    spdlog::debug("Current binning_x and y of {} are {} {}", _name, *binning_x, *binning_y);
                }
            }
        }
        else if (name == "FRAME")
        {
            const auto [indi_frame_x, indi_frame_y, indi_frame_width, indi_frame_height] =
                std::make_tuple(find_number(nvp, "X"), find_number(nvp, "Y"), find_number(nvp, "WIDTH"), find_number(nvp, "HEIGHT"));

            if (indi_frame_x && indi_frame_y && indi_frame_width && indi_frame_height)
            {
                camera_info["frame"] = {
                    {"x", *indi_frame_x},
                    {"y", *indi_frame_y},
                    {"width", *indi_frame_width},
                    {"height", *indi_frame_height}};
                spdlog::debug("Current frame of {} are {} {} {} {}", _name, *indi_frame_width, *indi_frame_y, *indi_frame_width, *indi_frame_height);
            }
        }
        else if (name == "CCD_TEMPERATURE")
        {
            if (auto temperature = find_number(nvp, "CCD_TEMPERATURE_VALUE"))
            {
                camera_info["temperature"]["current"] = *temperature;
                spdlog::debug("Current temperature of {} is {}", _name, *temperature);
            }
        }
        else if (name == "CCD_GAIN")
        {
            if (auto gain = find_number(nvp, "GAIN"))
            {
                camera_info["exposure"]["gain"] = *gain;
                spdlog::debug("Current camera gain of {} is {}", _name, *gain);
            }
        }
        else if (name == "CCD_OFFSET")
        {
            if (auto offset = find_number(nvp, "OFFSET"))
            {
                camera_info["exposure"]["offset"] = *offset;
                spdlog::debug("Current camera offset of {} is {}", _name, *offset);
            }
        }
        else if (name == "POLLING_PERIOD")
        {
            if (auto period = find_number(nvp, "PERIOD_MS"))
            {
                camera_info["network"]["period"] = *period;
                spdlog::debug("Current period of {} is {}", _name, *period);
            }
        }
        else if (name == "LIMITS")
        {
            if (auto max_buffer = find_number(nvp, "LIMITS_BUFFER_MAX"))
            {
                camera_info["limits"]["maxbuffer"] = *max_buffer;
                spdlog::debug("Current max buffer of {} is {}", _name, *max_buffer);
            }
            if (auto max_fps = find_number(nvp, "LIMITS_PREVIEW_FPS"))
            {
                camera_info["limits"]["maxfps"] = *max_fps;
                spdlog::debug("Current max fps of {} is {}", _name, *max_fps);
            }
        }
        else if (name == "STREAM_DELAY")
        {
            if (auto stream_delay = find_number(nvp, "STREAM_DELAY_TIME"))
            {
                camera_info["video"]["delay"] = *stream_delay;
                spdlog::debug("Current stream delay of {} is {}", _name, *stream_delay);
            }
        }
        else if (name == "STREAMING_EXPOSURE")
        {
            if (auto exposure = find_number(nvp, "STREAMING_EXPOSURE_VALUE"))
            {
                camera_info["video"]["exposure"] = *exposure;
                spdlog::debug("Current streaming exposure of {} is {}", _name, *exposure);
            }
            if (auto division = find_number(nvp, "STREAMING_DIVISOR_VALUE"))
            {
                camera_info["video"]["division"] = *division;
                spdlog::debug("Current streaming division of {} is {}", _name, *division);
            }
        }
        else if (name == "FPS")
        {
            if (auto fps = find_number(nvp, "EST_FPS"))
            {
                camera_info["video"]["fps"] = *fps;
                spdlog::debug("Current fps of {} is {}", _name, *fps);
            }
            if (auto avg_fps = find_number(nvp, "AVG_FPS"))
            {
                camera_info["video"]["avgfps"] = *avg_fps;
                spdlog::debug("Current average fps of {} is {}", _name, *avg_fps);
            }
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
        std::string_view PropName(property->getName());
        INDI_PROPERTY_TYPE Proptype = property->getType();

        if (Proptype == INDI_BLOB && PropName == indi_blob_name)
        {
            has_blob = 1;
            setBLOBMode(B_ALSO, device_name.c_str(), indi_blob_name.c_str());

#ifdef INDI_SHARED_BLOB_SUPPORT
            enableDirectBlobAccess(device_name.c_str(), indi_blob_name.c_str());
#endif
        }
        else if (Proptype == INDI_NUMBER && (PropName == indi_camera_cmd + "EXPOSURE" || PropName == indi_camera_cmd + "FRAME" || PropName == "STREAMING_EXPOSURE" || PropName == "FPS" || PropName == "POLLING_PERIOD" || PropName == "LIMITS"))
        {
            std::variant<INumberVectorProperty *, INumber *> num;
            if (PropName == indi_camera_cmd + "EXPOSURE")
            {
                expose_prop = property->getNumber();
                num = &expose_prop;
            }
            else if (PropName == indi_camera_cmd + "FRAME")
            {
                frame_prop = property->getNumber();
                num = &frame_prop;
            }
            else if (PropName == "STREAMING_EXPOSURE")
            {
                video_exposure_prop = property->getNumber();
                num = &video_exposure_prop;
            }
            else if (PropName == "FPS")
            {
                video_fps_prop = property->getNumber();
                num = &video_fps_prop;
            }
            else if (PropName == "POLLING_PERIOD")
            {
                polling_prop = property->getNumber();
                num = &polling_prop;
            }
            else if (PropName == "LIMITS")
            {
                camera_limit_prop = property->getNumber();
                num = &camera_limit_prop;
            }
            newNumber(*num);
        }
        else if (Proptype == INDI_SWITCH && (PropName == indi_camera_cmd + "FRAME_TYPE" || PropName == indi_camera_cmd + "VIDEO_STREAM" || PropName == "CONNECTION" || PropName == "DEBUG" || PropName == "CCD_COMPRESSION" || PropName == "UPLOAD_MODE" || PropName == "CCD_FAST_TOGGLE" || PropName == "FLIP" || PropName == "SIMULATION"))
        {
            std::variant<ISwitchVectorProperty *, ISwitch *> sw;
            if (PropName == indi_camera_cmd + "FRAME_TYPE")
            {
                frame_type_prop = property->getSwitch();
                sw = &frame_type_prop;
            }
            else if (PropName == indi_camera_cmd + "VIDEO_STREAM")
            {
                video_prop = property->getSwitch();
                sw = &video_prop;
            }
            else if (PropName == "CONNECTION")
            {
                connection_prop = property->getSwitch();
                sw = &connection_prop;
                ISwitch *connectswitch = IUFindSwitch(connection_prop, "CONNECT");
                is_connected = (connectswitch->s == ISS_ON);
                if (!is_connected)
                {
                    connection_prop->sp->s = ISS_ON;
                    sendNewSwitch(connection_prop);
                }
            }
            else if (PropName == "DEBUG")
            {
                debug_prop = property->getSwitch();
                sw = &debug_prop;
            }
            else if (PropName == "CCD_COMPRESSION")
            {
                compression_prop = property->getSwitch();
                sw = &compression_prop;
            }
            else if (PropName == "UPLOAD_MODE")
            {
                image_upload_mode_prop = property->getSwitch();
                sw = &image_upload_mode_prop;
            }
            else if (PropName == "CCD_FAST_TOGGLE")
            {
                fast_read_out_prop = property->getSwitch();
                sw = &fast_read_out_prop;
            }
            else if (PropName == "FLIP")
            {
                asi_image_flip_prop = property->getSwitch();
                sw = &asi_image_flip_prop;
            }
            else if (PropName == "SIMULATION")
            {
                toupcam_simulation_prop = property->getSwitch();
                sw = &toupcam_simulation_prop;
            }
            newSwitch(*sw);
        }
        else if (Proptype == INDI_TEXT && (PropName == indi_camera_cmd + "CFA" || PropName == "DEVICE_PORT" || PropName == "DRIVER_INFO" || PropName == "ACTIVE_DEVICES"))
        {
            std::variant<ITextVectorProperty *, IText *> txt;
            if (PropName == indi_camera_cmd + "CFA")
            {
                ITextVectorProperty *cfa_prop = property->getText();
                IText *cfa_type = IUFindText(cfa_prop, "CFA_TYPE");
                if (cfa_type && cfa_type->text && *cfa_type->text)
                {
                    spdlog::debug("{} CFA_TYPE is {}", _name, cfa_type->text);
                    is_color = true;
                }
                return;
            }
            else if (PropName == "DEVICE_PORT")
            {
                camera_port = property->getText();
                txt = camera_port;
                camera_info["network"]["port"] = camera_port->tp->text;
                spdlog::debug("Current device port of {} is {}", _name, camera_port->tp->text);
            }
            else if (PropName == "DRIVER_INFO")
            {
                device_name = IUFindText(property->getText(), "DRIVER_NAME")->text;
                indi_camera_exec = IUFindText(property->getText(), "DRIVER_EXEC")->text;
                indi_camera_version = IUFindText(property->getText(), "DRIVER_VERSION")->text;
                indi_camera_interface = IUFindText(property->getText(), "DRIVER_INTERFACE")->text;
                camera_info["driver"]["name"] = device_name;
                camera_info["driver"]["exec"] = indi_camera_exec;
                camera_info["driver"]["version"] = indi_camera_version;
                camera_info["driver"]["interfaces"] = indi_camera_interface;
                spdlog::debug("Camera Name : {} connected exec {}", _name, device_name, indi_camera_exec);
                return;
            }
            else if (PropName == "ACTIVE_DEVICES")
            {
                active_device_prop = property->getText();
                txt = active_device_prop;
            }
            newText(*txt);
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

    bool INDICamera::getParameter(const std::string &paramName)
    {
        if (paramName.empty())
        {
            spdlog::error("INDICamera::getParameter : Parameter name is required");
            return false;
        }
        return false;
    }

    bool INDICamera::setParameter(const std::string &paramName, const std::string &paramValue)
    {
        if (paramName.empty() || paramValue.empty())
        {
            spdlog::error("INDICamera::setParameter : Parameter name and value are required");
            return false;
            ;
        }
        return false;
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
        // 定义任务名称与逻辑之间的映射关系
        std::map<std::string, std::function<void(const nlohmann::json &)>> task_map = {
            {"Connect", [this](const nlohmann::json &tparams)
             {
                 if (tparams["name"].empty())
                 {
                     spdlog::error("No camera name specified");
                     return;
                 }
                 if (!this->connect(tparams["name"]))
                 {
                     spdlog::error("Failed to connect to camera {}", _name);
                     return;
                 }
             }},
            {"Disconnect", [this](const nlohmann::json &tparams)
             {
                 if (!this->is_connected && !this->camera_info["connected"].empty())
                 {
                     spdlog::warn("Camera is not connected, please do not execute disconnect command");
                     return;
                 }
                 if (!this->disconnect())
                 {
                     spdlog::error("Failed to disconnect from camera {}", _name);
                 }
             }},
            {"Reconnect", [this](const nlohmann::json &tparams)
             {
                 if (!this->is_connected && !this->camera_info["connected"].empty())
                 {
                     spdlog::warn("Camera is not connected, please do not execute reconnect command");
                     return;
                 }
                 if (!this->reconnect())
                 {
                     spdlog::error("Failed to reconnect from camera {}", _name);
                 }
             }},

            {"Scanning", [this](const nlohmann::json &tparams)
             {
                 if (!this->scanForAvailableDevices())
                 {
                     spdlog::error("Failed to scan for available devices from camera {}", _name);
                 }
             }},
            {"GetParameter", [this](const nlohmann::json &tparams)
             {
                 if (!this->getParameter(tparams["name"].get<std::string>()))
                 {
                     spdlog::error("Failed to get parameter from camera {}", _name);
                 }
             }},
            {"SetParameter", [this](const nlohmann::json &tparams)
             {
                 if (!this->setParameter(tparams["name"].get<std::string>(), tparams["value"].get<std::string>()))
                 {
                     spdlog::error("Failed to set parameter to camera {}", _name);
                 }
             }},
            {"SingleShot", [this](const nlohmann::json &tparams)
             {
                 spdlog::debug("{} SingleShot task is called", this->_name);
             }},

            {"AbortShot", [this](const nlohmann::json &tparams)
             {
                 spdlog::debug("{} AbortShot task is called", this->_name);
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
                     spdlog::error("Can't cool while cooling is unsupported");
                     return;
                 }
                 if (!this->setCoolingOn(tparams["enable"].get<bool>()))
                 {
                     spdlog::error("Failed to change the mode of cooling");
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
                     spdlog::error("Can't set temperature while cooling is unsupported");
                     return;
                 }
             }},
            {"SetGain", [this](const nlohmann::json &tparams)
             {
                 spdlog::debug("{} SetGain task is called", this->_name);
             }},
            {"SetOffset", [this](const nlohmann::json &tparams)
             {
                 spdlog::debug("{} SetOffset task is called", this->_name);
             }},
            {"SetBinning", [this](const nlohmann::json &tparams)
             {
                 if (!this->can_binning)
                 {
                     spdlog::error("Can't bin while binning is unsupported");
                     return;
                 }
                 if (!this->setBinning(tparams["binning"].get<int>()))
                 {
                     spdlog::error("Failed to change the mode of binning");
                 }
             }},
            {"SetROIFrame", [this](const nlohmann::json &tparams)
             {
                 if (!this->setROIFrame(tparams["start_x"].get<int>(), tparams["start_y"].get<int>(), tparams["frame_x"].get<int>(), tparams["frame_y"].get<int>()))
                 {
                     spdlog::error("Failed to change the mode of ROI");
                 }
             }}};

        auto it = task_map.find(task_name);
        if (it != task_map.end())
        {
            return std::make_shared<OpenAPT::SimpleTask>(it->second, std::initializer_list<json>{params});
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