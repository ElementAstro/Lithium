/*
 * hydrogencamera.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-9

Description: Hydorgen Camera

**************************************************/

#include "hydrogencamera.hpp"

#include "config.h"

#include "atom/log/loguru.hpp"

HydrogenCamera::HydrogenCamera(const std::string &name) : Camera(name) {
    DLOG_F(INFO, "Hydorgen camera {} init successfully", name);
    m_number_switch = std::make_unique<
        Atom::Utils::StringSwitch<HYDROGEN::PropertyViewNumber *>>();
    m_switch_switch = std::make_unique<
        Atom::Utils::StringSwitch<HYDROGEN::PropertyViewSwitch *>>();
    m_text_switch = std::make_unique<
        Atom::Utils::StringSwitch<HYDROGEN::PropertyViewText *>>();
}

HydrogenCamera::~HydrogenCamera() {}

bool HydrogenCamera::connect(const json &params) {
    std::string name = params["name"];
    std::string hostname = params["host"];
    int port = params["port"];
    DLOG_F(INFO, "Trying to connect to {}", name);
    setServer(hostname.c_str(), port);
    // Receive messages only for our camera.
    watchDevice(name.c_str());
    // Connect to server.
    if (connectServer()) {
        DLOG_F(INFO, "{}: connectServer done ready", GetName());
        connectDevice(name.c_str());
        return !is_ready.load();
    }
    return false;
}

bool HydrogenCamera::disconnect(const json &params) {
    DLOG_F(INFO, "%s is disconnected", GetName());
    return true;
}

bool HydrogenCamera::reconnect(const json &params) { return true; }

bool HydrogenCamera::isConnected() { return true; }

bool HydrogenCamera::startExposure(const json &params) { return true; }

bool HydrogenCamera::abortExposure(const json &params) { return true; }

bool HydrogenCamera::getExposureStatus(const json &params) { return true; }

bool HydrogenCamera::getExposureResult(const json &params) { return true; }

bool HydrogenCamera::saveExposureResult(const json &params) { return true; }

bool HydrogenCamera::startVideo(const json &params) { return true; }

bool HydrogenCamera::stopVideo(const json &params) { return true; }

bool HydrogenCamera::getVideoStatus(const json &params) { return true; }

bool HydrogenCamera::getVideoResult(const json &params) { return true; }

bool HydrogenCamera::saveVideoResult(const json &params) { return true; }

bool HydrogenCamera::startCooling(const json &params) { return true; }

bool HydrogenCamera::stopCooling(const json &params) { return true; }

bool HydrogenCamera::isCoolingAvailable() { return true; }

bool HydrogenCamera::getTemperature(const json &params) { return true; }

bool HydrogenCamera::getCoolingPower(const json &params) { return true; }

bool HydrogenCamera::setTemperature(const json &params) { return true; }

bool HydrogenCamera::setCoolingPower(const json &params) { return true; }

bool HydrogenCamera::getGain(const json &params) { return true; }

bool HydrogenCamera::setGain(const json &params) { return true; }

bool HydrogenCamera::isGainAvailable() { return true; }

bool HydrogenCamera::getOffset(const json &params) { return true; }

bool HydrogenCamera::setOffset(const json &params) { return true; }

bool HydrogenCamera::isOffsetAvailable() { return true; }

bool HydrogenCamera::getISO(const json &params) { return true; }

bool HydrogenCamera::setISO(const json &params) { return true; }

bool HydrogenCamera::isISOAvailable() { return true; }

bool HydrogenCamera::getFrame(const json &params) { return true; }

bool HydrogenCamera::setFrame(const json &params) { return true; }

bool HydrogenCamera::isFrameSettingAvailable() { return true; }

void HydrogenCamera::newDevice(HYDROGEN::BaseDevice dp) {
    if (strcmp(dp.getDeviceName(), GetName().c_str()) == 0) {
        camera_device = dp;
    }
}

void HydrogenCamera::removeDevice(HYDROGEN::BaseDevice dp) {
    ClearStatus();
    DLOG_F(INFO, "{} disconnected", dp.getDeviceName());
}

