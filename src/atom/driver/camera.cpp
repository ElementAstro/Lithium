/*
 * camera.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Camera Simulator and Basic Definition

**************************************************/

#include "camera.hpp"

Camera::Camera(const std::string &name) : AtomDriver(name) {}

Camera::~Camera() {}

bool Camera::connect(const json &params) { return true; }

bool Camera::disconnect(const json &params) { return true; }

bool Camera::reconnect(const json &params) { return true; }

bool Camera::isConnected() { return true; }

bool Camera::startExposure(const json &params) { return true; }

bool Camera::abortExposure(const json &params) { return true; }

bool Camera::getExposureStatus(const json &params) { return true; }

bool Camera::getExposureResult(const json &params) { return true; }

bool Camera::saveExposureResult(const json &params) { return true; }

bool Camera::startVideo(const json &params) { return true; }

bool Camera::stopVideo(const json &params) { return true; }

bool Camera::getVideoStatus(const json &params) { return true; }

bool Camera::getVideoResult(const json &params) { return true; }

bool Camera::saveVideoResult(const json &params) { return true; }

bool Camera::startCooling(const json &params) { return true; }

bool Camera::stopCooling(const json &params) { return true; }

bool Camera::isCoolingAvailable() { return true; }

bool Camera::getTemperature(const json &params) { return true; }

bool Camera::getCoolingPower(const json &params) { return true; }

bool Camera::getCoolingStatus(const json &params) { return true; }

bool Camera::setTemperature(const json &params) { return true; }

bool Camera::setCoolingPower(const json &params) { return true; }

bool Camera::getGain(const json &params) { return true; }

bool Camera::setGain(const json &params) { return true; }

bool Camera::isGainAvailable() { return true; }

bool Camera::getOffset(const json &params) { return true; }

bool Camera::setOffset(const json &params) { return true; }

bool Camera::isOffsetAvailable() { return true; }

bool Camera::getISO(const json &params) { return true; }

bool Camera::setISO(const json &params) { return true; }

bool Camera::isISOAvailable() { return true; }

bool Camera::getFrame(const json &params) { return true; }

bool Camera::setFrame(const json &params) { return true; }

bool Camera::isFrameSettingAvailable() { return true; }