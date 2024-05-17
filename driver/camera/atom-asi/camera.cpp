/*
 * camera.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: ASICamera Simulator and Basic Definition

**************************************************/

#include "camera.hpp"

#include "atom/driver/macro.hpp"
#include "atom/log/loguru.hpp"

#include <unistd.h>
#include <memory>

#define ASI_CAMERA_CONNECTION_CHECK                 \
    if (is_connected.load()) {                      \
        LOG_F(WARNING, "Camera already connected"); \
        return true;                                \
    }

#define ASI_CAMERA_CONNECT_CHECK              \
    if (!is_connected.load()) {               \
        LOG_F(ERROR, "Camera not connected"); \
        return false;                         \
    }

#define ASI_CAMERA_EXPOSURE_CHECK           \
    if (is_exposing.load()) {               \
        LOG_F(ERROR, "Camera is exposing"); \
        return false;                       \
    }

#define ASI_CAMERA_VIDEO_CHECK              \
    if (is_videoing.load()) {               \
        LOG_F(ERROR, "Camera is videoing"); \
        return false;                       \
    }

using ImgBufferPtr = std::unique_ptr<unsigned char[]>;

ASICamera::ASICamera(const std::string &name) : AtomCamera(name) {}

ASICamera::~ASICamera() {}

bool ASICamera::initialize() {
    AtomCamera::initialize();

    registerVariable("CAMERA_COUNT", 0, "the number of connected cameras");
    return true;
}

bool ASICamera::destroy() {
    AtomCamera::destroy();
    return true;
}

bool ASICamera::connect(const json &params) {
    ASI_CAMERA_CONNECTION_CHECK;
    if (!params.contains("name")) {
        LOG_F(ERROR, "No camera name provided");
        return false;
    }
    auto camera_name = params["name"].get<std::string>();

    auto camera_count = ASIGetNumOfConnectedCameras();
    if (camera_count <= 0) {
        LOG_F(ERROR,
              "ASI camera not found, please check the power supply or make "
              "sure the camera is connected.");
        return false;
    }
    for (int i = 0; i < camera_count; i++) {
        /*获取相机信息*/
        if ((errCode = ASIGetCameraProperty(&ASICameraInfo, i)) !=
            ASI_SUCCESS) {
            LOG_F(ERROR,
                  "Unable to get {} configuration information,the error "
                  "code is {},please check program permissions.\n",
                  ASICameraInfo.Name, errCode);
            return false;
        }
        if (ASICameraInfo.Name == camera_name) {
            LOG_F(INFO, "Find camera {}", ASICameraInfo.Name);
            // Max: The member variable is faster than component variable
            setVariable("DEVICE_ID", ASICameraInfo.CameraID);
            setVariable("DEVICE_NAME", ASICameraInfo.Name);
            m_camera_id = ASICameraInfo.CameraID;
            m_camera_name = ASICameraInfo.Name;
            /*打开相机*/
            if ((errCode = ASIOpenCamera(ASICameraInfo.CameraID)) !=
                ASI_SUCCESS) {
                LOG_F(ERROR, "Unable to turn on the {}, error code: {}.",
                      ASICameraInfo.Name, errCode);
                return false;
            }
            /*初始化相机*/
            if ((errCode = ASIInitCamera(ASICameraInfo.CameraID)) !=
                ASI_SUCCESS) {
                LOG_F(ERROR,
                      "Unable to initialize connection to "
                      "camera,the error code is {}.",
                      errCode);
                return false;
            }
            setVariable("DEVICE_CONNECTED", true);
            is_connected.store(true);
            LOG_F(INFO, "Camera connected successfully\n");
            return true;
        } else {
            LOG_F(ERROR, "This is not a designated camera");
        }
    }
    LOG_F(ERROR, "No camera found");
    return false;
}