void HydrogenCamera::newProperty(HYDROGEN::Property property) {
    std::string PropName(property.getName());
    HYDROGEN_PROPERTY_TYPE Proptype = property.getType();

    switch (Proptype) {
        case HYDROGEN_SWITCH:
            newSwitch(property.getSwitch());
            break;
        case HYDROGEN_NUMBER:
            newNumber(property.getNumber());
            break;
        case HYDROGEN_TEXT:
            newText(property.getText());
            break;
        case HYDROGEN_BLOB:
            newBLOB(property.getBLOB());
            break;
    };
}

void HydrogenCamera::updateProperty(HYDROGEN::Property property) {
    // we go here every time a Switch state change

    switch (property.getType()) {
        case HYDROGEN_SWITCH: {
            auto svp = property.getSwitch();
            DLOG_F(INFO, "{}: {}", GetName(), svp->name);
            newSwitch(svp);
        } break;
        case HYDROGEN_NUMBER: {
            auto nvp = property.getNumber();
            DLOG_F(INFO, "{}: {}", GetName(), nvp->name);
            newNumber(nvp);
        } break;
        case HYDROGEN_TEXT: {
            auto tvp = property.getText();
            DLOG_F(INFO, "{}: {}", GetName(), tvp->name);
            newText(tvp);
        } break;
        case HYDROGEN_BLOB: {
            // we go here every time a new blob is available
            // this is normally the image from the camera

            auto bvp = property.getBLOB();
            auto bp = bvp->at(0);
            newBLOB(bvp);
        } break;
        default:
            break;
    }
}

