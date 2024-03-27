/*
 * camera.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: AtomCamera Simulator and Basic Definition

**************************************************/

#include "camera.hpp"

#include "code.hpp"
#include "magic_enum/magic_enum.hpp"
#include "marco.hpp"

AtomCamera::AtomCamera(const std::string &name) : AtomDriver(name) {}

AtomCamera::~AtomCamera() {}

bool AtomCamera::initialize() {
    AtomDriver::initialize();
    // CCD Temperature
    registerVariable("CCD_TEMPERATURE_VALUE", 0.0, "Temperature (C)");
    registerVariableRanges("CCD_TEMPERATURE_VALUE", -50.0, 50.0);

    // AtomCamera temperature ramp
    registerVariable("RAMP_SLOPE", 0, "Max. dT (C/min)");
    registerVariableRanges("RAMP_SLOPE", 0, 30);
    registerVariable("RAMP_THRESHOLD", 0.2, "Threshold (C)");
    registerVariableRanges("RAMP_THRESHOLD", 0.1, 2);

    /**********************************************/
    /**************** Primary Chip ****************/
    /**********************************************/

    // Primary CCD Region-Of-Interest (ROI)
    registerVariable("X", 0.0, "Left");
    registerVariable("Y", 0.0, "Top");
    registerVariable("WIDTH", 0.0 ,"Width");
    registerVariable("HEIGHT", 0.0, "Height");

    // Primary CCD Frame Type
    registerVariable("FRAME_LIGHT", true, "Light");
    registerVariable("FRAME_BIAS", false, "Bias");
    registerVariable("FRAME_DARK", false, "Dark");
    registerVariable("FRAME_FLAT", false, "Flat");

    // Primary CCD Exposure
    registerVariable("CCD_EXPOSURE_VALUE", 1.0 ,"Duration (s)");
    registerVariableRanges("CCD_EXPOSURE_VALUE", 0.01, 3600);

    // Primary CCD Abort
    registerVariable("CCD_ABORT_EXPOSURE", false, "Abort");

    // Primary CCD Binning
    registerVariable("HOR_BIN", 1, "X");
    registerVariableRanges("HOR_BIN", 1, 4);
    registerVariable("VER_BIN", 1, "Y");
    registerVariableRanges("VER_BIN", 1, 4);

    // Primary CCD Info
    registerVariable("CCD_MAX_X", 0, "Max. Width");
    registerVariableRanges("CCD_MAX_X", 1, 16000);
    registerVariable("CCD_MAX_Y", 0, "Max. Height");
    registerVariableRanges("CCD_MAX_Y", 1, 16000);
    registerVariable("CCD_PIXEL_SIZE", 0, "Pixel size (um)");
    registerVariableRanges("CCD_PIXEL_SIZE", 1, 40);

    registerVariable("CCD_PIXEL_SIZE_X", 0.0 ,"Pixel size X");
    registerVariableRanges("CCD_PIXEL_SIZE_X", 0, 40);
    registerVariable("CCD_PIXEL_SIZE_Y", 0.0 ,"Pixel size Y");
    registerVariableRanges("CCD_PIXEL_SIZE_Y", 0, 40);
    registerVariable("CCD_BITSPERPIXEL", 0, "Bits per pixel");
    registerVariableRanges("CCD_BITSPERPIXEL", 8, 64);

    // Primary CCD Compression Options
    registerVariable("CCD_COMPRESSION", false, "Compression");

    /**********************************************/
    /******************** WCS *********************/
    /**********************************************/

    registerVariable("WCS_ENABLE", false, "Enable");
    registerVariable("CCD_ROTATION_VALUE", 0, "Rotation");
    registerVariableRanges("CCD_ROTATION_VALUE", -360, 360);
    registerVariable("FOCAL_LENGTH", 0, "Focal Length (mm)");
    registerVariableRanges("FOCAL_LENGTH", 10, 10000);
    registerVariable("APERTURE", 0, "Aperture (mm)");
    registerVariableRanges("APERTURE", 10, 3000);

    /**********************************************/
    /************** Capture Format ***************/
    /**********************************************/

    registerVariable("CCD_CAPTURE_FORMAT", "FTIS", "FitsFormat");

    /**********************************************/
    /************** Upload Settings ***************/
    /**********************************************/

    // Upload Mode
    registerVariable("UPLOAD_MODE", "Both", "Upload");

    // Upload Settings
    registerVariable("UPLOAD_DIR", "", "Dir");
    registerVariable("UPLOAD_PREFIX", "IMAGE_XXX", "Prefix");
    registerVariable("CCD_FILE_PATH", "", "Filename");

    /**********************************************/
    /****************** FITS Header****************/
    /**********************************************/
    registerVariable("FTIS_KEYWORD_NAME", "", "Name");
    registerVariable("FITS_KEYWORD_VALUE", "", "Value");
    registerVariable("KEYWORD_COMMENT", "", "Comment");

    /**********************************************/
    /******************** Methods******************/
    /**********************************************/

    registerFunc("startExposure", &AtomCamera::_startExposure, this);
    registerFunc("abortExposure", &AtomCamera::_abortExposure, this);
    registerFunc("getExposureStatus", &AtomCamera::_getExposureStatus, this);
    registerFunc("getExposureResult", &AtomCamera::_getExposureResult, this);

    return true;
}

