/*
 * camera.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Touptek Camera

**************************************************/

#include "camera.hpp"

#include "atom/log/loguru.hpp"

#include <memory>

ToupCamera::ToupCamera(const std::string &name)
 : ToupCamera(name) {}

ToupCamera::~ToupCamera() {}

bool ToupCamera::initialize() { return true; }

bool ToupCamera::destroy() { return true; }

bool ToupCamera::connect(const json &params) { return true; }

bool ToupCamera::disconnect(const json &params) { return true; }

bool ToupCamera::reconnect(const json &params) { return true; }

bool ToupCamera::isConnected() { return true; }

bool ToupCamera::startExposure(const double &duration) { return true; }

bool ToupCamera::abortExposure() { return true; }

bool ToupCamera::getExposureStatus() { return true; }

bool ToupCamera::getExposureResult() { return true; }

bool ToupCamera::saveExposureResult() { return true; }

bool ToupCamera::startVideo() { return true; }

bool ToupCamera::stopVideo() { return true; }

bool ToupCamera::getVideoStatus() { return true; }

bool ToupCamera::getVideoResult() { return true; }

bool ToupCamera::saveVideoResult() { return true; }

bool ToupCamera::startCooling() { return true; }

bool ToupCamera::stopCooling() { return true; }

bool ToupCamera::getCoolingStatus() { return true; }

bool ToupCamera::isCoolingAvailable() { return true; }

bool ToupCamera::getTemperature() { return true; }

bool ToupCamera::getCoolingPower() { return true; }

bool ToupCamera::setTemperature(const double &temperature) { return true; }

bool ToupCamera::setCoolingPower(const double &power) { return true; }

bool ToupCamera::getGain() { return true; }

bool ToupCamera::setGain(const int &gain) { return true; }

bool ToupCamera::isGainAvailable() { return true; }

bool ToupCamera::getOffset() { return true; }

bool ToupCamera::setOffset(const int &offset) { return true; }

bool ToupCamera::isOffsetAvailable() { return true; }

bool ToupCamera::getISO() { return true; }

bool ToupCamera::setISO(const int &iso) { return true; }

bool ToupCamera::isISOAvailable() { return true; }

bool ToupCamera::getFrame() { return true; }

bool ToupCamera::setFrame(const int &x, const int &y, const int &w,
                          const int &h) {
    return true;
}

bool ToupCamera::isFrameSettingAvailable() { return true; }

bool ToupCamera::getBinning() { return true; }

bool ToupCamera::setBinning(const int &hor, const int &ver) { return true; }

bool ToupCamera::getFrameType() { return true; }

bool ToupCamera::setFrameType(FrameType type) { return true; }

bool ToupCamera::getUploadMode() { return true; }

bool ToupCamera::setUploadMode(UploadMode mode) { return true; }