void HydrogenCamera::newSwitch(HYDROGEN::PropertyViewSwitch *svp) {
    std::string name = svp->name;

    DLOG_F(INFO, "{} Received Switch: {}", GetName(), name);

    if (name == "CONNECTION") {
        m_connection_prop.reset(svp);
        if (auto connectswitch = IUFindSwitch(svp, "CONNECT");
            connectswitch->s == ISS_ON) {
            SetVariable("connect", true);
            is_connected.store(true);
            DLOG_F(INFO, "{} is connected", GetName());
        } else {
            if (is_ready.load()) {
                ClearStatus();
                SetVariable("connect", false);
                is_connected.store(true);
                DLOG_F(INFO, "{} is disconnected", GetName());
            }
        }
    } else if (name == "DEBUG") {
        debug_prop.reset(svp);
        if (auto debugswitch = IUFindSwitch(svp, "ENABLE");
            debugswitch->s == ISS_ON) {
            SetVariable("debug", true);
            is_debug.store(true);
            DLOG_F(INFO, "DEBUG mode of {} is enabled", GetName());
        } else {
            SetVariable("debug", false);
            is_debug.store(false);
            DLOG_F(INFO, "DEBUG mode of {} is disabled", GetName());
        }
    } else if (name == "CCD_FRAME_TYPE") {
        frame_type_prop.reset(svp);
        std::string type;
        if (auto lightswitch = IUFindSwitch(svp, "FRAME_LIGHT");
            lightswitch->s == ISS_ON)
            type = "Light";
        else if (auto darkswitch = IUFindSwitch(svp, "FRAME_DARK");
                 darkswitch->s == ISS_ON)
            type = "Dark";
        else if (auto flatswitch = IUFindSwitch(svp, "FRAME_FLAT");
                 flatswitch->s == ISS_ON)
            type = "Flat";
        else if (auto biasswitch = IUFindSwitch(svp, "FRAME_BIAS");
                 biasswitch->s == ISS_ON)
            type = "Bias";
        SetVariable("frame_type", type);
        frame.frame_type = type;
        DLOG_F(INFO, "Current frame type of {} is {}", GetName(),
               frame.frame_type);
    } else if (name == "CCD_TRANSFER_FORMAT") {
        frame_format_prop.reset(svp);
        std::string format;
        if (auto fitsswitch = IUFindSwitch(svp, "FORMAT_FITS");
            fitsswitch->s == ISS_ON)
            format = "Fits";
        else if (auto natswitch = IUFindSwitch(svp, "FORMAT_NATIVE");
                 natswitch->s == ISS_ON)
            format = "Raw";
        else if (auto xisfswitch = IUFindSwitch(svp, "FORMAT_XISF");
                 xisfswitch->s == ISS_ON)
            format = "Xisf";
        SetVariable("frame_format", format);
        frame.frame_format = format;
        DLOG_F(INFO, "Current frame format of {} is {}", GetName(),
               frame.frame_format);
    } else if (name == "CCD_ABORT_EXPOSURE") {
        abort_exposure_prop.reset(svp);
        if (auto abortswitch = IUFindSwitch(svp, "ABORT_EXPOSURE");
            abortswitch->s == ISS_ON) {
            SetVariable("is_exposure", false);
            is_exposure.store(false);
            DLOG_F(INFO, "{} is stopped", GetName());
        }
    } else if (name == "UPLOAD_MODE") {
        image_upload_mode_prop.reset(svp);
        std::string mode;
        if (auto clientswitch = IUFindSwitch(svp, "UPLOAD_CLIENT");
            clientswitch->s == ISS_ON)
            mode = "Client";
        else if (auto localswitch = IUFindSwitch(svp, "UPLOAD_LOCAL");
                 localswitch->s == ISS_ON)
            mode = "Local";
        else if (auto bothswitch = IUFindSwitch(svp, "UPLOAD_BOTH");
                 bothswitch->s == ISS_ON)
            mode = "Both";
        frame.upload_mode = mode;
        DLOG_F(INFO, "Current upload mode of {} is {}", GetName(),
               frame.upload_mode);
    } else if (name == "CCD_FAST_TOGGLE") {
        fast_read_out_prop.reset(svp);
        if (auto enabledswitch = IUFindSwitch(svp, "HYDROGEN_ENABLED");
            enabledswitch->s == ISS_ON) {
            SetVariable("is_fastread", true);
            frame.is_fastread.store(true);
            DLOG_F(INFO, "Current fast readout mode of {} is enabled",
                   GetName());
        } else if (auto disabledswitch = IUFindSwitch(svp, "HYDROGEN_DISABLED");
                   disabledswitch->s == ISS_ON) {
            SetVariable("is_fastread", false);
            frame.is_fastread.store(false);
            DLOG_F(INFO, "Current fast readout mode of {} is disabled",
                   GetName());
        }
    } else if (name == "CCD_VIDEO_STREAM") {
        video_prop.reset(svp);
        if (auto onswitch = IUFindSwitch(svp, "STREAM_ON");
            onswitch->s == ISS_ON) {
            SetVariable("is_video", true);
            is_video.store(true);
            DLOG_F(INFO, "{} start video capture", GetName());
        } else if (auto offswitch = IUFindSwitch(svp, "STREAM_OFF");
                   offswitch->s == ISS_ON) {
            SetVariable("is_video", false);
            is_video.store(false);
            DLOG_F(INFO, "{} stop video capture", GetName());
        }
    } else if (name == "FLIP") {
    } else if (name == "CCD_COMPRESSION") {
    }

    else if (name == "CCD_CONTROLS") {
    } else if (name == "CCD_CONTROLS_MODE") {
    }
    // The following properties are for Toup Camera
    else if (name == "TC_FAN_CONTROL") {
    } else if (name == "TC_FAN_Speed") {
    } else if (name == "TC_AUTO_WB") {
    } else if (name == "TC_HEAT_CONTROL") {
    } else if (name == "TC_HCG_CONTROL") {
    } else if (name == "TC_LOW_NOISE_CONTROL") {
    }
    /*
    else if (PropName == "CCD_BINNING_MODE" && Proptype == HYDROGEN_SWITCH)
    {
    }
    */
}

void HydrogenCamera::newMessage(HYDROGEN::BaseDevice dp, int messageID) {
    DLOG_F(INFO, "{} Received message: {}", GetName(),
           dp.messageQueue(messageID));
}

void HydrogenCamera::serverConnected() {
    DLOG_F(INFO, "{} Connected to server", GetName());
}

void HydrogenCamera::serverDisconnected(int exit_code) {
    DLOG_F(INFO, "{} Disconnected from server", GetName());

    ClearStatus();
}