bool AtomCamera::connect(const json &params) { return true; }

bool AtomCamera::disconnect(const json &params) { return true; }

bool AtomCamera::reconnect(const json &params) { return true; }

bool AtomCamera::isConnected() { return true; }

bool AtomCamera::startExposure(const double &duration) { return true; }

bool AtomCamera::abortExposure() { return true; }

bool AtomCamera::getExposureStatus() { return true; }

bool AtomCamera::getExposureResult() { return true; }

bool AtomCamera::saveExposureResult() { return true; }

bool AtomCamera::startVideo() { return true; }

bool AtomCamera::stopVideo() { return true; }

bool AtomCamera::getVideoStatus() { return true; }

bool AtomCamera::getVideoResult() { return true; }

bool AtomCamera::saveVideoResult() { return true; }

bool AtomCamera::startCooling() { return true; }

bool AtomCamera::stopCooling() { return true; }

bool AtomCamera::getCoolingStatus() { return true; }

bool AtomCamera::isCoolingAvailable() { return true; }

bool AtomCamera::getTemperature() { return true; }

bool AtomCamera::getCoolingPower() { return true; }

bool AtomCamera::setTemperature(const double &temperature) { return true; }

bool AtomCamera::setCoolingPower(const double &power) { return true; }

bool AtomCamera::getGain() { return true; }

bool AtomCamera::setGain(const int &gain) { return true; }

bool AtomCamera::isGainAvailable() { return true; }

bool AtomCamera::getOffset() { return true; }

bool AtomCamera::setOffset(const int &offset) { return true; }

bool AtomCamera::isOffsetAvailable() { return true; }

bool AtomCamera::getISO() { return true; }

bool AtomCamera::setISO(const int &iso) { return true; }

bool AtomCamera::isISOAvailable() { return true; }

bool AtomCamera::getFrame() { return true; }

bool AtomCamera::setFrame(const int &x, const int &y, const int &w, const int &h) {
    return true;
}

bool AtomCamera::isFrameSettingAvailable() { return true; }

bool AtomCamera::getBinning() { return true; }

bool AtomCamera::setBinning(const int &hor, const int &ver) { return true; }

bool AtomCamera::getFrameType() { return true; }

bool AtomCamera::setFrameType(FrameType type) { return true; }

bool AtomCamera::getUploadMode() { return true; }

bool AtomCamera::setUploadMode(UploadMode mode) { return true; }

json AtomCamera::_startExposure(const json &params) {
    CHECK_PARAM("exposure")
    double exposure = params["exposure"].get<double>();
    if (!setVariable("CCD_EXPOSURE_VALUE", exposure)) {
        LOG_F(ERROR, "Failed to set exposure time!");
        return createErrorResponse(
            __func__,
            {"error", magic_enum::enum_name(DeviceError::InvalidValue)},
            "Invalid Value");
    }
    setVariable("IS_EXPOSURE", true);
    if (!startExposure(exposure)) {
        LOG_F(ERROR, "Failed to start exposure");
        setVariable("IS_EXPOSURE", false);
        return createErrorResponse(
            __func__,
            {"error", magic_enum::enum_name(CameraError::ExposureError)},
            "Exposure Error");
    }
    setVariable("IS_EXPOSURE", false);
    DLOG_F(INFO, "Exposure success");
    return createSuccessResponse(__func__, {});
}

json AtomCamera::_abortExposure(const json &params) { return {}; }

json AtomCamera::_getExposureStatus(const json &params) { return {}; }

json AtomCamera::_getExposureResult(const json &params) { return {}; }

json AtomCamera::_saveExposureResult(const json &params) { return {}; }

json AtomCamera::_startVideo(const json &params) { return {}; }

json AtomCamera::_stopVideo(const json &params) { return {}; }

json AtomCamera::_getVideoStatus(const json &params) { return {}; }

json AtomCamera::_getVideoResult(const json &params) { return {}; }

json AtomCamera::_saveVideoResult(const json &params) { return {}; }

json AtomCamera::_startCooling(const json &params) { return {}; }

json AtomCamera::_stopCooling(const json &params) { return {}; }

json AtomCamera::_getCoolingStatus(const json &params) { return {}; }

json AtomCamera::_getTemperature(const json &params) { return {}; }

json AtomCamera::_getCoolingPower(const json &params) { return {}; }

json AtomCamera::_setTemperature(const json &params) { return {}; }

json AtomCamera::_setCoolingPower(const json &params) { return {}; }

json AtomCamera::_getGain(const json &params) { return {}; }

json AtomCamera::_setGain(const json &params) { return {}; }

json AtomCamera::_getOffset(const json &params) { return {}; }

json AtomCamera::_setOffset(const json &params) { return {}; }

json AtomCamera::_getISO(const json &params) { return {}; }

json AtomCamera::_setISO(const json &params) { return {}; }

json AtomCamera::_getFrame(const json &params) { return {}; }

json AtomCamera::_setFrame(const json &params) { return {}; }