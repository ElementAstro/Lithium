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
#include <optional>

#include "macro.hpp"

AtomCamera::AtomCamera(const std::string &name) : AtomDriver(name) {}

AtomCamera::~AtomCamera() {}

auto AtomCamera::connect(const std::string &name, int timeout,
                         int maxRetry) -> bool {
    ATOM_UNREF_PARAM(name);
    ATOM_UNREF_PARAM(timeout);
    ATOM_UNREF_PARAM(maxRetry);
    return true;
}

auto AtomCamera::disconnect(bool force, int timeout, int maxRetry) -> bool {
    ATOM_UNREF_PARAM(force);
    ATOM_UNREF_PARAM(timeout);
    ATOM_UNREF_PARAM(maxRetry);
    return true;
}

auto AtomCamera::reconnect(int timeout, int maxRetry) -> bool {
    ATOM_UNREF_PARAM(timeout);
    ATOM_UNREF_PARAM(maxRetry);
    return true;
}

auto AtomCamera::scan() -> std::vector<std::string> { return {}; }

auto AtomCamera::isConnected() -> bool { return true; }

auto AtomCamera::startExposure(const double &duration) -> bool {
    ATOM_UNREF_PARAM(duration);
    return true;
}

auto AtomCamera::abortExposure() -> bool { return true; }

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

auto AtomCamera::getTemperature() -> std::optional<double> {
    return std::nullopt;
}

bool AtomCamera::getCoolingPower() { return true; }

bool AtomCamera::setTemperature(const double &temperature) {
    ATOM_UNREF_PARAM(temperature);
    return true;
}

bool AtomCamera::setCoolingPower(const double &power) {
    ATOM_UNREF_PARAM(power);
    return true;
}

auto AtomCamera::getGain() -> std::optional<double> { return std::nullopt; }

bool AtomCamera::setGain(const int &gain) {
    ATOM_UNREF_PARAM(gain);
    return true;
}

auto AtomCamera::isGainAvailable() -> bool { return true; }

auto AtomCamera::getOffset() -> std::optional<double> { return std::nullopt; }

bool AtomCamera::setOffset(const int &offset) {
    ATOM_UNREF_PARAM(offset);
    return true;
}

bool AtomCamera::isOffsetAvailable() { return true; }

bool AtomCamera::getISO() { return true; }

bool AtomCamera::setISO(const int &iso) {
    ATOM_UNREF_PARAM(iso);
    return true;
}

bool AtomCamera::isISOAvailable() { return true; }

bool AtomCamera::getFrame() { return true; }

bool AtomCamera::setFrame(const int &x, const int &y, const int &w,
                          const int &h) {
    ATOM_UNREF_PARAM(x);
    ATOM_UNREF_PARAM(y);
    ATOM_UNREF_PARAM(w);
    ATOM_UNREF_PARAM(h);
    return true;
}

bool AtomCamera::isFrameSettingAvailable() { return true; }

auto AtomCamera::getBinning() -> std::optional<std::tuple<int, int, int, int>> {
    return std::nullopt;
}

auto AtomCamera::setBinning(const int &hor, const int &ver) -> bool {
    ATOM_UNREF_PARAM(hor);
    ATOM_UNREF_PARAM(ver);
    return true;
}

auto AtomCamera::getFrameType() -> bool { return true; }

auto AtomCamera::setFrameType(FrameType type) -> bool {
    ATOM_UNREF_PARAM(type);
    return true;
}

auto AtomCamera::getUploadMode() -> bool { return true; }

auto AtomCamera::setUploadMode(UploadMode mode) -> bool {
    ATOM_UNREF_PARAM(mode);
    return true;
}