inline static const char *StateStr(IPState st) {
    switch (st) {
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

void HydrogenCamera::newNumber(HYDROGEN::PropertyViewNumber *nvp) {
    using namespace std::string_literals;
    const std::string name = nvp->name;
    if (name == "CCD_EXPOSURE") {
        const double exposure = nvp->np->value;
        current_exposure.store(exposure);
        DLOG_F(INFO, "Current CCD_EXPOSURE for {} is {}", GetName(), exposure);
    } else if (name == "CCD_INFO") {
        ccdinfo_prop.reset(nvp);
        frame.pixel.store(IUFindNumber(nvp, "CCD_PIXEL_SIZE")->value);
        frame.pixel_x.store(IUFindNumber(nvp, "CCD_PIXEL_SIZE_X")->value);
        frame.pixel_y.store(IUFindNumber(nvp, "CCD_PIXEL_SIZE_Y")->value);
        frame.max_frame_x.store(IUFindNumber(nvp, "CCD_MAX_X")->value);
        frame.max_frame_y.store(IUFindNumber(nvp, "CCD_MAX_Y")->value);
        frame.pixel_depth.store(IUFindNumber(nvp, "CCD_BITSPERPIXEL")->value);

        DLOG_F(INFO,
               "{} pixel {} pixel_x {} pixel_y {} max_frame_x {} max_frame_y "
               "{} pixel_depth {}",
               GetName(), frame.pixel.load(), frame.pixel_x.load(),
               frame.pixel_y.load(), frame.max_frame_x.load(),
               frame.max_frame_y.load(), frame.pixel_depth.load());
    } else if (name == "CCD_BINNING") {
        hydrogen_binning_x.reset(IUFindNumber(nvp, "HOR_BIN"));
        hydrogen_binning_y.reset(IUFindNumber(nvp, "VER_BIN"));
        frame.binning_x.store(hydrogen_binning_x->value);
        frame.binning_y.store(hydrogen_binning_y->value);
        DLOG_F(INFO, "Current binning_x and y of {} are {} {}", GetName(),
               hydrogen_binning_x->value, hydrogen_binning_y->value);
    } else if (name == "CCD_FRAME") {
        hydrogen_frame_x.reset(IUFindNumber(nvp, "X"));
        hydrogen_frame_y.reset(IUFindNumber(nvp, "Y"));
        hydrogen_frame_width.reset(IUFindNumber(nvp, "WIDTH"));
        hydrogen_frame_height.reset(IUFindNumber(nvp, "HEIGHT"));

        frame.frame_x.store(hydrogen_frame_x->value);
        frame.frame_y.store(hydrogen_frame_y->value);
        frame.frame_height.store(hydrogen_frame_height->value);
        frame.frame_width.store(hydrogen_frame_width->value);

        DLOG_F(INFO, "Current frame of {} are {} {} {} {}", GetName(),
               hydrogen_frame_width->value, hydrogen_frame_y->value,
               hydrogen_frame_width->value, hydrogen_frame_height->value);
    } else if (name == "CCD_TEMPERATURE") {
        camera_temperature_prop.reset(nvp);
        current_temperature.store(
            IUFindNumber(nvp, "CCD_TEMPERATURE_VALUE")->value);
        DLOG_F(INFO, "Current temperature of {} is {}", GetName(),
               current_temperature.load());
    } else if (name == "CCD_GAIN") {
        gain_prop.reset(nvp);
        current_gain.store(IUFindNumber(nvp, "GAIN")->value);
        SetVariable("gain", current_gain.load());
        DLOG_F(INFO, "Current camera gain of {} is {}", GetName(),
               current_gain.load());
    } else if (name == "CCD_OFFSET") {
        offset_prop.reset(nvp);
        current_offset.store(IUFindNumber(nvp, "OFFSET")->value);
        SetVariable("offset", current_offset.load());
        DLOG_F(INFO, "Current camera offset of {} is {}", GetName(),
               current_offset.load());
    }
    /*
     else if (name == "POLLING_PERIOD")
    {
        device_info["network"]["period"] = IUFindNumber(nvp,
    "PERIOD_MS")->value; DLOG_F(INFO, "Current period of {} is {}", GetName(),
    device_info["network"]["period"].dump());
    }
    else if (name == "LIMITS")
    {
        device_info["limits"]["maxbuffer"] = IUFindNumber(nvp,
    "LIMITS_BUFFER_MAX")->value; DLOG_F(INFO, "Current max buffer of {} is {}",
    GetName(), device_info["limits"]["maxbuffer"].dump());
        device_info["limits"]["maxfps"] = IUFindNumber(nvp,
    "LIMITS_PREVIEW_FPS")->value; DLOG_F(INFO, "Current max fps of {} is {}",
    GetName(), device_info["limits"]["maxfps"].dump());
    }
    else if (name == "STREAM_DELAY")
    {
        device_info["video"]["delay"] = IUFindNumber(nvp,
    "STREAM_DELAY_TIME")->value; DLOG_F(INFO, "Current stream delay of {} is
    {}", GetName(), device_info["video"]["delay"].dump());
    }
    else if (name == "STREAMING_EXPOSURE")
    {
        device_info["video"]["exposure"] = IUFindNumber(nvp,
    "STREAMING_EXPOSURE_VALUE")->value; DLOG_F(INFO, "Current streaming exposure
    of {} is {}", GetName(), device_info["video"]["exposure"].dump());
        device_info["video"]["division"] = IUFindNumber(nvp,
    "STREAMING_DIVISOR_VALUE")->value; DLOG_F(INFO, "Current streaming division
    of {} is {}", GetName(), device_info["video"]["division"].dump());
    }
    else if (name == "FPS")
    {
        device_info["video"]["fps"] = IUFindNumber(nvp, "EST_FPS")->value;
        DLOG_F(INFO, "Current fps of {} is {}", GetName(),
    device_info["video"]["fps"].dump()); device_info["video"]["avgfps"] =
    IUFindNumber(nvp, "AVG_FPS")->value; DLOG_F(INFO, "Current average fps of {}
    is {}", GetName(), device_info["video"]["avgfps"].dump());
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

void HydrogenCamera::newText(HYDROGEN::PropertyViewText *tvp) {
    const std::string name = tvp->name;
    DLOG_F(INFO, "{} Received Text: {} = {}", GetName(), name, tvp->tp->text);

    if (name == hydrogen_camera_cmd + "CFA") {
        cfa_prop.reset(tvp);
        cfa_type_prop.reset(IUFindText(tvp, "CFA_TYPE"));
        if (cfa_type_prop && cfa_type_prop->text && *cfa_type_prop->text) {
            DLOG_F(INFO, "{} CFA_TYPE is {}", GetName(), cfa_type_prop->text);
            is_color = true;
            SetVariable("is_color", true);
        } else {
            SetVariable("is_color", false);
        }
    } else if (name == "DEVICE_PORT") {
        camera_prop.reset(tvp);
        hydrogen_camera_port = tvp->tp->text;
        SetVariable("port", hydrogen_camera_port);
        DLOG_F(INFO, "Current device port of {} is {}", GetName(),
               camera_prop->tp->text);
    } else if (name == "DRIVER_INFO") {
        hydrogen_camera_exec = IUFindText(tvp, "DRIVER_EXEC")->text;
        hydrogen_camera_version = IUFindText(tvp, "DRIVER_VERSION")->text;
        hydrogen_camera_interface = IUFindText(tvp, "DRIVER_INTERFACE")->text;
        DLOG_F(INFO, "Camera Name : {} connected exec {}", GetName(), GetName(),
               hydrogen_camera_exec);
    } else if (name == "ACTIVE_DEVICES") {
        active_device_prop.reset(tvp);
    }
}

void HydrogenCamera::newBLOB(HYDROGEN::PropertyViewBlob *bp) {
    // we go here every time a new blob is available
    // this is normally the image from the camera

    DLOG_F(INFO, "{} Received BLOB {} len = {} size = {}", GetName(), bp->name);

    if (exposure_prop) {
        if (bp->name == hydrogen_blob_name.c_str()) {
            has_blob = 1;
            // set option to receive blob and messages for the selected CCD
            setBLOBMode(B_ALSO, GetName().c_str(), hydrogen_blob_name.c_str());

#ifdef HYDROGEN_SHARED_BLOB_SUPPORT
            // Allow faster mode provided we don't modify the blob content or
            // free/realloc it
            enableDirectBlobAccess(GetName().c_str(),
                                   hydrogen_blob_name.c_str());
#endif
        }
    } else if (video_prop) {
    }
}

void HydrogenCamera::ClearStatus() {
    m_connection_prop = nullptr;
    exposure_prop = nullptr;
    frame_prop = nullptr;
    frame_type_prop = nullptr;
    ccdinfo_prop = nullptr;
    binning_prop = nullptr;
    video_prop = nullptr;
    camera_prop = nullptr;
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
