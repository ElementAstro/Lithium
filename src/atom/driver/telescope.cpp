/*
 * telescope.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Telescope Simulator and Basic Definition

**************************************************/

#include "telescope.hpp"

Telescope::Telescope(const std::string &name) : AtomDriver(name) {}

Telescope::~Telescope() {}

bool Telescope::connect(const json &params) { return true; }

bool Telescope::disconnect(const json &params) { return true; }

bool Telescope::reconnect(const json &params) { return true; }

bool Telescope::isConnected() { return true; }

bool Telescope::SlewTo(const json &params) { return true; }

bool Telescope::Abort(const json &params) { return true; }

bool Telescope::isSlewing(const json &params) { return true; }

std::string Telescope::getCurrentRA(const json &params) { return ""; }

std::string Telescope::getCurrentDec(const json &params) { return ""; }

bool Telescope::StartTracking(const json &params) { return true; }

bool Telescope::StopTracking(const json &params) { return true; }

bool Telescope::setTrackingMode(const json &params) { return true; }

bool Telescope::setTrackingSpeed(const json &params) { return true; }

std::string Telescope::getTrackingMode(const json &params) { return ""; }

std::string Telescope::getTrackingSpeed(const json &params) { return ""; }

bool Telescope::Home(const json &params) { return true; }

bool Telescope::isAtHome(const json &params) { return true; }

bool Telescope::setHomePosition(const json &params) { return true; }

bool Telescope::isHomeAvailable(const json &params) { return true; }

bool Telescope::Park(const json &params) { return true; }

bool Telescope::Unpark(const json &params) { return true; }

bool Telescope::isAtPark(const json &params) { return true; }

bool Telescope::setParkPosition(const json &params) { return true; }

bool Telescope::isParkAvailable(const json &params) { return true; }