bool ASICamera::disconnect(const json &params) { /*在关闭相机之前停止所有任务*/
    ASI_CAMERA_CONNECT_CHECK;
    if (!params.empty()) {
        LOG_F(ERROR, "No parameters are allowed");
        return false;
    }

    if (is_videoing.load()) {
        if ((errCode = ASIStopVideoCapture(m_camera_id)) !=
            ASI_SUCCESS)  // 停止视频拍摄
        {
            LOG_F(ERROR,
                  "Unable to stop video capture,error code is {},please try "
                  "again.",
                  errCode);
            return false;
        }
        is_videoing.store(false);
        setVariable("CCD_VIDEO_STATUS", false);
        LOG_F(INFO, "Stop video capture");
    }
    if (is_exposing.load()) {
        if ((errCode = ASIStopExposure(m_camera_id)) !=
            ASI_SUCCESS)  // 停止曝光
        {
            LOG_F(ERROR,
                  "Unable to stop exposure,error code is {}, please try again.",
                  errCode);
            return false;
        }
        is_exposing.store(false);
        setVariable("CCD_EXPOSURE_STATUS", false);
        LOG_F(INFO, "Stop exposure");
    }
    /*关闭相机*/
    if ((errCode = ASICloseCamera(m_camera_id)) != ASI_SUCCESS)  // 关闭相机
    {
        LOG_F(ERROR, "Unable to turn off the camera,error code: {}", errCode);
        return false;
    }
    setVariable("DEVICE_CONNECTED", false);
    is_connected.store(false);
    LOG_F(INFO, "Disconnect from camera");
    return true;
}

bool ASICamera::reconnect(const json &params) {
    ASI_CAMERA_CONNECT_CHECK;
    int timeout = 0;
    if (params.contains("timeout")) {
        timeout = params["timeout"].get<int>();
    }

    if (!disconnect({})) {
        LOG_F(ERROR, "Unable to disconnect from camera");
        return false;
    }
    if (!connect({})) {
        LOG_F(ERROR, "Unable to connect to camera");
        return false;
    }
    LOG_F(INFO, "Reconnect to camera: {}",
          getVariable<std::string>("DEVICE_NAME"));
    return true;
}

bool ASICamera::isConnected() { return is_connected.load(); }

bool ASICamera::startExposure(const double &duration) {
    ASI_CAMERA_CONNECT_CHECK;
    if (is_exposing.load()) {
        LOG_F(ERROR, "Exposure is already in progress");
        return false;
    }

    const long blink_duration = duration * 1000000;
    LOG_F(INFO, "Blinking {} time(us) before exposure", blink_duration);
    if ((errCode = ASISetControlValue(m_camera_id, ASI_EXPOSURE, blink_duration,
                                      ASI_FALSE)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Failed to set blink exposure to {}us, error {}",
              blink_duration, errCode);
        return false;
    }
    if ((errCode = ASIStartExposure(m_camera_id, ASI_FALSE)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Failed to start blink exposure, error code: {}", errCode);
        return false;
    }
    is_exposing.store(true);
    setVariable("CCD_EXPOSURE_STATUS", true);
    // Max: A timer is needed here
    do {
        usleep(10000);
        errCode = ASIGetExpStatus(m_camera_id, &expStatus);
    } while (errCode == ASI_SUCCESS && expStatus == ASI_EXP_WORKING);
    if (errCode != ASI_SUCCESS) {
        LOG_F(ERROR, "Blink exposure failed, error {}, error code: {}", errCode,
              expStatus);
        return false;
    }
    is_exposing.store(false);
    setVariable("CCD_EXPOSURE_STATUS", false);
    LOG_F(INFO, "Blink exposure completed");
    return true;
}

bool ASICamera::abortExposure() {
    ASI_CAMERA_CONNECT_CHECK;
    if (!is_exposing.load()) {
        LOG_F(ERROR, "No exposure is in progress");
        return false;
    }
    if ((errCode = ASIStopExposure(m_camera_id)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to stop camera exposure, error code: {}", errCode);
        return false;
    }
    setVariable("CCD_EXPOSURE_STATUS", false);
    is_exposing.store(false);
    LOG_F(INFO, "Abort exposure");
    return true;
}

bool ASICamera::getExposureStatus() {
    ASI_CAMERA_CONNECT_CHECK;
    if ((errCode = ASIGetExpStatus(m_camera_id, &expStatus)) != ASI_SUCCESS) {
        LOG_F(INFO, "Camera is busy, status code: {}", errCode);
        setVariable("CCD_EXPOSURE_STATUS", true);
        is_exposing.store(true);
        return true;
    }
    LOG_F(INFO, "Camera is idle");
    return false;
}

bool ASICamera::getExposureResult() {
    ASI_CAMERA_CONNECT_CHECK;
    ASI_CAMERA_EXPOSURE_CHECK;

    GET_INT_VARIABLE(width);
    GET_INT_VARIABLE(height);

    long imgSize = width * height;
    //* (1 + (ASICAMERA->ImageType == ASI_IMG_RAW16));

    // 使用智能指针管理图像缓冲区内存
    ImgBufferPtr imgBuf(new unsigned char[imgSize]);

    /*曝光后获取图像信息*/
    int errCode = ASIGetDataAfterExp(m_camera_id, imgBuf.get(), imgSize);
    if (errCode != ASI_SUCCESS) {
        // 获取图像失败
        LOG_F(ERROR, "Unable to get image from camera, error code: {}",
              errCode);
        return;
    }

    // 图像下载完成
    LOG_F(INFO, "Download from camera completely.");

    GET_STR_VARIABLE(upload_mode);
    if (upload_mode == "LOCAL") [[likely]] {
        // Max: image filename generation logic is needed
        std::string FitsName = "test.fits";
        LOG_F(INFO, "Upload mode is LOCAL, save image to {}", FitsName);
        /*将图像写入本地文件*/
        // auto res = getComponent("LITHIUM_IMAGE")
        //                ->runFunc("SaveImage", {{"filename", FitsName},
        //                                       {"data", imgBuf},
        //{ "size", imgSize }
        //});
        // if (res.contains("error")) {
        //    LOG_F(ERROR, "Unable to save image to {}, error: {}", FitsName,
        //          res["error"].get<std::string>());
        //    return false;
        //}
    } else if (upload_mode == "CLIENT") [[unlikely]] {
    } else if (upload_mode == "None") [[unlikely]] {
        LOG_F(INFO, "Upload mode is NONE, skip upload");
    } else {
        LOG_F(ERROR, "Invalid upload mode: {}", upload_mode);
        return false;
    }
    return true;
}

bool ASICamera::saveExposureResult() { return true; }

bool ASICamera::startVideo() { return true; }

bool ASICamera::stopVideo() { return true; }

bool ASICamera::getVideoStatus() { return true; }

bool ASICamera::getVideoResult() { return true; }

bool ASICamera::saveVideoResult() { return true; }

bool ASICamera::startCooling() { return true; }

bool ASICamera::stopCooling() { return true; }

bool ASICamera::getCoolingStatus() { return true; }

bool ASICamera::isCoolingAvailable() { return true; }

bool ASICamera::getTemperature() { return true; }

bool ASICamera::getCoolingPower() { return true; }

bool ASICamera::setTemperature(const double &temperature) {
    ASI_CAMERA_CONNECT_CHECK;
    ASI_CAMERA_EXPOSURE_CHECK;
    ASI_CAMERA_VIDEO_CHECK;
    if (!is_cooling_available) {
        LOG_F(ERROR, "Cooling is not available");
        return false;
    }
    /*转化温度参数*/
    long TargetTemp;
    if (temperature > 0.5)
        TargetTemp = static_cast<long>(temperature + 0.49);
    else if (temperature < 0.5)
        TargetTemp = static_cast<long>(temperature - 0.49);
    else
        TargetTemp = 0;
    /*设置相机温度*/
    if ((errCode = ASISetControlValue(m_camera_id, ASI_TEMPERATURE, TargetTemp,
                                      ASI_FALSE)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to set camera temperature, error code: {}",
              errCode);
        return false;
    }
    setVariable("CCD_TEMPERATURE_VALUE", TargetTemp);
    LOG_F(INFO, "Set camera cooling temperature to {}", TargetTemp);
    return true;
}

bool ASICamera::setCoolingPower(const double &power) { return true; }

bool ASICamera::getGain() {
    ASI_CAMERA_CONNECT_CHECK;
    long gain;
    if ((errCode = ASIGetControlValue(m_camera_id, ASI_GAIN, &gain, NULL)) !=
        ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to get camera gain, error code: {}", errCode);
        return false;
    }
    setVariable("CCD_GAIN", static_cast<int>(gain));
    m_gain.store(static_cast<int>(gain));
    LOG_F(INFO, "Get camera gain: {}", gain);
    return true;
}

bool ASICamera::setGain(const int &gain) {
    ASI_CAMERA_CONNECT_CHECK;
    ASI_CAMERA_EXPOSURE_CHECK;
    ASI_CAMERA_VIDEO_CHECK;
    if ((errCode = ASISetControlValue(m_camera_id, ASI_GAIN, gain,
                                      ASI_FALSE)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to set camera gain,error code: {}", errCode);
        return false;
    }
    setVariable("CCD_GAIN", gain);
    m_gain.store(gain);
    LOG_F(INFO, "Set camera gain to {}", gain);
    return true;
}

bool ASICamera::isGainAvailable() {
    LOG_F(INFO, "Gain is available for {}", m_camera_name);
    return true;
}

bool ASICamera::getOffset() {
    ASI_CAMERA_CONNECT_CHECK;
    long offset;
    if ((errCode = ASIGetControlValue(m_camera_id, ASI_BRIGHTNESS, &offset,
                                      NULL)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to get camera offset, error code: {}", errCode);
        return false;
    }
    setVariable("CCD_OFFSET", static_cast<int>(offset));
    m_offset.store(static_cast<int>(offset));
    LOG_F(INFO, "Get camera offset: {}", offset);
    return true;
}

bool ASICamera::setOffset(const int &offset) {
    ASI_CAMERA_CONNECT_CHECK;
    ASI_CAMERA_EXPOSURE_CHECK;
    ASI_CAMERA_VIDEO_CHECK;
    if ((errCode = ASISetControlValue(m_camera_id, ASI_BRIGHTNESS, offset,
                                      ASI_FALSE)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to set camera offset, error code: {}", errCode);
        return false;
    }
    setVariable("CCD_OFFSET", offset);
    m_offset.store(offset);
    LOG_F(INFO, "Set camera offset to {}", offset);
    return true;
}

bool ASICamera::isOffsetAvailable() {
    LOG_F(INFO, "Offset is available for {}", m_camera_name);
    return true;
}

bool ASICamera::getISO() {
    LOG_F(ERROR, "ISO is not available for {}", m_camera_name);
    return false;
}

bool ASICamera::setISO(const int &iso) {
    LOG_F(ERROR, "ISO is not available for {}", m_camera_name);
    return false;
}

bool ASICamera::isISOAvailable() {
    LOG_F(INFO, "ISO is not available for {}", m_camera_name);
    return false;
}

bool ASICamera::getFrame() { return true; }

bool ASICamera::setFrame(const int &x, const int &y, const int &w,
                         const int &h) {
    return true;
}

bool ASICamera::isFrameSettingAvailable() { return true; }

bool ASICamera::getBinning() { return true; }

bool ASICamera::setBinning(const int &hor, const int &ver) { return true; }

bool ASICamera::getFrameType() { return true; }

bool ASICamera::setFrameType(FrameType type) { return true; }

bool ASICamera::getUploadMode() { return true; }

bool ASICamera::setUploadMode(UploadMode mode) { return true; }

bool ASICamera::refreshCameraInfo() {
    if ((errCode = ASIGetCameraProperty(&ASICameraInfo, i)) != ASI_SUCCESS) {
        LOG_F(ERROR, "Unable to get camera information, error code: {}",
              errCode);
        return false;
    }
    return true;
}
